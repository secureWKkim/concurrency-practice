// 스핀락은 직접 구현하는 것보다 라이브러리에서 제공하는 뮤텍스를 이용하는 것이 바람직하다. 아래는 이용 예시다.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//뮤텍스용 공유 변수. PTHREAD_MUTEX_INITIALIZER는 매크로다.
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void* func_for_thread(void* arg) {
    //pthread_mutex_lock 함수에 뮤텍스용 공유변수 mut의 포인터를 전달해 락을 얻는다.
    if (pthread_mutex_lock(&mut) != 0) perror("pthread_mutex_lock"); exit(-1);

    //크리티컬 섹션

    if (pthread_mutex_unlock(&mut) != 0) perror("pthread_mutex_unlock"); exit(-1);

    return NULL;
}

extern int pthread_mutex_example(int argc, char* argv[]) {
    //스레드 생성
    pthread_t th1, th2;
    if (pthread_create(&th1, NULL, func_for_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }

    if (pthread_create(&th2, NULL, func_for_thread, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }

    //thread 종료 대기
    if (pthread_join(th1, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }

    if (pthread_join(th2, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }

    // 뮤텍스 객체 반환 (릴리즈)
    if (pthread_mutex_destroy(&mut) != 0) { // 생성한 뮤텍스용 변수는 pthread_mutex_destroy 함수로 반환하지 않으면 메모리 누출을 일으킴
        perror("pthread_mutex_destroy");
        return -1;
    }

    return 0;
}