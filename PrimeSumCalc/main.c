#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>

void on_usr1(int signal) {
    // Signal handler intentionally left blank
}

int* generatePrimes(int range);
void writePrimeNumbersToFile(int* primeNumbers, int size, char* path);
void calculate_partial_sum(int id, int *primeNumbers, int* shared, int numberOfPrimes, int numberOfProcesses);

int main(int argc, char** argv) {
    int numberOfProcesses = argc > 1 ? atoi(argv[1]) : 2;
    int numberOfPrimeNumbers = argc > 2 ? atoi(argv[2]) : 10;
    char *path = argc > 3 ? argv[3] : "primeNumbers.txt";

    if(numberOfProcesses > numberOfPrimeNumbers) {
        printf("Number of processes cannot be higher than the number of prime numbers\n");
        return EXIT_FAILURE;
    }

    int *primeNumbers = generatePrimes(numberOfPrimeNumbers);
    if(primeNumbers == NULL) {
        printf("Failed to allocate memory\n");
        return EXIT_FAILURE;
    }
    writePrimeNumbersToFile(primeNumbers, numberOfPrimeNumbers, path);
    sigset_t mask;
    struct sigaction sa;
    sigemptyset(&mask);
    sa.sa_handler = (&on_usr1);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    key_t key = ftok("primeNumbers.txt", 'R');
    int size = numberOfProcesses * sizeof(int);
    int shmid = shmget(key, size, 0666 | IPC_CREAT);
    if(shmid == -1) {
        printf("Error occurred while trying to create shared memory\n");
        free(primeNumbers);
        return EXIT_FAILURE;
    }

    int *shared = (int *)shmat(shmid, NULL, 0);
    if(shared == (void *)-1) {
        printf("Error occurred while attaching memory segment to process\n");
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }
    int *shm;
    pid_t childPids[numberOfProcesses];
    for(int i = 0; i < numberOfProcesses; i++) {
        pid_t pid = fork();
        if(pid < 0) {
            printf("Error occurred while trying to fork\n");
            return EXIT_FAILURE;
        } else if(pid == 0) { // Child process
	    shmid = shmget(key, size, 0666);
	    shm = shmat(key, size, IPC_CREAT | 0666);
            pause(); // Wait for signal from parent
            
            shmdt(shared);
            exit(EXIT_SUCCESS);
        } else {
            childPids[i] = pid;
        }
    }
    if(pid == 0){
	calculate_partial_sum(i, primeNumbers, shared, numberOfPrimeNumbers, numberOfProcesses);
    }
    else {
	for(int i = 0; i < numberOfProcesses; i++) {
        	kill(childPids[i], SIGUSR1);
    	}

    	for(int i = 0; i < numberOfProcesses; i++) {
        	waitpid(childPids[i], NULL, 0); // Wait for all child processes to finish
    	}
	int totalSum = 0;
    	for(int i = 0; i < numberOfProcesses; i++) {
        	totalSum += shared[i];
    	}
    	printf("Total sum of prime numbers: %d\n", totalSum);
    	}
    
    

    

    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    free(primeNumbers);

    return EXIT_SUCCESS;
}


void calculate_partial_sum(int id, int *primeNumbers, int* shared, int numberOfPrimes, int numberOfProcesses){
	int start = id * (numberOfPrimes/numberOfProcesses);
	int end = (id+1) * (numberOfPrimes/numberOfProcesses);
	printf("Start: %d, End: %d\n", start, end);
	if(id == numberOfProcesses -1) {
		end += numberOfPrimes % numberOfProcesses;
	}
	int sum = 0;
	for(int i = start; i < end; i++) {
		sum += primeNumbers[i];
	}
	shared[id] = sum;
	printf("output: %d\n", sum);
}

int* generatePrimes(int n) {
	int *primeNumbers = (int*)malloc(n*sizeof(int));
	if(primeNumbers == NULL) {
		return NULL;
	}
	primeNumbers[0] = 2;
	int curr = 3;
	int index = 1;
	while( index < n) {
		bool isPrime = true;
		int limit = sqrt(curr);
		for(int i =2; i <= limit; i++) {
			if(curr % i == 0) {
				isPrime = false;
				break;
			}
		}
		if(isPrime) {
			primeNumbers[index++] = curr;
		}
		curr+=2;
	}
	return primeNumbers;
}

void writePrimeNumbersToFile(int *primeNumbers, int size, char *path) {
	FILE *f = fopen(path, "w+");
	if(f == NULL) {
		printf("Problem with the file occured\n");
		return;
	}
	for(int i = 0; i < size; i++) {
		fprintf(f, "%d ", primeNumbers[i]);
	}
	fclose(f);
}
