#include <bits/stdc++.h>
#include <pthread.h>
// Tamanho do sistema linear
#define M 4
// Quantidade de execuções (quanto maior, mais preciso será o resultado)
#define P 10
using namespace std;

pthread_barrier_t barrier;

vector<int> work[4]; // Tarefas que serão executadas por cada thread
double a[M][M] = {{10, -1, 2, 0}, {-1, 11, -1, 3}, {2, -1, 10, -1}, {0, 3, -1, 8}}; // Equações
double b[M] = {6, 25, -11, 15}; // Resultados das equações
double x[M];
double cx[M] = {1, 1, 1, 1}; // Valor inicial associados aos x1, x2, ..., xi, ..., xM

// Executa todas as tarefas associadas a thread
void *function(void *arg) {
  long thread_id = (long) arg;
  for(int c = 0; c < work[thread_id].size(); c++) {
    int i = work[thread_id][c]; // ID da tarefa (xi que será calculado)

    // Soma valor atual com o coeficiente multiplicado pelo valor de xi, para todos os elementos diferentes do próprio
    double sum = 0;
    for(int j = 0; j < M; j++) {
      if(i != j) {
        sum += a[i][j] * cx[j];
      }
    }

    // xi recebe o valor do resultado da equação, subtraido pelo somatório dos outros coeficientes com seus respectivos xj e tudo dividido pelo coeficiente de xi
    x[i] = (b[i] - sum) / a[i][i];
  }

  // Indica que o thread chegou na barreira
  pthread_barrier_wait(&barrier);
  pthread_exit(NULL);
}

int main() {
  // Le o input da quantidade de threads
  int n;
  cout << "Digite a quantidade de Threads: " << endl;
  cin >> n;
  n = min(n, M); // Caso tenham menos linhas que thread, só podemos usar um thread por linha
  pthread_t threads[n];

  for(int k = 0; k < P; k++) { // Execução do cálculo de todos os x
    pthread_barrier_init(&barrier, NULL, n); // Inicia a barreira

    // Associa as tarefas para as threas, balanceando uma tarefa por task
    for(int i = 0, cc = 0; i < M; i++, cc++) {
      if(cc == n) cc = 0; // Reseta o contador da thread, para garantir o balanceamento
      work[cc].push_back(i);
    }

    // Inicia e aguarda o fim da execução das threads
    for(long i = 0; i < n; i++) {
      pthread_create(&threads[i], NULL, function, (void *) i);
    }
    for(int i = 0; i < n; i++) {
      pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    // Copia os resultados calculados para usar na próxima execução
    for(int c = 0; c < M; c++) {
      cx[c] = x[c];
    }

    // Output teste para ver os valores aproximados ao final de cada execução
    cout << "P  = " << (k + 1) << endl;
    for(int i = 0; i < M; i++) {
      cout << "X" << i << " = " << x[i] << endl;
    }
    cout << endl;
  }

  return 0;
}
