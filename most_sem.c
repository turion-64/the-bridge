// Wersja A

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// zmienne stanu
int cars_A;
int waiting_A;
int cars_B;
int waiting_B;
int occupied = 0;
int bridge_car = -1;
int bridge_dir = 0; // 1 dla A->B, 2 dla B->A, 0 kiedy nikogo nie ma na moście

// narzędzia synchronizacji
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t bridge_sem;

// funkcja wypisująca stan
void megaprint() {
    printf("A-%d %d>>> ", cars_A, waiting_A);
    if (occupied) {
        if (bridge_dir == 1) {
            printf("[>> %d >>]", bridge_car);
        } else {
            printf("[<< %d <<]", bridge_car);
        }
    } else {
        printf("[   --   ]");
    }
    printf(" <<<%d %d-B\n", waiting_B, cars_B);
}

void* car(void* arg) {
    int id = *(int*)arg;
    int location = 0; // 0 = Miasto A, 1 = Miasto B

    while (1) {
        // jazda po mieście
        usleep(500000 + rand() % 1000000); // 0.5s do 1.5s

        if (location == 0) {
            // a -> b

            // samochód podjeżdża do mostu i ustawia się w kolejce
            // blokujemy mutex i zmieniamy wartości
            pthread_mutex_lock(&print_mutex);
            cars_A--;
            waiting_A++;
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // czeka na zwolnienie blokady
            sem_wait(&bridge_sem);

            // wjeżdża na most
            pthread_mutex_lock(&print_mutex);
            waiting_A--;
            occupied = 1;
            bridge_car = id;
            bridge_dir = 1;
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // przejazd przez most
            usleep(800000);

            // zjeżdża z mostu do miasta B
            pthread_mutex_lock(&print_mutex);
            occupied = 0;
            bridge_car = -1;
            bridge_dir = 0;
            cars_B++;
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // następuje zwolnienie blokady
            sem_post(&bridge_sem);
            location = 1;

        } else {
            // b -> a

            // samochód podjeżdża do mostu i ustawia się w kolejce
            pthread_mutex_lock(&print_mutex);
            cars_B--;
            waiting_B++;
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // czeka na zwolnienie blokady
            sem_wait(&bridge_sem);

            // wjeżdża na most
            pthread_mutex_lock(&print_mutex);
            waiting_B--;
            occupied = 1;
            bridge_car = id;
            bridge_dir = 2; // B do A
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // przejazd przez most
            usleep(800000);

            // zjeżdża z mostu do miasta A
            pthread_mutex_lock(&print_mutex);
            occupied = 0;
            bridge_car = -1;
            bridge_dir = 0;
            cars_A++;
            megaprint();
            pthread_mutex_unlock(&print_mutex);

            // następuje zwolnienie blokady
            sem_post(&bridge_sem);
            location = 0;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        goto err;
    }

    int N = atoi(argv[1]);
    if (N <= 0) goto err;

    // domyślny stan to 5 aut w mieście A
    cars_A = N;
    waiting_A = 0;
    cars_B = 0;
    waiting_B = 0;

    srand(time(NULL));
    sem_init(&bridge_sem, 0, 1); // pojemność mostu 1

    pthread_t* threads = malloc(N * sizeof(pthread_t));
    int* ids = malloc(N * sizeof(int));

    // wypisanie stanu początkowego
    megaprint();

    for (int i = 0; i < N; i++) {
        ids[i] = i; // samochody numerowane są od zera
        pthread_create(&threads[i], NULL, car, &ids[i]);
    }

    // oczekiwanie na wątki
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(ids);
    sem_destroy(&bridge_sem);
    return 0;

err:
    fprintf(stderr, "Uzycie: %s <N>\n", argv[0]);
    fprintf(stderr, "Gdzie <N> to liczba samochodow (watkow).\n");
    return 1;
}
