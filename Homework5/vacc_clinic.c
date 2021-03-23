/*
 * Your info here.
 */

// TODO: Something is wrong with buffer stuff (client 1 assigned to station 0...);

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_VIALS 30 // 30
#define SHOTS_PER_VIAL 6 // 6
#define NUM_CLIENTS (NUM_VIALS * SHOTS_PER_VIAL)
#define NUM_NURSES 10 // 10
#define NUM_STATIONS NUM_NURSES
#define NUM_REGISTRATIONS_SIMULTANEOUSLY 4 // 4
#define WALK_DELAYS 0 // 1 = true, 0 = false

// bounded buffer struct
typedef struct {
    int buff[NUM_STATIONS];
    int next_in, next_out, count;
} bounded_buffer_t;

/* global variables */
int availableVials = NUM_VIALS;
bounded_buffer_t nurseStations;

// semaphores
sem_t nurseReadySem;
sem_t clientReadySem;
sem_t registrationDeskSem;
sem_t numVialsMutexSem;
sem_t buffMutexSem;
sem_t empty;
sem_t full;

sem_t vaccinationReadySemArray[NUM_STATIONS];
sem_t vaccinationCompleteSemArray[NUM_STATIONS];

// threads
pthread_t clients[NUM_CLIENTS];
pthread_t nurses[NUM_NURSES];

void produceIntoStationAssignmentQ(int value) {
    nurseStations.buff[nurseStations.next_in] = value;
    nurseStations.next_in = (nurseStations.next_in + 1) % NUM_STATIONS;
    nurseStations.count++;
}

int consumeFromStationAssignmentQ() {
    int value = nurseStations.buff[nurseStations.next_out];
    nurseStations.next_out = (nurseStations.next_out + 1) % NUM_STATIONS;
    nurseStations.count--;
    return value;
}

void initializeBuffer(bounded_buffer_t* buff) {
    buff->next_in = buff->next_out = buff->count = 0;
    sem_init(&empty, 0, NUM_STATIONS);
    sem_init(&full, 0, 0);
    sem_init(&buffMutexSem, 0, 1);
}

