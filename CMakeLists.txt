cmake_minimum_required(VERSION 3.30.5)
project(concurrency_practice C)

set(CMAKE_C_STANDARD 23)

add_executable(concurrency_practice main.c
        good_mutex.c
        mutex_pthread.c
        semaphore.c
        semaphore_posix.c
        barrier_pthreads.c
        rwlock_spinlock.c
        rwlock_pthreads.c)
