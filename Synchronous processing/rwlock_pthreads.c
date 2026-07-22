#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct {
    char name[32];
    int  score;
} Record;

Record shared_record = {"Alice", 100};  // reader/writer가 공유하는 구조체

void* reader(void *arg) {
    // 락을 쥐는 시간은 짧을수록 좋으므로, 공유 자원 접근만 크리티컬 섹션 안에 둔다. printf는 밖으로 뺀다.
    int score;
    char name[32];

    if (pthread_rwlock_rdlock(&rwlock) != 0) {
        perror("pthread_rwlock_rdlock"); exit(-1);
    }

    // 크리티컬 섹션: shared_record(공유 자원) 읽기만 수행
    score = shared_record.score;
    // shared_record(공유 자원)에서 읽어 지역 변수에 복사 — 공유 자원 자체는 읽기만 수행
    strncpy(name, shared_record.name, sizeof(name));

    if (pthread_rwlock_unlock(&rwlock) != 0) {
        perror("pthread_rwlock_unlock"); exit(-1);
    }

    // 락 밖에서 출력: printf는 공유 자원 접근이 아니므로 크리티컬 섹션 범주가 아님
    printf("reader: name=%s, score=%d\n", name, score);

    return NULL;
}

void* writer(void *arg) {
    int new_score;

    if (pthread_rwlock_wrlock(&rwlock) != 0) {
        perror("pthread_rwlock_wrlock"); exit(-1);
    }

    // 크리티컬 섹션: shared_record(공유 자원) 쓰기만 수행
    shared_record.score += 10;
    new_score = shared_record.score;

    if (pthread_rwlock_unlock(&rwlock) != 0) {
        perror("pthread_rwlock_unlock"); exit(-1);
    }

    // 락 밖에서 출력
    printf("writer: score updated to %d\n", new_score);

    return NULL;
}

int example(int argc, char **argv) {
    pthread_t rd, wr;
    pthread_create(&rd, NULL, reader, NULL);  //여기서 reader 함수 사용
    pthread_create(&wr, NULL, writer, NULL);  // 이하 라이터 함수로 동문

    pthread_join(rd, NULL);
    pthread_join(wr, NULL);

    // rw락 옵션 반환(해제)
    if (pthread_rwlock_destroy(&rwlock) != 0) {
        perror("pthread_rwlock_destroy"); return -1;
    }

    return 0;
}