/* 의사 각성: 특정 조건이 만족될 때까지 대기 중이어야 하는 프로세스가
   해당 조건이 만족되지 않았음에도 불구하고 실행 상태로 변경되는 것.
   아래는 의사 각성 발생 예시. */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//시그널 핸들러. 시그널 번호를 표시할 뿐이다.
void handler(int sig) { printf("Received signal %d\n", sig); }

int example(int argc, char *argv[]) {
  pid_t pid = getpid();
  printf("PID: %d\n", pid); //여기 표시되는 프로세스 아이디에 대해 시그널을 송신한다.

  signal(SIGUSR1, handler); // SIGUSR1 시그널에 대한 시그널 핸들러 등록

  /* wait 하고 있지만 notify하는 스레드가 따로 없기 때문에 영원히 대기할 것이다.
  그러나 OpenBSD, macOS에서 해당 프로세스 아이디에 대해 다른 콘솔에서 "kill -s SIGUSR1 pid" 명령을 실행하면
  SIGUSR1 시그널이 송신되고 프로그램이 종료된다.
  리눅스에서는 wait에 futex라는 시스템 콜을 이용한다 (리눅스 커널 버전 2.6.22 이전 버전만 해당)

  따라서 아래 코드에서 의사 각성 발생 여부는 실행 환경 따라 달라짐 */
  pthread_mutex_lock(&mutex);
  if (pthread_cond_wait(&cond, &mutex) != 0) {
    perror("pthread_cond_wait");
    exit(1);
  }

  printf("spurious wakeup\n");
  pthread_mutex_unlock(&mutex);

  return 0;
}