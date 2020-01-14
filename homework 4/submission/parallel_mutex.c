#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];

//  // declare a lock
// pthread_mutex_t mLock;
pthread_mutex_t mLock1;
pthread_mutex_t mLock2;
pthread_mutex_t mLock3;
pthread_mutex_t mLock4;
pthread_mutex_t mLock5;


typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;

    // acquire lock
    // pthread_mutex_lock(&mLock);
    if(i == 0)
    	pthread_mutex_lock(&mLock1);
    else if(i == 1)
    	pthread_mutex_lock(&mLock2);
    else if(i == 2)
    	pthread_mutex_lock(&mLock3);
    else if(i == 3)
    	pthread_mutex_lock(&mLock4);
    else if(i == 4)
    	pthread_mutex_lock(&mLock5); 

    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

    // release lock
    //pthread_mutex_unlock(&mLock); 
    if(i == 0)
        pthread_mutex_unlock(&mLock1);
    else if(i == 1)
        pthread_mutex_unlock(&mLock2);
    else if(i == 2)
        pthread_mutex_unlock(&mLock3);
    else if(i == 3)
        pthread_mutex_unlock(&mLock4);
    else if(i == 4)
        pthread_mutex_unlock(&mLock5);
}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }

    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        if (retrieve(keys[key]) == NULL) lost++;
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }

    // initialize the lock
    // pthread_mutex_init(&mLock, NULL); 
    pthread_mutex_init(&mLock1, NULL); 
    pthread_mutex_init(&mLock2, NULL); 
    pthread_mutex_init(&mLock3, NULL); 
    pthread_mutex_init(&mLock4, NULL); 
    pthread_mutex_init(&mLock5, NULL); 

    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }
    
    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}


