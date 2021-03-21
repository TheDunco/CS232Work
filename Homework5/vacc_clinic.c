/*
 * Your info here.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_VIALS 30 // 30
#define SHOTS_PER_VIAL 6
#define NUM_CLIENTS (NUM_VIALS * SHOTS_PER_VIAL)
#define NUM_NURSES 10
#define NUM_STATIONS NUM_NURSES
#define NUM_REGISTRATIONS_SIMULTANEOUSLY 4
#define WALK_DELAYS 1

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
sem_t numVialsSem;
sem_t buffMutexSem;
sem_t empty;
sem_t full;

sem_t vaccinationReadySemArray[NUM_STATIONS];
sem_t vaccinationCompleteSemArray[NUM_STATIONS];


void produceIntoStationAssignmentQ(int value) {
    nurseStations.buff[nurseStations.next_in] = value;
    nurseStations.next_in = (nurseStations.next_in + 1) % NUM_STATIONS;
    nurseStations.count++;
}

int consumeFromStationAssignmentQ() {
    int value = nurseStations.buff[nurseStations.next_out];
    nurseStations.next_out = (nurseStations.next_out + 1) % NUM_STATIONS;
    nurseStations.count--;
}

void initBuffer(bounded_buffer_t* buff) {
    buff->next_in = buff->next_out = buff->count = 0;
    sem_init(&buffMutexSem, 0, 0);
    sem_init(&empty, 0, 1);
    sem_init(&full, 0, 0);

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
    usleep(microseconds * 1000000);
}

// lower and upper are in seconds.
void walk(int lower, int upper) {
    waitSeconds(get_rand_in_range(lower, upper));
}

// arg is the nurses station number.
void *nurse(void *arg) {
    long int id = (long int)arg;
    long int stationId = id;
    int shotsLeftInVial = SHOTS_PER_VIAL;


    /* Get Vial */
    fprintf(stderr, "%s: nurse %ld started and walking to get a vial...\n", curr_time_s(), id);

    // Walk to get a vile
    if (WALK_DELAYS)
        walk(1, 3);

    fprintf(stderr, "%s: nurse %ld waiting to get a vial\n", curr_time_s(), id);
    sem_wait(&numVialsSem);

    if (availableVials == 0) {
        fprintf(stderr, "%s: nurse %ld had no vials available and therefore left the clinic\n", curr_time_s(), id);
        pthread_exit(NULL);
    }

    availableVials--; // take a vial

    sem_post(&numVialsSem); // signal that you've taken a vial

    fprintf(stderr, "%s: nurse %ld has aquired a vial and is walking back to their station...\nThere are %d vials left\n", curr_time_s(), id, availableVials);

    // Walk back to station
    if (WALK_DELAYS)
        walk(1, 3);

    fprintf(stderr, "%s: nurse %ld has arrived back to their station with a vial!\n", curr_time_s(), id);


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
    // after vial is empty...
    fprintf(stderr, "%s: nurse %ld is done for the day!\n", curr_time_s(), id);

    pthread_exit(NULL);
}

void *client(void *arg) {
    long int id = (long int)arg;

    /* Register */
    fprintf(stderr, "%s: client %ld has arrived and is walking to registration desk...\n", curr_time_s(), id);
    // walk to registration desk
    if (WALK_DELAYS)
        walk(3, 10);

    fprintf(stderr, "%s: client %ld is waiting for a spot at the registration desk...\n", curr_time_s(), id);
    sem_wait(&registrationDeskSem);
    fprintf(stderr, "%s: client %ld is registering at the desk...\n", curr_time_s(), id);

    // take some time to register
    if (WALK_DELAYS)
        walk(3, 10);

    sem_post(&registrationDeskSem);
    fprintf(stderr, "%s: client %ld is registered and waiting to get vaccinated...\n", curr_time_s(), id);


    /* Station Assignment */
    // walk to station assignment q
    if (WALK_DELAYS)
        walk(3, 10);

    // get assigned to a station
    sem_wait(&full);
            sem_wait(&buffMutexSem);
                int station = consumeFromStationAssignmentQ();
            sem_post(&buffMutexSem);
    sem_post(&empty);
    fprintf(stderr, "%s: client %ld has been assigned to station %d\n", curr_time_s(), id, station);


    /* Get vaccinated! */
    // walk to nurses station
    if (WALK_DELAYS)
        walk(1, 2);
    
    sem_post(&vaccinationReadySemArray[station]); // post to the nurse that you're ready for the vaccination
    sem_wait(&vaccinationCompleteSemArray[station]); // wait for vaccination to complete

    /* Leave */
    fprintf(stderr, "%s: client %ld leaves the clinic!\n", curr_time_s(), id);

    pthread_exit(NULL);
}

int main() {
    srand(time(0));

    // initialize semaphores
    sem_init(&clientReadySem, 0, 0);
    sem_init(&registrationDeskSem, 0, NUM_REGISTRATIONS_SIMULTANEOUSLY);
    sem_init(&numVialsSem, 0, 1);

    sem_init(&empty, 0, NUM_STATIONS);
    sem_init(&full, 0, 0);
    sem_init(&buffMutexSem, 0, 1);

    for (int i = 0; i < NUM_STATIONS; i++) {
        sem_init(&vaccinationCompleteSemArray[i], 0, 0);
        sem_init(&vaccinationReadySemArray[i], 0, 0);
    }


    pthread_t clientThread;
    pthread_t nurseThread;

    pthread_create(&clientThread, NULL, client, (void*) (1));
    pthread_create(&nurseThread, NULL, nurse, (void*) (1));

    pthread_exit(0);
}
