#include <stdint.h>
#include <stdbool.h>


/* CAS는 세마포어, 락프리, 웨이트프리한 데이터 구조를 구현하기 위해 이용하는 처리다.

뮤텍스(락)의 문제는 한 스레드가 락을 쥐고 죽거나 멈추면 나머지는 전부 기다려야 한다는 거다.

CAS를 쓰면 락 없이도 동기화가 가능하다. 실패하면 그냥 다시 시도하면 되기 때문:
// 락 없이 카운터 증가하는 예시
while (!CAS(&counter, old, old + 1)) {
    old = counter;  // 누가 바꿨으면 다시 읽고 재시도
}

락프리: 락이 없으니 한 스레드가 멈춰도 다른 스레드들은 계속 진행 가능
웨이트프리: 재시도 횟수에도 상한이 있어서 무한정 기다리는 일이 없음 (더 강한 보장)

아래 구현 함수는 아토믹하지 않다.
아토믹하게 처리하려면 내장 함수인 _sync_bool_compare_and_swap(same signature as below) 를 사용한다.
CAS가 아토믹하지 않다 = 값 확인과 변경이 각각의 CPU 명령으로 실행된다. 쪼갤 수 있는 동작으로 실행되기에 경쟁 상태 문제가 있는 것.
      아토믹하다 = 두 명령이 "하나의" CPU 명령으로 실행된다.
      
요약하면, CAS는 뮤텍스 없이 안전하게 공유 변수를 수정하는 핵심 도구이고, 아래 코드는 그 개념을 보여주는 예시인데 실제로는 쓰면 안 된다. */
bool compare_and_swap(uint64_t* p, uint64_t oldval, uint64_t newval) {
    if (*p != oldval) return false;
    *p = newval;
    return true;
}

/* TAS는 확인(Test)하고 → 세팅(Set)한다는 동작으로, 값의 비교와 대입이 아토믹하게 실행된다.
AS는 CAS보다 단순한 대신 "점유됨 / 비어있음" 두 상태만 표현할 수 있어서 주로 스핀락 구현을 위해 이용된다.
스핀락은 락을 얻을 때까지 빙글빙글 돌면서 계속 시도하는 락
내장 함수로 동기 처리 함수인 __sync_lock_test_and_set() 을 제공 */
bool test_and_set(bool *p) { //항상 *p를 true로 만들고, 원래 값을 돌려준다
    if (*p) return true; //원래 true였음 true 반환
    else {
        *p = true; //원래 false였음 true로 바꾸고
        return false; //false 반환.
    }
    //TAS가 *lock을 true로 바꾸는 것 자체가 "내가 지금 쓸게" 라고 깃발 꽂는 동작
}


bool lock = false; // 공유변수. false = 락 안 걸림, true = 락


/*스핀락 구현 예시
// 락 걸기
while (test_and_set(&lock) == true) {
    // 누가 이미 쓰고 있음 → 계속 기다림 (spin)
}
// 여기 오면 내가 락을 획득한 것

// ... 공유 자원 사용 ...

// 락 풀기
lock = false;
*/

/*TAS가 확인 + 세팅을 CPU 레벨에서 한 방에 처리하기 때문에
중간에 다른 스레드가 끼어들 수 없어서 race condition이 없다.
그래서 굿 뮤텍스.
*/
void good_mutex() {
    retry:
        // 스핀락으로 락 걸기. TAS를 이용함으로써 읽기와 쓰기를 동시에 수행할 수 있게 된다.
        if (!test_and_set(&lock)) {  // 검사 및 락 획득
            // critical section
        } else goto retry;
        // tas_release(&lock); // lock 해제. 컴파일 에러 방지를 위한 주석 처리
}