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
#include <time.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/time.h>

void on_usr1(int signal);

int* generatePrimes(int range);

void writePrimeNumbersToFile(int* primeNumbers, int size, char* path);

void calculate_partial_sum(int id, int *primeNumbers, long long int* shared, int numberOfPrimes, int numberOfProcesses);


int main(int argc, char** argv) {

    int numberOfProcesses = argc > 1 ? atoi(argv[1]) : 2;
    int numberOfPrimeNumbers = argc > 2 ? atoi(argv[2]) : 10;
    char *path = argc > 3 ? argv[3] : "primeNumbers.txt";

	if(numberOfPrimeNumbers <= 0 || numberOfProcesses <= 0) {
		printf("Illegal arguments\n");
		return EXIT_FAILURE;
	}

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
    struct sigaction usr1;
    sigemptyset(&mask);
    usr1.sa_handler = (&on_usr1);
    usr1.sa_mask = mask;
    usr1.sa_flags = 0;
    sigaction(SIGUSR1, &usr1, NULL);

    int size = (numberOfProcesses+1) * sizeof(long long int);
    
    long long int *shared = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(shared == MAP_FAILED ) {
		printf("Error occured while creating shared memory\n");
		return EXIT_FAILURE;
    }

	pid_t pid;
    pid_t childPidNumbers[numberOfProcesses];

    int index;

	clock_t start = clock();

    for(index = 0; index < numberOfProcesses; index++) {
		pid = fork();
		if(pid < 0) {
			printf("Error occured while trying to fork\n");
	        return EXIT_FAILURE;	
		}
		else if(pid == 0) {
			pause();
			break;
		}
		else {
			childPidNumbers[index] = pid;
		}
    }
	
    if(pid == 0) {
    	calculate_partial_sum(index, primeNumbers, shared, numberOfPrimeNumbers, numberOfProcesses);
    }
    else {
		sleep(1);

    	for(int i = 0; i < numberOfProcesses; i++) {
			kill(childPidNumbers[i], SIGUSR1);
		}

		for(int i = 0; i < numberOfProcesses; i++) {
			waitpid(childPidNumbers[i], NULL, 0);
		}

		clock_t end = clock();

		double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

		long long int totalSum = 0;

		for(int i = 0; i < numberOfProcesses; i++) {
			totalSum += shared[i];
		}
		
		printf("Total sum of prime numbers: %lld\n", totalSum);

		printf("Child processes executed in: %f seconds\n", cpu_time_used);

		if (munmap(shared, size) == -1) {
            printf("Failed to unmap");
        }
    }
    
    free(primeNumbers);

    return EXIT_SUCCESS;
}

void on_usr1(int signal) {
	//Used only to wake up the process
}

void calculate_partial_sum(int id, int *primeNumbers, long long int* shared, int numberOfPrimes, int numberOfProcesses){
	int start = id * (numberOfPrimes/numberOfProcesses);
	int end = (id+1) * (numberOfPrimes/numberOfProcesses);

	if(id == numberOfProcesses -1) {
		end += numberOfPrimes % numberOfProcesses;
	}

	long long int sum = 0;

	for(int i = start; i < end; i++) {
		sum += primeNumbers[i];
	}

	shared[id] = sum;
}

int* generatePrimes(int n) {
	int upperBound = (n > 6) ? (int)(1.2 * n * (log(n) + log(log(n)))) : 15;
    bool* isPrime = malloc((upperBound+1) * sizeof(bool));
    int* primeNumbers = malloc((upperBound+1) * sizeof(int));

	if(isPrime == NULL || primeNumbers == NULL) {
		free(isPrime);
		free(primeNumbers);
		return NULL;
	}

    int count = 0;

    for (int i = 2; i <= upperBound; i++) {
        isPrime[i] = true;
    }

    for (int i = 2; i <= sqrt(upperBound); i++) {
        if (isPrime[i]) {
            for (int j = i * i; j <= upperBound; j += i) {
                isPrime[j] = false;
            }
        }
    }

    for (int i = 2; i <= upperBound && count < n; i++) {
        if (isPrime[i]) {
            primeNumbers[count++] = i;
        }
    }

	if(count < n) printf("Failed to find %d prime numbers\n", n);

    free(isPrime);

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

