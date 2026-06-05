/* 일반적으로 시그널과 멀티스레드는 궁합이 맞지 않다고 알려져 있다.
 어떤 타이밍에서 시그널 핸들러가 호출되는지 알 수 없기 때문이다. 시그널 핸들러에 데드락이 발생하기도 쉽다.
  이런 상태에 빠지는 걸 방지하기 위해 시그널 수신 전용 스레드를 이용할 수 있다. 아래는 그 예시다. */

#include "concurrency_bugs.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sigset_t set;

void *handler(void *arg) { // 시그널 처리 전용 스레드 함수
  pthread_detach(pthread_self());

  int sig;
  for (;;) {
    // sigwait로 시그널 수신. 인수는 각각 수신 시그널의 종류, 수신한 시그널의 종류
    if (sigwait(&set, &sig) != 0) { perror("sigwait"); exit(1); }
    printf("received signal %d\n", sig);
    pthread_mutex_lock(&mutex);
    //무언가 처리
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

/* 워커 스레드용 함수. 이 스레드 실행 후에는 시그널 핸들러가 실행되지 않으므로 데드락이 발생하지 않음 */
void *worker(void *arg) {
  for (int i = 0; i < 10; i++) {
    pthread_mutex_lock(&mutex);
    //무언가 처리
    sleep(1);
    pthread_mutex_unlock(&mutex);
    sleep(1);
  }
  return NULL;
}

int example2(int argc, char *argv[]) {
  return 0;
}