#include <stdint.h>
/* CAS는 세마포어, 락프리, 웨이트프리한 데이터 구조를 구현하기 위해 이용하는 처리다.  아토믹하지 않다.
아토믹하게 처리하려면 내장 함수인 _sync_bool_compare_and_swap(same signature as below) 를 사용한다. */
bool compare_and_swap(uint64_t* p, uint64_t oldval, uint64_t newval) {
    if (*p != oldval) return false;
    *p = newval;
    return true;
}

/* TAS도 아토믹 처리의 일종이며, 값의 비교와 대입이 아토믹하게 실행된다. 스핀락 등의 구현을 위해 이용된다.
내장 함수로 동기 처리 함수인 __sync_lock_test_and_set() 을 제공 */
bool test_and_set(bool *p) { //포인터가 가리키는 값이 어떻게 bool일 수 있는지..
    if (*p) return true;
    else {
        *p = true;
        return false;
    }
}


bool lock = false; // shared value

void good_mutex() {
    retry:
        // TAS를 이용함으로써 읽기와 쓰기를 동시에 수행할 수 있게 된다.
        if (!test_and_set(&lock)) {  // 검사 및 락 획득
            // critical section
        } else goto retry;
        // tas_release(&lock); // lock 해제. 컴파일 에러 방지를 위한 주석 처리
}