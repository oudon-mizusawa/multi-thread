#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRIME_NUMBERS 10000

int primeNumbers[MAX_PRIME_NUMBERS];
int nPrimeNumber;
int primeNumberChecked;
pthread_t usingPrimeNumber;

int isPrimeNumber(int n) {
    int i;
    for (i = 0; i < nPrimeNumber; i++) {
        if (primeNumbers[i] > n/2)
            return 1;
        if (n%primeNumbers[i] == 0)
            return 0;
    }
    return 1;
}

void generetePrimeNumbers(int n) {
    int i;

    pthread_mutex_lock(&usingPrimeNumber);
    if (n <= primeNumberChecked) {
        pthread_mutex_unlock(&usingPrimeNumber);
        return;
    }
    
    for (i = primeNumberChecked +1; i <= n; i++) {
        if (isPrimeNumber(i)) {
            if (nPrimeNumber == MAX_PRIME_NUMBERS) {
                printf("too many prime numbers. \n");
                exit(1);
            }
            primeNumbers[nPrimeNumber] = i;
            nPrimeNumber++;
        }
    }
    primeNumberChecked = n;
    pthread_mutex_unlock(&usingPrimeNumber);
    return;
}

int countPrimeNumbers(int n) {
    int count, i;
    generetePrimeNumbers(n);
    count = 0;
    for (i = 0; i < nPrimeNumber; i++) {
        if (primeNumbers[i] > n)
            break;
        count++;
    }
    return count;
}

void *threadFunc(void *arg) {
    int n = (int)arg;
    int x;

    x = countPrimeNumbers(n);
    printf("number of prime numbers under %d is %d\n" , n, x);
    return NULL;
}

// How to compile is below here. added warning ignore option.
// "gcc fastPrimeNumber.c -o firstThread -lpthread -w"   
int main() {
    int numberList[6] = {1, 10, 100, 1000, 10000, 100000};
    pthread_t threads[6];
    int i;
    pthread_mutex_init(&usingPrimeNumber, NULL);

    nPrimeNumber = 0;
    primeNumberChecked = 1;
    for (i = 0; i < 6; i++) {
        if (pthread_create(&threads[i], NULL, threadFunc, (void *)numberList[i]) != 0) {
            printf("cant create thread %d\n", i);
            exit(1);
        }   
    }

    for (i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);  
    }

        pthread_mutex_destroy(&usingPrimeNumber);

    printf("DONE\n");
    return 0;
}