/*Wersja A: Tylko Mutexy i Semafory (bez zmiennych warunkowych)
W tej wersji używamy sem_t (semafora) o wartości początkowej 1 do ochrony samego mostu
(ponieważ ma on pojemność dokładnie 1 samochodu). 
Używamy również pthread_mutex_t do ochrony sekcji krytycznej, 
w której zmieniamy stan zmiennych i wypisujemy go na ekran (aby komunikaty nie nakładały się na siebie).*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Zmienne stanu
int cars_in_A;
int waiting_A;
int cars_in_B;
int waiting_B;
int bridge_occupied = 0;
int bridge_car = -1;
int bridge_dir = 0; // 1 dla A->B, 2 dla B->A

// Narzędzia synchronizacji
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t bridge_sem;

// Funkcja wypisująca stan - MUSI być wywoływana wewnątrz zamkniętego mutexa
void print_state() {
    printf("A-%d %d>>> ", cars_in_A, waiting_A);
    if (bridge_occupied) {
        if (bridge_dir == 1) {
            printf("[>> %d >>]", bridge_car);
        } else {
            printf("[<< %d <<]", bridge_car);
        }
    } else {
        printf("[   --   ]");
    }
    printf(" <<<%d %d-B\n", waiting_B, cars_in_B);
}

void* car_thread(void* arg) {
    int id = *(int*)arg;
    int location = 0; // 0 = Miasto A, 1 = Miasto B

    while (1) {
        // 1. Przebywanie w mieście (odpoczynek/jazda lokalna)
        usleep(500000 + rand() % 1000000); // 0.5s do 1.5s

        if (location == 0) {
            // --- JAZDA Z A DO B ---
            
            // Samochód podjeżdża do mostu i ustawia się w kolejce
            pthread_mutex_lock(&print_mutex);
            cars_in_A--;
            waiting_A++;
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Czeka na zwolnienie mostu
            sem_wait(&bridge_sem);

            // Wjeżdża na most
            pthread_mutex_lock(&print_mutex);
            waiting_A--;
            bridge_occupied = 1;
            bridge_car = id;
            bridge_dir = 1;
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Przejazd przez most
            usleep(800000); 

            // Zjeżdża z mostu do miasta B
            pthread_mutex_lock(&print_mutex);
            bridge_occupied = 0;
            bridge_car = -1;
            bridge_dir = 0;
            cars_in_B++;
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Zwalnia most dla innych
            sem_post(&bridge_sem);
            location = 1;

        } else {
            // --- JAZDA Z B DO A ---
            
            // Samochód podjeżdża do mostu i ustawia się w kolejce
            pthread_mutex_lock(&print_mutex);
            cars_in_B--;
            waiting_B++;
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Czeka na zwolnienie mostu
            sem_wait(&bridge_sem);

            // Wjeżdża na most
            pthread_mutex_lock(&print_mutex);
            waiting_B--;
            bridge_occupied = 1;
            bridge_car = id;
            bridge_dir = 2; // B do A
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Przejazd przez most
            usleep(800000);

            // Zjeżdża z mostu do miasta A
            pthread_mutex_lock(&print_mutex);
            bridge_occupied = 0;
            bridge_car = -1;
            bridge_dir = 0;
            cars_in_A++;
            print_state();
            pthread_mutex_unlock(&print_mutex);

            // Zwalnia most
            sem_post(&bridge_sem);
            location = 0;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <N>\n", argv[0]);
        fprintf(stderr, "Gdzie <N> to liczba samochodow (watkow).\n");
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) N = 5; // Domyślna bezpieczna wartość

    // Inicjalizacja stanu
    cars_in_A = N;
    waiting_A = 0;
    cars_in_B = 0;
    waiting_B = 0;

    srand(time(NULL));
    sem_init(&bridge_sem, 0, 1); // Semafor o wartości 1 (pojemność mostu)

    pthread_t* threads = malloc(N * sizeof(pthread_t));
    int* ids = malloc(N * sizeof(int));

    // Wypisanie stanu początkowego
    print_state();

    for (int i = 0; i < N; i++) {
        ids[i] = i; // Identyfikatory od 0 do N-1
        pthread_create(&threads[i], NULL, car_thread, &ids[i]);
    }

    // Oczekiwanie na wątki (w tym wariancie pętla jest nieskończona)
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(ids);
    sem_destroy(&bridge_sem);
    return 0;
}
