#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
/* 배리어(Barrier)가 뭔지부터
배리어는 "모든 스레드가 여기까지 도달할 때까지 아무도 다음으로 못 넘어가"라는 동기화 방법입니다.
예를 들어 10개 스레드가 각자 계산을 하고, 전부 끝난 뒤에 결과를 합산해야 한다면 배리어가 필요합니다.

코드 흐름
  cnt는 "지금까지 배리어에 도착한 스레드 수", max는 "총 스레드 수"입니다.

  스레드가 barrier() 호출
      → 락 걸기
      → cnt++ (내가 도착했다고 카운트)
      → cnt == max? (모두 도착했나?)
          YES → cond_broadcast로 대기 중인 전원 깨우기
          NO  → cond_wait로 락 풀고 잠들어서 대기

  cond_wait는 두 가지를 동시에 합니다:
  1. 락을 풀어서 다른 스레드가 cnt++ 할 수 있게 함
  2. 이 스레드는 잠들어서 broadcast 신호를 기다림

  마지막 스레드가 도착해서 cnt == max가 되면 broadcast로 잠든 스레드를 전부 깨우고, 모두 다음 코드로 진행합니다.
*/

/* pthread_mutex_t — 내부적으로 TAS/CAS 기반의 점유 플래그를 가집니다. "잠겼냐 안 잠겼냐" 두 상태만 추적합니다.

  pthread_cond_t — 내부적으로 대기 중인 스레드 큐를 가집니다. cond_wait로 들어온 스레드들을 줄 세워두고, broadcast/signal이
  오면 꺼내서 깨웁니다.

  INITIALIZER 매크로 둘 다 역할은 같습니다 — 선언과 동시에 내부 구조체를 초기값으로 세팅하는 것. 차이는 세팅하는 구조체가 다른
  것뿐입니다.

  - PTHREAD_MUTEX_INITIALIZER → 플래그를 "잠금 해제" 상태로
  - PTHREAD_COND_INITIALIZER → 대기 큐를 "비어있음" 상태로

  한 줄 요약: 뮤텍스는 플래그, 조건변수는 큐. 둘을 같이 쓰는 이유는 뮤텍스로 공유변수를 보호하면서, 조건변수로 "언제 깨울지"를
  제어하기 위해서입니다.
 */
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
        do { //값이 같지 않으면 pthread_cond_wait를 호출하고 대기한다.
            if (pthread_cond_wait(&barrier_cond, &barrier_mut) != 0) {
                perror("pthread_cond_wait"); exit(-1);
            }
        } while (*cnt < max);
    }
}
