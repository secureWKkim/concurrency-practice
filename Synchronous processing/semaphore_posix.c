#include <pthread.h>  //posix 세마포어는 이 라이브러리를 포함시키면 컴파일 및 실행 가능하다.
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 10
#define NUM_LOOP 10  //thread 안의 루프 수

int cnt = 0;  //각 스레드 안에서 증감할 글로벌 변수 정의

void* func_for_thread(void* arg) {
    /*스레드에서 이름이 붙은 세마포어를 생성한다. 두 번째 인자 0은 oflag 입니다.
  oflag에 O_CREAT를 넘기면 세마포어를 새로 만들고, 0을 넘기면 이미 존재하는 세마포어를 열기만 합니다.
  func_for_thread에서는 메인에서 이미 만들어놨으니 그냥 열기만 하면 되는 거예요.
     */
    sem_t *s = sem_open("/mysemaphore", 0);
    if (s == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    for (int i = 0; i < NUM_LOOP; i++) {
        // sem_wait 함수를 호출하고, 락을 획득할 때까지 대기
        if (sem_wait(s) == -1) {  //세마포어 값을 1 감소시킨다.
            perror("sem_wait");
            exit(1);
        }

        // 카운터를 아토믹하게 증가
        __sync_fetch_and_add(&cnt, 1);
        printf("cnt = %d\n", cnt);

        usleep(10000);

        __sync_fetch_and_sub(&cnt, -1);

        // 세마포어 값을 증가시키고, 크리티컬 섹션에서 벗어난다.
        if (sem_post(s) == -1) { // 이게 세마포어 값을 증가시킴
            perror("sem_post");
            exit(1);
        }
    }

    // 필요 없어진 세마포어는 sem_close를 호출하여 닫아야 한다.
    if (sem_close(s) == -1)
        perror("sem_close");

    return NULL;
}


int posix_semaphore_example(int argc, char* argv[]) {
    /* 이름이 붙은 세마포어를 연다. 자신과 그룹이 이용할 수 있는 세마포어다.
     * oflag 인자. O_CREAT → 세마포어가 없으면 새로 생성, 0 → 기존 것만 열기
     * 0660은 권한을 의미하며, 유닉스 계열 OS의 파일 권한과 동일하다. 여기선 OS 프로세스의 소유자와 그룹이 읽고 쓸 수 있도록 지정한다.
     * 3은 락을 동시에 획득할 수 있는 프로세스의 상한이다. 정확히는 세마포어 초기값 = 크리티컬 섹션에 동시 진입 가능한 프로세스 수 */
    sem_t *s = sem_open("/mysemaphore", O_CREAT, 0660, 3);
    if (s == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pthread_t v[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&v[i], NULL, func_for_thread, NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(v[i], NULL);//스레드가 끝날 때까지 기다리는 함수. 이게 없으면 메인이 먼저 종료되면서 스레드가 중간에 강제로 죽을 수 있다.

    if (sem_close(s) == -1) perror("sem_close");

    // 이름이 있는 세마포어를 닫은 것은 핸들러를 닫은 것뿐이므로 OS 측에는 세마포어용 리소스가 남아 있다.
    // 이름 있/없는 세마포어에 대해선 따로 학습
    // 이를 완전히 삭제하려면 sem_unlink 함수를 호출해야 한다.
    if (sem_unlink("/mysemaphore") == -1) perror("sem_unlink");

    return 0;
}

/*
● ┌───────────┬─────────────────────────────────────────────┬────────────────────────────────────┐
  │           │             이름 있는 세마포어              │         이름 없는 세마포어         │
  ├───────────┼─────────────────────────────────────────────┼────────────────────────────────────┤
  │ 생성      │ sem_open("/이름", ...)                      │ sem_init(&s, ...)                  │
  ├───────────┼─────────────────────────────────────────────┼────────────────────────────────────┤
  │ 공유 범위 │ 다른 프로세스끼리 이름으로 찾아서 공유 가능 │ 같은 프로세스 내 스레드끼리만 공유 │
  ├───────────┼─────────────────────────────────────────────┼────────────────────────────────────┤
  │ 실체      │ OS에 파일처럼 등록됨                        │ 그냥 메모리 변수                   │
  ├───────────┼─────────────────────────────────────────────┼────────────────────────────────────┤
  │ 정리      │ sem_close + sem_unlink 필요                 │ sem_destroy만 하면 됨              │
  └───────────┴─────────────────────────────────────────────┴────────────────────────────────────┘

*핸들러: 특정 요청이나 이벤트를 처리하는 함수(또는 객체). 예를 들어:
HTTP 요청 → Request Handler
버튼 클릭 → Click Handler
마우스 이동 → Mouse Event Handler
예외 발생 → Exception Handler
시그널 수신 → Signal Handler
모두 어떤 사건(Event)이 발생했을 때 실행되는 처리기다.
 */