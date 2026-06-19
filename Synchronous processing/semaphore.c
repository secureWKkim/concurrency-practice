//세마포어는 물리적 계산 리소스 이용에 제한을 적용하고 싶은 경우 등에 이용할 수 있다.

#define NUM 4


/* 인수로 정수 타입의 공유 변수에 대한 포인터를 받는다.
 뮤텍스는 락이 이미 획득되었는지만 알면 되므로 bool 타입 공유 변수룰 이용했지만,
 세마포어에서는 다수의 프로세스가 락을 획득했는지 알아야 하므로 정수 타입을 이용한다. */
void semaphore_acquire(volatile int *cnt) {
    for (;;) {
        while (*cnt >= NUM);  //공유변수 값이 최댓값 NUM 이상이면 스핀하며 대기한다.
        __sync_fetch_and_add(cnt, 1);  //NUM 미만이면 공유 변숫값을 아토믹하게 증가한다.
        if (*cnt <= NUM)  //증가한 공유변수 값이 최댓값 NUM 이하인지 검사하여, 그렇다면 루프를 벗어나 락을 얻는다.
            break;
        __sync_fetch_and_sub(cnt, 1);  //그렇지 않으면 여러 프로세스가 동시에 락을 획득한 것이므로 공유 변숫값을 감소하여 다시 시도한다.
    }
}

void semaphore_release(int *cnt) {
    __sync_fetch_and_sub(cnt, 1);  //락을 반환한다. 공유변수값을 아토믹하게 감소시킨다.
}


int cnt = 0;

void example() {
    for (;;) {
        semaphore_acquire(&cnt); // gain lock
        // do sth
        semaphore_release(&cnt); // eject lock
    }
}