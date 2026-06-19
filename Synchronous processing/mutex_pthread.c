// 스핀락은 직접 구현하는 것보다 라이브러리에서 제공하는 뮤텍스를 이용하는 것이 바람직하다. 아래는 이용 예시다.

#include <stdio.h>
#include <stdlib.h>  //exit(), malloc(), free() 같은 기본 유틸리티 함수들을 모아놓은 헤더
#include <pthread.h>

//뮤텍스용 공유 변수. PTHREAD_MUTEX_INITIALIZER는 매크로다.
//pthread_mutex_t는 뮤텍스의 상태를 추적하기 위한 내부 데이터를 담는 구조체
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void* func_for_thread(void* arg) {
    //pthread_mutex_lock 함수에 뮤텍스용 공유변수 mut의 포인터를 전달해 락을 얻는다.
    if (pthread_mutex_lock(&mut) != 0) perror("pthread_mutex_lock"); exit(-1);
    //크리티컬 섹션
    if (pthread_mutex_unlock(&mut) != 0) perror("pthread_mutex_unlock"); exit(-1);
    return NULL;
}


/* 아래 스레드들은 병렬로 실행된다.
메인:     pthread_create(th1) → th1 즉시 시작
  메인:     pthread_create(th2) → th2 즉시 시작  ← th1이랑 동시에 실행 중
  메인:     pthread_join(th1)   → th1 끝날 때까지 대기
  메인:     pthread_join(th2)   → th2 끝날 때까지 대기 (이미 끝났을 수도 있음)

  create 시점에 이미 두 스레드가 동시에 돌고 있고, join은 그냥 "끝날 때까지 기다렸다가 다음 줄로 가"는 역할만 합니다.
 */
extern int pthread_mutex_example(int argc, char* argv[]) {
    /*pthread_create 인자 해설
  ┌─────────────────────┬────────────────────┬───────────────────────────────────────────┐
  │        인자         │         값         │                   의미                    │
  ├─────────────────────┼────────────────────┼───────────────────────────────────────────┤
  │ 1번 &th1            │ 스레드 핸들 포인터 │ 생성된 스레드의 ID를 여기에 저장해줘      │
  ├─────────────────────┼────────────────────┼───────────────────────────────────────────┤
  │ 2번 NULL            │ 스레드 속성        │ 스택 크기 같은 세부 설정. NULL이면 기본값 │
  ├─────────────────────┼────────────────────┼───────────────────────────────────────────┤
  │ 3번 func_for_thread │ 함수 포인터        │ 이 스레드가 실행할 함수                   │
  ├─────────────────────┼────────────────────┼───────────────────────────────────────────┤
  │ 4번 NULL            │ 함수에 넘길 인자   │ func_for_thread(arg)의 arg. 없으면 NULL   │
  └─────────────────────┴────────────────────┴───────────────────────────────────────────┘
  즉 "스레드를 하나 만들어서, func_for_thread를 실행시키고, 그 스레드의 ID를 th1에 저장해줘" 라는 뜻 */

    pthread_t th1, th2;
    if (pthread_create(&th1, NULL, func_for_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }

    if (pthread_create(&th2, NULL, func_for_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }

    /*  pthread_join은 thread(바로 아래는 th1) 종료 때까지 대기 작업을 수행
     *  pthread_join의 2번째 인자는 스레드 함수의 반환값을 받는 포인터입니다.
  void* retval;
  pthread_join(th1, &retval);  // func_for_thread의 return 값이 retval에 저장됨

  func_for_thread가 return NULL을 하니까 필요없어서 NULL을 넘긴 겁니다.
*/
    if (pthread_join(th1, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }

    if (pthread_join(th2, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }

    // 뮤텍스 객체 반환 (릴리즈)
    // 생성한 뮤텍스용 변수는 pthread_mutex_destroy 함수로 반환하지 않으면 메모리 누수를 일으킴
    if (pthread_mutex_destroy(&mut) != 0) {
        perror("pthread_mutex_destroy");
        return -1;
    }

    return 0;
}