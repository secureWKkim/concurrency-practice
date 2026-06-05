cmake_minimum_required(VERSION 3.30.5)
project(concurrency_practice C)

set(CMAKE_C_STANDARD 23)

add_executable(concurrency_practice main.c
        "Synchronous processing/good_mutex.c"
        "Synchronous processing/mutex_pthread.c"
        "Synchronous processing/semaphore.c"
        "Synchronous processing/semaphore_posix.c"
        "Synchronous processing/barrier_pthreads.c"
        "Synchronous processing/rwlock_spinlock.c"
        "Synchronous processing/rwlock_pthreads.c")
