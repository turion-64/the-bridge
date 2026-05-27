/*Wersja B: Ze Zmiennymi Warunkowymi (Condition Variables)
W tej wersji rezygnujemy z semaforów na rzecz pthread_cond_t. Dzięki temu cały
cykl sprawdzania dostępu i modyfikacji stanu zamyka się w obrębie jednego
zabezpieczonego mutexem bloku.*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Zmienne stanu
int cars_in_A;
int waiting_A;
int cars_in_B;
int waiting_B;
int bridge_occupied = 0;
int bridge_car = -1;
int bridge_dir = 0;

// Narzędzia synchronizacji
pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bridge_cond = PTHREAD_COND_INITIALIZER;

void print_state() {
  printf("A-%d %d>>> ", cars_in_A, waiting_A);
  if (bridge_occupied) {
    if (bridge_dir == 1)
      printf("[>> %d >>]", bridge_car);
    else
      printf("[<< %d <<]", bridge_car);
  } else {
    printf("[ -- ]");
  }
  printf(" <<<%d %d-B\n", waiting_B, cars_in_B);
}

void *car_thread(void *arg) {
  int id = *(int *)arg;
  int location = 0;
  while (1) {
    usleep(500000 + rand() % 1000000);
    if (location == 0) {
      // --- JAZDA Z A DO B ---
      pthread_mutex_lock(&monitor_mutex);
      cars_in_A--;
      waiting_A++;
      print_state();

      // Pętla warunkowa: czekaj dopóki most jest zajęty
      while (bridge_occupied == 1) {
        pthread_cond_wait(&bridge_cond, &monitor_mutex);
      }

      // Most zwolniony, wjeżdżamy
      waiting_A--;
      bridge_occupied = 1;
      bridge_car = id;
      bridge_dir = 1;
      print_state();

      pthread_mutex_unlock(&monitor_mutex);

      usleep(800000); // Przejazd

      pthread_mutex_lock(&monitor_mutex);

      bridge_occupied = 0;
      bridge_car = -1;
      bridge_dir = 0;
      cars_in_B++;
      print_state();

      // Sygnalizowanie innym wątkom (samochodom), że most jest
      wolny pthread_cond_broadcast(&bridge_cond);
      pthread_mutex_unlock(&monitor_mutex);
      location = 1;
    } else {
      // --- JAZDA Z B DO A ---
      pthread_mutex_lock(&monitor_mutex);
      cars_in_B--;
      waiting_B++;
      print_state();

      while (bridge_occupied == 1) {
        pthread_cond_wait(&bridge_cond, &monitor_mutex);
      }

      waiting_B--;
      bridge_occupied = 1;
      bridge_car = id;
      bridge_dir = 2;
      print_state();

      pthread_mutex_unlock(&monitor_mutex);

      usleep(800000); // Przejazd

      pthread_mutex_lock(&monitor_mutex);

      bridge_occupied = 0;
      bridge_car = -1;
      bridge_dir = 0;
      cars_in_A++;
      print_state();
      pthread_cond_broadcast(&bridge_cond);
      pthread_mutex_unlock(&monitor_mutex);
      location = 0;
    }
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Uzycie: %s <N>\n", argv[0]);
    return 1;
  }

  int N = atoi(argv[1]);
  if (N <= 0)
    N = 5;

  cars_in_A = N;
  waiting_A = 0;
  cars_in_B = 0;
  waiting_B = 0;
  
  srand(time(NULL));
  
  pthread_t *threads = malloc(N * sizeof(pthread_t));
  
  int *ids = malloc(N * sizeof(int));
  print_state();
  
  for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, car_thread, &ids[i]);
  }
  
  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }
  
  free(threads);
  free(ids);
  pthread_mutex_destroy(&monitor_mutex);
  pthread_cond_destroy(&bridge_cond);
  
  return 0;
}