int get_rand_in_range(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

char *time_str;
char *curr_time_s() {
    time_t t;
    time(&t);
    time_str = ctime(&t);
    // replace ending newline with end-of-string.
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

void waitSeconds(int microseconds) {
    if (WALK_DELAYS)
        usleep(microseconds * 1000000);
    else
        usleep(microseconds);
}

// lower and upper are in microseconds.
void walk(int lower, int upper) {
    waitSeconds(get_rand_in_range(lower, upper));
}

// function for the nurses to grab a vial
// returns 1 (true) if nurse was able to get a vial
// returns 0 (false) if there are no vials left.
int _nurseGetVial(long int id) {

    fprintf(stderr, "%s: nurse %ld is walking to get a vial...\n", curr_time_s(), id);

    // Walk to get a vile
    walk(1, 3);

    fprintf(stderr, "%s: nurse %ld waiting to get a vial\n", curr_time_s(), id);
    sem_wait(&numVialsMutexSem);

    if (availableVials == 0) {
        fprintf(stderr, "%s: nurse %ld had no vials available and therefore left the clinic\n", curr_time_s(), id);
        sem_post(&numVialsMutexSem); // unlock vial mutex so other nurses can get in
        return 0;
    }

    availableVials--; // take a vial

    fprintf(stderr, "%s: nurse %ld has aquired a vial and is walking back to their station...\nThere are %d vials left\n", curr_time_s(), id, availableVials);

    sem_post(&numVialsMutexSem); // signal that you've taken a vial

    // Walk back to station
    walk(1, 3);

    return 1;
}

// arg is the nurses station number.
void *nurse(void *arg) {
    long int id = (long int)arg;
    long int stationId = id;

    fprintf(stderr, "%s: nurse %ld has arrived!\n", curr_time_s(), id);


    // Keep vaccinating clients while there are vials available.
    while (_nurseGetVial(id)) {
        fprintf(stderr, "%s: nurse %ld has arrived back to their station with a vial!\n", curr_time_s(), id);

        int shotsLeftInVial = SHOTS_PER_VIAL;

        /* Vaccinate Clients! */
        while (shotsLeftInVial > 0) {
            // produce into the station assignment q to signal that you're ready for a client
            sem_wait(&empty);
                sem_wait(&buffMutexSem); 
                    produceIntoStationAssignmentQ(stationId);
                sem_post(&buffMutexSem);
            sem_post(&full);
            fprintf(stderr, "%s: nurse %ld waiting for client...\n", curr_time_s(), id);

            // wait for a client to come to your station
            sem_wait(&vaccinationReadySemArray[stationId]);

            // vaccinate client
            fprintf(stderr, "%s: nurse %ld is vaccinating a client!\n", curr_time_s(), id);
            // wait 5 seconds to vaccinate client
            waitSeconds(5);

            // signal to the client that you're done vaccinating them
            sem_post((&vaccinationCompleteSemArray[stationId]));

            fprintf(stderr, "%s: nurse %ld successfully vaccinated a client!\n", curr_time_s(), id);
            shotsLeftInVial--;
        }
    } // after vial is empty...
    fprintf(stderr, "%s: there are no more vials! Nurse %ld is done for the day!\n", curr_time_s(), id);
    pthread_exit(NULL);
}

void *client(void *arg) {
    long int id = (long int)arg;

    /* Register */
    fprintf(stderr, "%s: client %ld has arrived and is walking to registration desk...\n", curr_time_s(), id);

    // walk to registration desk
    walk(3, 10);

    fprintf(stderr, "%s: client %ld is waiting for a spot at the registration desk...\n", curr_time_s(), id);
    sem_wait(&registrationDeskSem);
    fprintf(stderr, "%s: client %ld is registering at the desk...\n", curr_time_s(), id);

    // take some time to register
    walk(3, 10);

    sem_post(&registrationDeskSem);
    fprintf(stderr, "%s: client %ld is registered and waiting to be assigned a station...\n", curr_time_s(), id);


    /* Station Assignment */
    // walk to station assignment q
    walk(3, 10);

    // get assigned to a station
    sem_wait(&full);
            sem_wait(&buffMutexSem);
                int station = consumeFromStationAssignmentQ();
            sem_post(&buffMutexSem);
    sem_post(&empty);
    fprintf(stderr, "%s: client %ld has been assigned to station %d!\n", curr_time_s(), id, station);

    /* Get vaccinated! */
    fprintf(stderr, "%s: client %ld is walking to station %d\n", curr_time_s(), id, station);

    // walk to nurses station
    walk(1, 2);
    
    sem_post(&vaccinationReadySemArray[station]); // post to the next ready nurse that you're ready for the vaccination
    fprintf(stderr, "%s: client %ld is sitting down, getting vaccinated...\n", curr_time_s(), id);
    sem_wait(&vaccinationCompleteSemArray[station]); // wait for vaccination to complete

    /* Leave */
    fprintf(stderr, "%s: client %ld is vaccinated and leawves the clinic! - See you soon!\n", curr_time_s(), id);

    pthread_exit(NULL);
}

void initializeSemaphores() {
    // initialize semaphores and semaphore arrays
    sem_init(&clientReadySem, 0, 0);
    sem_init(&registrationDeskSem, 0, NUM_REGISTRATIONS_SIMULTANEOUSLY);
    sem_init(&numVialsMutexSem, 0, 1);
    
    // initialize all of the station semaphores
    // these determine the rendezvous between the nurse and client
    for (int i = 0; i < NUM_STATIONS; i++) {
        sem_init(&vaccinationCompleteSemArray[i], 0, 0);
        sem_init(&vaccinationReadySemArray[i], 0, 0);
    }
}

void initializeClientsAndNurses() {
    // initialize the nurse threads
    for (long int i = 0; i < NUM_NURSES; i++) {
        pthread_create(&nurses[i], NULL, nurse, (void*) (i));
    }

    // initialize the client threads
    for (long int j = 0; j < NUM_CLIENTS; j++) {
        pthread_create(&clients[j], NULL, client, (void*) (j));
        walk(0, 1);
    }
}

int main() {
    srand(time(0));

    initializeSemaphores();

    initializeBuffer(&nurseStations);

    initializeClientsAndNurses();

    printf("Vaccine clinic closing for the day\n");

    pthread_exit(0);
}
