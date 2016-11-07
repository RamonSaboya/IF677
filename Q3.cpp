#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
// Quantidade de produtores
#define P 4
// Quantidade de consumidores
#define C 4
// Tamanho de cada fila bloqueante
#define BSIZE 20
// Quantidade de filas bloqueantes
#define B 3
// Auxiliar para pegar index da fila (usados nos arrays de mutexes e conds)
#define bqueue_index bqindex[get_address(Q)]
// Struct do nó das filas
typedef struct elem {
  int value;
  struct elem *prox;
} Elem;

// Struct da fila bloqueante
typedef struct blockingQueue {
  unsigned bufferSize, bufferStatus;
  Elem *head, *last;
} BlockingQueue;

pthread_mutex_t mutex[B]; // Array dos mutexes: um para cada fila bloqueante
pthread_cond_t cond_prod[B], cond_cons[B]; // Array dos conds de produtores e consumidores: um produtor e um consumidor para cada fila bloqueante
BlockingQueue *bqueue[B]; // Array de filas bloqueantes
map<string, int> bqindex; // Mapeamento de endereço de memória para index dos arrays acima

long prod_bq[P] = {0, 1, 1, 2}, cons_bq[C] = {0, 1, 2, 2}; // Direcionamento de cada produtor e consumidor: associa qual fila bloqueante os threads usarão

// Retorna um endereço de memória hexadecimal em string (utilizado para mapear as filas para um index)
string get_address(BlockingQueue *Q) {
  int adr_int = *((int*) Q);
  char *adr_char = new char[32];
  sprintf(adr_char, "%x", adr_int);
  string adr = adr_char;
  return adr;
}

// Inicia uma fila bloqueante vazia e retorna seu endereço de memória
BlockingQueue *newBlockingQueue(unsigned inBufferSize) {
  BlockingQueue *bq = new BlockingQueue;
  bq->bufferSize = inBufferSize;
  bq->bufferStatus = 0;
  bq->head = NULL;
  bq->last = NULL;
  return bq;
}

// Inserre um valor em uma fila bloqueante
void putBlockingQueue(BlockingQueue *Q, int newValue) {
  // Cria o elemento que vai ser inserido com o próximo valor nulo
  Elem *e = new Elem;
  e->value = newValue;
  e->prox = NULL;

  pthread_mutex_lock(&mutex[bqueue_index]);

  // Caso a fila esteja cheia, é necessário aguardar o sinal de quando algum elemento for removido da mesma
  while(Q->bufferStatus == (Q->bufferSize - 1)) {
    cout << "    (" << bqueue_index << ") BQ cheia, pausando execução" << endl;
    pthread_cond_wait(&cond_prod[bqueue_index], &mutex[bqueue_index]);
    cout << "    (" << bqueue_index << ") BQ esvaziou, continuando execução" << endl;
  }

  // Insere o novo elemento
  if(Q->head == NULL) {
    // Caso a fila estivesse vazia, head e tail vão receber o endereço do elemento
    Q->head = e;
    Q->last = e;
  } else {
    // Caso a fila não estivesse vazia, o próximo do tail será setado como o novo elemento, que será a nova tail
    Q->last->prox = e;
    Q->last = e;
  }
  Q->bufferStatus++; // Incrementa o contador de elementos da fila

  pthread_cond_signal(&cond_cons[bqueue_index]); // Avisa que a fila recebeu um valor

  pthread_mutex_unlock(&mutex[bqueue_index]);
}

// Remove o elemento na frente de uma fila bloqueante
int takeBlockingQueue(BlockingQueue *Q) {
  pthread_mutex_lock(&mutex[bqueue_index]);

  // Caso a fila estiver vazia, é necessário aguardar o sinal de quando algum elemento for adcionado a mesma
  while(Q->head == NULL) {
    cout << "    (" << bqueue_index << ") BQ vazia, pausando execução" << endl;
    pthread_cond_wait(&cond_cons[bqueue_index], &mutex[bqueue_index]);
    cout << "    (" << bqueue_index << ") BQ recebeu valores, continuando execução" << endl;
  }

  int value = Q->head->value; // Valor que será retornado (frente da fila)

  // Remove o elemento da frente
  if(Q->head == Q->last) {
    // Caso a fila esteja apenas com um elemento, libera o endereço de memória usado por esse elemento e seta head e tail como nulos
    delete Q->head;
    Q->head = NULL;
    Q->last = NULL;
  } else {
    // Caso a fila tivesse mais de um elemento, o endereço da head será selecionado, a nova head será selecionada, como o próximo da fila e libera o endereço de memória da head antiga
    Elem *e = Q->head;
    Q->head = Q->head->prox;
    delete e;
  }
  Q->bufferStatus--; // Decrementa o contador de elementos da fila

  pthread_cond_signal(&cond_prod[bqueue_index]); // Avisa que a fila teve seu tamanho reduzido

  pthread_mutex_unlock(&mutex[bqueue_index]);
  return value; // Retorna o valor da antiga head
}

// Produz elementos para uma fila bloqueante específica
void *prod(void *arg) {
  long bq_id = (long) arg; // ID da fila bloqueante
  BlockingQueue *bq = bqueue[(int) bq_id];

  // Produz elementos aleatórios(0-99) para a fila em loop infinito
  while(true) {
    int value = rand() % 100;
    cout << "Produzindo para BQ-" << bq_id << " (" << bq->bufferStatus << ") valor: " << value << endl;
    putBlockingQueue(bq, value);
    cout << "Produzido para BQ-" << bq_id << " (" << bq->bufferStatus << ") valor: " << value << endl;
  }
  pthread_exit(NULL);
}

// Consome elementos de uma fila bloqueante específica
void *cons(void *arg) {
  long bq_id = (long) arg; // ID da fila bloqueante
  BlockingQueue *bq = bqueue[(int) bq_id];

  // Consome elementos da fila em loop infinito
  while(true) {
    cout << "Consumindo da BQ-" << bq_id << " (" << bq->bufferStatus << ")" << endl;
    int value = takeBlockingQueue(bq);
    cout << "Consumido da BQ-" << bq_id << " (" << bq->bufferStatus << ") valor:" << value << endl;
  }
  pthread_exit(NULL);
}

int main() {
  for(int c = 0; c < B; c++) { // Para cada fila bloqueante
    // Inicia os mutex e cond de produtor e consumidor referentes a fila bloqueante
    pthread_mutex_init(&mutex[c], NULL);
    pthread_cond_init(&cond_prod[c], NULL);
    pthread_cond_init(&cond_cons[c], NULL);

    BlockingQueue *bq = newBlockingQueue(BSIZE); // Inicia uma fila bloqueante e pega seu endereço
    string adr = get_address(bq); // Endereço de memória da fila em string
    bqueue[c] = bq; // Coloca no array 
    bqindex[adr] = c; // Mapeamento do endereço da fila bloqueante

    cout << "Criando lista bloqueante: " << (c + 1) << endl;
  }

  // Inicia todos os threads consumidores e produtores e aguarda o fim de suas execuções
  pthread_t thread_prod[P];
  pthread_t thread_cons[C];
  for(int c = 0; c < P; c++) {
    pthread_create(&thread_prod[c], NULL, prod, (void *) prod_bq[c]);
  }
  for(int c = 0; c < C; c++) {
    pthread_create(&thread_prod[c], NULL, cons, (void *) cons_bq[c]);
  }
  for(int c = 0; c < P; c++) {
    pthread_join(thread_prod[c], NULL);
  }
  for(int c = 0; c < C; c++) {
    pthread_join(thread_cons[c], NULL);
  }

  return 0;
}
