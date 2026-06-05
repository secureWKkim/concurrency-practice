#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void* reader(void *arg) {
    if (pthread_rwlock_rdlock(&rwlock) != 0) {
        perror("pthread_rwlock_rdlock"); exit(-1);
    }

    //critical section(read only)

    if (pthread_rwlock_unlock(&rwlock) != 0) {
        perror("pthread_rwlock_unlock"); exit(-1);
    }

    return NULL;
}

void* writer(void *arg) {
    if (pthread_rwlock_wrlock(&rwlock) != 0) {
        perror("pthread_rwlock_wrlock"); exit(-1);
    }

    //critical section(read only)

    if (pthread_rwlock_unlock(&rwlock) != 0) {
        perror("pthread_rwlock_unlock"); exit(-1);
    }

    return NULL;
}

int example(int argc, char **argv) {
    pthread_t rd, wr;
    pthread_create(&rd, NULL, reader, NULL);
    pthread_create(&wr, NULL, writer, NULL);

    pthread_join(rd, NULL);
    pthread_join(wr, NULL);

    // rw락 옵션 반환(해제)
    if (pthread_rwlock_destroy(&rwlock) != 0) {
        perror("pthread_rwlock_destroy"); return -1;
    }

    return 0;
}