/* ---------------------------answers--------------------------------------
1)Yes, there is overhead for using mutex

2)Spinlock is faster than mutex.The fundamental difference between spinlock and mutex is that spinlock keeps checking the lock (busy waiting), while mutex puts threads waiting for the lock into sleep (blocked). A busy-waiting thread wastes CPU cycles, while a blocked thread does not. However Spinlock is slightly faster than mutex for the same reason.

3)Retrieving can work in parallel.Retrieving an item from the hash table does not require a lock. 

4)If two inserts are being done on different buckets in parallel,then no keys will be lost. For this we can set locks on all those buckets where the updates are being done, so no two threads will go to write and rewrite in the same bucket and hence no keys would be lost. 

//Differences between three for 64 threads

-------------------------parallel_hashtable---------------------------------
user@cs3224:~/Assignment4$ ./parallel_hashtable 64
[main] Inserted 100000 keys in 0.052719 seconds
[thread 63] 0 keys lost!
[thread 62] 0 keys lost!
[thread 60] 0 keys lost!
[thread 61] 0 keys lost!
[thread 59] 0 keys lost!
[thread 58] 0 keys lost!
[thread 57] 13 keys lost!
[thread 56] 6 keys lost!
[thread 55] 5 keys lost!
[thread 54] 1 keys lost!
[thread 52] 0 keys lost!
[thread 53] 0 keys lost!
[thread 51] 3 keys lost!
[thread 50] 0 keys lost!
[thread 49] 0 keys lost!
[thread 48] 4 keys lost!
[thread 47] 0 keys lost!
[thread 46] 0 keys lost!
[thread 45] 15 keys lost!
[thread 43] 0 keys lost!
[thread 44] 11 keys lost!
[thread 41] 7 keys lost!
[thread 42] 28 keys lost!
[thread 33] 0 keys lost!
[thread 39] 4 keys lost!
[thread 40] 12 keys lost!
[thread 31] 2 keys lost!
[thread 37] 39 keys lost!
[thread 32] 14 keys lost!
[thread 38] 19 keys lost!
[thread 35] 10 keys lost!
[thread 36] 0 keys lost!
[thread 29] 11 keys lost!
[thread 28] 12 keys lost!
[thread 30] 5 keys lost!
[thread 25] 17 keys lost!
[thread 34] 2 keys lost!
[thread 26] 14 keys lost!
[thread 27] 2 keys lost!
[thread 23] 5 keys lost!
[thread 24] 52 keys lost!
[thread 19] 8 keys lost!
[thread 18] 29 keys lost!
[thread 17] 36 keys lost!
[thread 22] 28 keys lost!
[thread 21] 24 keys lost!
[thread 20] 30 keys lost!
[thread 15] 0 keys lost!
[thread 16] 15 keys lost!
[thread 14] 0 keys lost!
[thread 1] 67 keys lost!
[thread 11] 1 keys lost!
[thread 10] 17 keys lost!
[thread 9] 9 keys lost!
[thread 8] 83 keys lost!
[thread 13] 0 keys lost!
[thread 12] 4 keys lost!
[thread 2] 2 keys lost!
[thread 4] 27 keys lost!
[thread 7] 26 keys lost!
[thread 6] 18 keys lost!
[thread 0] 66 keys lost!
[thread 3] 7 keys lost!
[thread 5] 112 keys lost!
[main] Retrieved 99078/100000 keys in 1.676505 seconds
-------------------------parallel_mutex---------------------------------
user@cs3224:~/Assignment4$ ./parallel_mutex 64
[main] Inserted 100000 keys in 0.035402 seconds
[thread 42] 0 keys lost!
[thread 58] 0 keys lost!
[thread 57] 0 keys lost!
[thread 59] 0 keys lost!
[thread 61] 0 keys lost!
[thread 56] 0 keys lost!
[thread 63] 0 keys lost!
[thread 52] 0 keys lost!
[thread 4] 0 keys lost!
[thread 51] 0 keys lost!
[thread 62] 0 keys lost!
[thread 49] 0 keys lost!
[thread 60] 0 keys lost!
[thread 3] 0 keys lost!
[thread 40] 0 keys lost!
[thread 8] 0 keys lost!
[thread 10] 0 keys lost!
[thread 45] 0 keys lost!
[thread 5] 0 keys lost!
[thread 9] 0 keys lost!
[thread 19] 0 keys lost!
[thread 23] 0 keys lost!
[thread 47] 0 keys lost!
[thread 25] 0 keys lost!
[thread 36] 0 keys lost!
[thread 15] 0 keys lost!
[thread 53] 0 keys lost!
[thread 11] 0 keys lost!
[thread 28] 0 keys lost!
[thread 54] 0 keys lost!
[thread 41] 0 keys lost!
[thread 6] 0 keys lost!
[thread 35] 0 keys lost!
[thread 14] 0 keys lost!
[thread 22] 0 keys lost!
[thread 26] 0 keys lost!
[thread 24] 0 keys lost!
[thread 2] 0 keys lost!
[thread 12] 0 keys lost!
[thread 1] 0 keys lost!
[thread 18] 0 keys lost!
[thread 39] 0 keys lost!
[thread 33] 0 keys lost!
[thread 43] 0 keys lost!
[thread 38] 0 keys lost!
[thread 37] 0 keys lost!
[thread 16] 0 keys lost!
[thread 44] 0 keys lost!
[thread 7] 0 keys lost!
[thread 34] 0 keys lost!
[thread 0] 0 keys lost!
[thread 31] 0 keys lost!
[thread 30] 0 keys lost!
[thread 48] 0 keys lost!
[thread 32] 0 keys lost!
[thread 50] 0 keys lost!
[thread 46] 0 keys lost!
[thread 55] 0 keys lost!
[thread 29] 0 keys lost!
[thread 21] 0 keys lost!
[thread 20] 0 keys lost!
[thread 17] 0 keys lost!
[thread 27] 0 keys lost!
[thread 13] 0 keys lost!
[main] Retrieved 100000/100000 keys in 1.816778 seconds
-------------------------parallel_spin---------------------------------
user@cs3224:~/Assignment4$ ./parallel_spin 64
[main] Inserted 100000 keys in 0.246210 seconds
[thread 14] 0 keys lost!
[thread 55] 0 keys lost!
[thread 36] 0 keys lost!
[thread 57] 0 keys lost!
[thread 35] 0 keys lost!
[thread 38] 0 keys lost!
[thread 40] 0 keys lost!
[thread 34] 0 keys lost!
[thread 20] 0 keys lost!
[thread 59] 0 keys lost!
[thread 37] 0 keys lost!
[thread 44] 0 keys lost!
[thread 43] 0 keys lost!
[thread 54] 0 keys lost!
[thread 56] 0 keys lost!
[thread 21] 0 keys lost!
[thread 39] 0 keys lost!
[thread 42] 0 keys lost!
[thread 26] 0 keys lost!
[thread 22] 0 keys lost!
[thread 45] 0 keys lost!
[thread 60] 0 keys lost!
[thread 50] 0 keys lost!
[thread 29] 0 keys lost!
[thread 48] 0 keys lost!
[thread 53] 0 keys lost!
[thread 10] 0 keys lost!
[thread 58] 0 keys lost!
[thread 41] 0 keys lost!
[thread 25] 0 keys lost!
[thread 33] 0 keys lost!
[thread 47] 0 keys lost!
[thread 46] 0 keys lost!
[thread 49] 0 keys lost!
[thread 52] 0 keys lost!
[thread 51] 0 keys lost!
[thread 32] 0 keys lost!
[thread 63] 0 keys lost!
[thread 27] 0 keys lost!
[thread 5] 0 keys lost!
[thread 31] 0 keys lost!
[thread 30] 0 keys lost!
[thread 24] 0 keys lost!
[thread 23] 0 keys lost!
[thread 62] 0 keys lost!
[thread 28] 0 keys lost!
[thread 61] 0 keys lost!
[thread 16] 0 keys lost!
[thread 8] 0 keys lost!
[thread 15] 0 keys lost!
[thread 19] 0 keys lost!
[thread 11] 0 keys lost!
[thread 2] 0 keys lost!
[thread 9] 0 keys lost!
[thread 12] 0 keys lost!
[thread 6] 0 keys lost!
[thread 7] 0 keys lost!
[thread 0] 0 keys lost!
[thread 17] 0 keys lost!
[thread 13] 0 keys lost!
[thread 18] 0 keys lost!
[thread 4] 0 keys lost!
[thread 3] 0 keys lost!
[thread 1] 0 keys lost!
[main] Retrieved 100000/100000 keys in 1.668986 seconds
user@cs3224:~/Assignment4$ 

*/