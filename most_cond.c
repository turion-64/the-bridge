// Wersja B

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// zmienne stanu
int cars_A;
int waiting_A;
int cars_B;
int waiting_B;
int occupied = 0;
int bridge_car = -1;
int bridge_dir = 0;

// mutex i semafor
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bridge_cond = PTHREAD_COND_INITIALIZER;

void megaprint() {
  printf("A-%d %d>>> ", cars_A, waiting_A);
  if (occupied) {
    if (bridge_dir == 1)
      printf("[>> %d >>]", bridge_car);
    else
      printf("[<< %d <<]", bridge_car);
  } else {
    printf("[ -- ]");
  }
  printf(" <<<%d %d-B\n", waiting_B, cars_B);
}

void *car(void *arg) {
  int id = *(int *)arg;
  int location = 0;
  while (1) {
    usleep(500000 + rand() % 1000000);
    if (location == 0) {
      // a -> b
      // samochód podjeżdża do mostu i ustawia się w kolejce
      // blokujemy mutex i zmieniamy wartości
      pthread_mutex_lock(&print_mutex);
      cars_A--;
      waiting_A++;
      megaprint();

      // pętla warunkowa czekająca az most sie zwolni
      // tu następuje unlock, potem znowu lock
      while (occupied == 1) {
        pthread_cond_wait(&bridge_cond, &print_mutex);
      }

      // most zwolniony, autko wjeżdża
      waiting_A--;
      occupied = 1;
      bridge_car = id;
      bridge_dir = 1;
      megaprint();

      pthread_mutex_unlock(&print_mutex);

      usleep(800000); // przejazd

      //ponowna blokada na zmiane wartości
      pthread_mutex_lock(&print_mutex);

      occupied = 0;
      bridge_car = -1;
      bridge_dir = 0;
      cars_B++;
      megaprint();

      // następuje zwolnienie blokady
      pthread_cond_broadcast(&bridge_cond);
      pthread_mutex_unlock(&print_mutex);
      location = 1;
    } else {
      // b -> a
      pthread_mutex_lock(&print_mutex);
      cars_B--;
      waiting_B++;
      megaprint();

      while (occupied == 1) {
        pthread_cond_wait(&bridge_cond, &print_mutex);
      }

      waiting_B--;
      occupied = 1;
      bridge_car = id;
      bridge_dir = 2;
      megaprint();

      pthread_mutex_unlock(&print_mutex);

      usleep(800000); // przejazd

      pthread_mutex_lock(&print_mutex);

      occupied = 0;
      bridge_car = -1;
      bridge_dir = 0;
      cars_A++;
      megaprint();
      pthread_cond_broadcast(&bridge_cond);
      pthread_mutex_unlock(&print_mutex);
      location = 0;
    }
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    goto err;
  }

  int N = atoi(argv[1]);
  if (N <= 0) goto err;

  cars_A = N;
  waiting_A = 0;
  cars_B = 0;
  waiting_B = 0;

  srand(time(NULL));

  pthread_t *threads = malloc(N * sizeof(pthread_t));

  int *ids = malloc(N * sizeof(int));
  megaprint();

  for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, car, &ids[i]);
  }

  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }

  free(threads);
  free(ids);
  pthread_mutex_destroy(&print_mutex);
  pthread_cond_destroy(&bridge_cond);

  return 0;

err:
  fprintf(stderr, "Uzycie: %s <N>\n", argv[0]);
  fprintf(stderr, "Gdzie <N> to liczba samochodow (watkow).\n");
  return 1;
}
