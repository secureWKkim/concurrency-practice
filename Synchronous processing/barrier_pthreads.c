#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t barrier_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barrier_cond = PTHREAD_COND_INITIALIZER;

void barrier(volatile int *cnt, int max) {
    if (pthread_mutex_lock(&barrier_mut) != 0) {
        perror("pthread_mutex_lock"); exit(-1);
    }

    (*cnt)++; //락을 획득하고 공유변수 *cnt를 증가시킨다.

    if (*cnt == max) {
        /* 모든 프로세스가 모였으므로 알림. 값이 같으면 pthread_cond_broadcast 를 호출하고,
         조건변수 barrier_cond로 대기 중인 스레드를 모두 실행한다. */
        if (pthread_cond_broadcast(&barrier_cond) != 0) {
            perror("pthread_cond_broadcast"); exit(-1);
        }
    } else {
        do { //값이 같지 않으면 pthread_cond_wait를 호출하고 대기한다.ㄴ
            if (pthread_cond_wait(&barrier_cond, &barrier_mut) != 0) {
                perror("pthread_cond_wait"); exit(-1);
            }
        } while (*cnt < max);
    }
}
