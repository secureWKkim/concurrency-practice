//세마포어는 물리적 계산 리소스 이용에 제한을 적용하고 싶은 경우 등에 이용할 수 있다.

#define NUM 4


/* 인수로 정수 타입의 공유 변수에 대한 포인터를 받는다.
 뮤텍스는 락이 이미 획득되었는지만 알면 되므로 bool 타입 공유 변수룰 이용했지만,
 세마포어에서는 다수의 프로세스가 락을 획득했는지 알아야 하므로 정수 타입을 이용한다. */
/* Q. 그럼 semaphore_posix.c 코드에서도 공유변수인 cnt가 만약 어디선가 함수 간 인자로 받게 된다면 그것도 volatile로 전달해야
  하는 거야?
    A)  꼭 그런 건 아닙니다.
  volatile이 필요한 조건은 컴파일러가 해당 변수를 레지스터에 캐시할 가능성이 있을 때입니다.
  전역변수인 cnt는 컴파일러가 보통 캐시를 잘 안 하기 때문에 붙이지 않은 겁니다.

  반면 함수 인자로 받은 포인터가 가리키는 값은 컴파일러가 "이 루프 동안 안 바뀌겠지"라고 판단해서 캐시할 수 있어서 volatile이
   필요할 수 있습니다.

  다만 현대 멀티스레드 코드에서는 volatile 대신 __sync_* 같은 아토믹 연산이나 뮤텍스로 동기화하는 게 더 올바른 방법입니다.
  volatile은 캐시만 막을 뿐 아토믹을 보장하지는 않거든요. */
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