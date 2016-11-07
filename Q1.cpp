#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

int t, n, f, type, amount, *price, *bad, good = 0;
pthread_mutex_t global_mutex, good_mutex, *bad_mutex;

// Lê todos os dados de um arquivo
void read(int file) {
  // Cria e abre a stream do arquivo <file>.in
  stringstream ssm;
  ssm << file << ".in";
  string filename = ssm.str(), line;
  ifstream myfile;
  myfile.open(filename.c_str());

  // Roda para todas as linhas do arquivo
  while(myfile >> type >> amount) {
    if(amount < 0) {
      // Caso as flechas estejam quebradas, da lock no mutex do index da flecha
      pthread_mutex_lock(&bad_mutex[type]);
      bad[type] += -amount;
      pthread_mutex_unlock(&bad_mutex[type]);
    } else {
      // Caso as flechas estejam boas, da lock no mutex de flecha boa
      pthread_mutex_lock(&good_mutex);
      good += amount;
      pthread_mutex_unlock(&good_mutex);
    }
  }
}

// Pega o próximo arquivo que deverá ser lido
void *thread_func(void *arg) {
  int file;
  while(true) {
    pthread_mutex_lock(&global_mutex);

    // Caso não tenha mais nenhum arquivo para ler, libera o mutex e encerra a thread
    if(n == 0) {
      pthread_mutex_unlock(&global_mutex);
      break;
    }
    file = n--; // ID do arquivo que será lido
    pthread_mutex_unlock(&global_mutex);
    read(file);
  }
  pthread_exit(NULL);
}

int main() {
  cin >> t >> n >> f;

  // Inicia os arrays que foram alocados dinamicamente
  price = new int[f];
  bad = new int[f];
  bad_mutex = new pthread_mutex_t[f];

  // Garante que os indexes estejam todos zerados (Alocando dinamicamente não podemos garantir os valores dos endereços de memória)
  memset(&price[0], 0, sizeof(price));
  memset(&bad[0], 0, sizeof(bad));

  // Inicia os mutexes simples
  pthread_mutex_init(&global_mutex, NULL);
  pthread_mutex_init(&good_mutex, NULL);

  // Le os preços dos tipos de flecha e inicia os mutexes respectivos
  cout << "Digite o custo de conserto das flechas de 0 a F - 1, respectivamente, separados por linha ou espaço:" << endl;
  for(int c = 0; c < f; c++) {
    cin >> price[c];
    pthread_mutex_init(&bad_mutex[c], NULL);
  }

  // Cria as threads e aguardo o final da execução de cada uma
  pthread_t thread[t];
  for(int c = 0; c < t; c++) {
    pthread_create(&thread[c], NULL, thread_func, NULL);
  }
  for(int c = 0; c < t; c++) {
    pthread_join(thread[c], NULL);
  }

  // Formata a saida
  cout << good << " flechas em bom estado!" << endl << endl;
  for(int c = 0; c < f; c++) {
    cout << "Custo de consertar as flechas de tipo " << c << ": R$ " << (bad[c] * price[c]) << ",00" << endl;
  }

  // Libera os endereços das memórias dos arrays dinâmicos
  delete [] price, bad, bad_mutex;

  return 0;
}
