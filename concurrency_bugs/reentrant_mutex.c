/* 재귀락: 락을 획득한 상태에서 프로세스가 그 락을 해제하기 전에 다시 그 락을 획득하는 것
* 단순 뮤텍스 구현에 대해 재귀락을 수행하면 데드락 상태가 된다.
* 재귀락을 수행해도 처리를 계속할 수 있는 락을 재진입 가능한 락이라고 한다. */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct reentrant_lock
{
    bool lock;   //spinlock용 공용 변수
    int id;      //현재 락을 획득 중인 스레드 아이디, 0이 아니면 락 획득 중임
    int cnt;    // 재귀락 수행 횟수
};

//재귀락 획득 함수
void reentlock_acquire(struct reentrant_lock *lock, int id) {
  //락 획득한 상태고, 동시에 자신이 획득한 상탠지 판정.
  if (lock->lock && lock->id == id)
    lock->cnt++;
  else {
    //어떤 스레드도 락을 획득하지 않음 or 다른 스레드가 락을 획득 중이면 락 획득
    spinlock_acquire(&lock->lock);
    lock->id = id;
    lock->cnt++;
  }
}

//재귀락 해제 함수
void reentlock_release(struct reentrant_lock *lock) {
  // 카운트를 감소하고, 해당 카운트가 0이 되면 락 해제
  lock->cnt--;
  if (lock->cnt == 0) {
    lock->id = 0;
    spinlock_release(&lock->lock);
  }
}


//이하는 사용 예시
struct reentrant_lock lock_var;  //락용 공용 변수

// n회 재귀적으로 호출해 락을 거는 테스트 함수
void reent_lock_test(int id, int n) {
  if (n==0) return;

  reentlock_acquire(&lock_var, id);
  reent_lock_test(id, n-1);
  reentlock_release(&lock_var);
}

//스레드용 함수
void *thread_func(void *arg) {
  int id = *(int *)arg;
  assert(id!=0);
  for (int i=0; i<10000; i++)
    reent_lock_test(id,10);
  return NULL;
}



int example(int argc, char* argv[]) {
  #define NUM_THREADS 10

  pthread_t v[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&v[i], NULL, thread_func, (void *)(i+1));
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(v[i], NULL);
  return 0;
}