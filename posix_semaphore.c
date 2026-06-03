#include <pthread.h>  //posix 세마포어는 이 라이브러리를 포함시키면 컴파일 및 실행 가능하다.
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 10
#define NUM_LOOP 10  //thread 안의 루프 수

int cnt = 0;  //각 스레드 안에서 증감할 글로벌 변수 정의

void* func_for_thread(void* arg) {
    //스레드에서 이름이 붙은 세마포어를 생성한다.
    sem_t *s = sem_open("/mysemaphore", 0);
    if (s == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    for (int i = 0; i < NUM_LOOP; i++) {
        // sem_wait 함수를 호출하고, 락을 획득할 때까지 대기
        if (sem_wait(s) == -1) {
            perror("sem_wait");
            exit(1);
        }

        // 카운터를 아토믹하게 증가
        __sync_fetch_and_add(&cnt, 1);
        printf("cnt = %d\n", cnt);

        usleep(10000);

        __sync_fetch_and_sub(&cnt, -1);

        // 세마포어 값을 증가시키고, 크리티컬 섹션에서 벗어난다.
        if (sem_post(s) == -1) {
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
    /* 이름이 붙은 세마포어를 연다.
     * 세마포어가 없으면 생성한다. 0_CREAT를 지정하면, 이미 해당 이름의 세마포어가 존재할 땐 생성하지 않고 열기만 한다.
     * 자신과 그룹이 이용할 수 있는 세마포어로, 크리티컬 섹션에 들어갈 수 있는 프로세스는 최대 3개다.
     * 0660은 권한을 의미하며, 유닉스 계열 OS의 파일 권한과 동일하다. 여기선 OS 프로세스의 소유자와 그룹이 읽고 쓸 수 있도록 지정한다.
     * 3은 락을 동시에 획득할 수 있는 프로세스의 상한이다. */
    sem_t *s = sem_open("/mysemaphore", 0_CREAT, 0660, 3);
    if (s == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pthread_t v[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&v[i], NULL, func_for_thread, NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(v[i], NULL);

    if (sem_close(s) == -1) perror("sem_close");

    // 이름이 있는 세마포어를 닫은 것은 핸들러를 닫은 것뿐이므로 OS 측에는 세마포어용 리소스가 남아 있다.
    // 이름 있/없는 세마포어에 대해선 따로 학습
    // 이를 완전히 삭제하려면 sem_unlink 함수를 호출해야 한다.
    if (sem_unlink("/mysemaphore") == -1) perror("sem_unlink");

    return 0;
}