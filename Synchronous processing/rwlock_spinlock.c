//read 용 락 획득 함수. rcnt, wcnt는 각각 Reader와 Writer 수를 나타내는 공유 변수에 대한 포인터다.
void rwlock_read_acquire(int *rcnt, volatile int *wcnt) {
    for (;;) {
        /* 라이터가 있으면 대기. *wcnt 값이 0보다 크면 스핀해서 대기한다. wcnt는 락을 얻은(또는 얻으려고 시도하는)
         writer 수를 나타내므로 이 수가 0인 경우에만 리더가 락을 획득할 수 있도록 설계했다. */
        while (*wcnt);

        __sync_fetch_and_add(rcnt, 1); // 리더 수를 증가시킨다.

        /* 라이터가 없으면 락 획득. 0이면 락을 획득하지만 그렇지 않으면 *rcnt 를 아토믹하게 감소하고 재시도한다.
         다시 *wcnt 값을 확인하는 이유는 *rcnt를 증가하는 도중 *wcnt 값이 증가될 가능성이 있기 때문. */
        if (*wcnt == 0) break;
        __sync_fetch_and_sub(rcnt, 1);
    }
}

//리더용 락 반환 함수
void rwlock_read_release(int *rcnt) {
    __sync_fetch_and_sub(rcnt, 1);
}

//writer용 락(로그) 획득 함수. lock은 라이터용 로그 변수로의 포인터다.
void rwlock_write_acquire(bool *lock, volatile int *rcnt, int *wcnt) {
    __sync_fetch_and_add(wcnt, 1); // 라이터 수를 증가시키고, 리더가 없어질 때까지 대기한다.
    while (*rcnt); //reader가 있으면 대기
    spinlock_acquire(lock); //mutex용 함수를 이용해 락을 획득한다. 뮤텍스를 이용하므로 동시에 락을 획득할 수 있는 라이터 수를 최대 1개로 제한하는 것이 된다.
}

// 라이터용 락 반환 함수
void rwlock_write_release(bool *lock, int *wcnt) {
    spinlock_release(lock); //뮤텍스의 락을 해제하고 라이터 수를 감소시킨다.
    __sync_fetch_and_sub(wcnt, 1);
}


//이하는 활용 예시.
//공유 변수
int rcnt = 0;
int wcnt = 0;
bool lock = false;

void reader(void *arg) {
    for (;;) {
        rwlock_read_acquire(&rcnt, &wcnt);
        //critical section (only read)
        rwlock_read_release(&rcnt);
    }
}

void writer(void *arg) {
    for (;;) {
        rwlock_write_acquire(&lock, &rcnt, &wcnt);
        //critical section (read & write)
        rwlock_write_release(&lock, &wcnt);
    }
}