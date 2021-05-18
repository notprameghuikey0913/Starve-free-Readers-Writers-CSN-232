#include<stdio.h>
#include<pthread.h> // c parallel threaders header
#include<semaphore.h> // semaphore header
#include<math.h>
/* 
    sem_t is a typedef defined in a header file as (apparently) a kind of unsigned char.
    writer mutex: semaphore for locking resource from writers;
    reader mutex: semaphore for locking resource for indi writer;
    queue mutex: semaphore for queue maintaining
*/
sem_t queue_mutex;

/*
    A mutex (named for "mutual exclusion") is a binary semaphore with an ownership restriction:
    it can be unlocked (the post operation) only by whoever locked it (the wait operation). 
    Thus a mutex offers a somewhat stronger protection than an ordinary semaphore.
    We declare a mutex as:
    pthread_mutex_t mutex;
*/
pthread_mutex_t reader_mutex , writer_mutex;

//RESOURCE
int resource = 1;
// READER COUNT
unsigned int readers = 0; // number of readers accessing resource

void *reader(void *reader_index)
{
    /*
       sem_wait() decrements (locks) the semaphore pointed to by sem.
       If the semaphore's value is greater than zero, then the decrement
       proceeds, and the function returns, immediately.  If the
       semaphore currently has the value zero, then the call blocks
       until either it becomes possible to perform the decrement (i.e.,
       the semaphore value rises above zero), or a signal handler
       interrupts the call.
    */
    sem_wait(&queue_mutex);

    /*
       The mutex object referenced by mutex shall be locked by a call to
       pthread_mutex_lock() that returns zero or [EOWNERDEAD].  If the
       mutex is already locked by another thread, the calling thread
       shall block until the mutex becomes available.
    */
    pthread_mutex_lock(&reader_mutex);
    if (readers == 0)
        pthread_mutex_lock(&writer_mutex);
    // increment the number of users.
    readers++;

    /*
       sem_post() increments (unlocks) the semaphore pointed to by sem.
       If the semaphore's value consequently becomes greater than zero,
       then another process or thread blocked in a sem_wait(3) call will
       be woken up and proceed to lock the semaphore.
    */
    sem_post(&queue_mutex);
    pthread_mutex_unlock(&reader_mutex);

    printf("Reader %d reads resource as %d\n",*((int *)reader_index),resource);

    pthread_mutex_lock(&reader_mutex);
    readers--;
    if( readers == 0 )
        pthread_mutex_unlock(&writer_mutex);
    pthread_mutex_unlock(&reader_mutex);

    return reader_index;
}

void *writer(void* writer_index)
{
    sem_wait(&queue_mutex);
    pthread_mutex_lock(&writer_mutex);
    sem_post(&queue_mutex);

    resource  = pow(2,*((int *)writer_index));
    printf("Writer %d modifies resource as %d\n",*((int *)writer_index),resource);

    pthread_mutex_unlock(&writer_mutex);

    return writer_index;
}

int main()
{
    // declaring pthreads or posix threads
    // we are having 10 readers and 10 writers in our system.
    pthread_t readers[10], writers[10];
   
    /* 
        Prototype:  int pthread_mutex_init(pthread_mutex_t * restrict mutex, const pthread_mutexattr_t * restrict attr);
        Library: #include <pthread.h>
        Purpose: This initializes *mutex with the attributes 
            specified by attr. If attr is NULL, a default set of 
            attributes is used. The initial state of *mutex will be
            "initialized and unlocked".
    */
    pthread_mutex_init(&reader_mutex, NULL);
    pthread_mutex_init(&writer_mutex , NULL);

    /*
        Prototype: int sem_init(sem_t * sem, int pshared, unsigned int value);
        Library: #include <semaphore.h>
        Purpose: This initializes the semaphore *sem. The initial value of the semaphore will be value. 
                 If pshared is 0, the semaphore is shared among all threads of a process (and hence need
                 to be visible to all of them such as a global variable). If pshared is not zero, the 
                 semaphore is shared but should be in shared memory.
    */
    sem_init(&queue_mutex,0,1);

    //num_labels for writers amd readers
    int num_labels[10] = {1,2,3,4,5,6,7,8,9,10};

    /*
        pthread_create: used to create a new thread
        Syntax:        int pthread_create(pthread_t * thread, 
                        const pthread_attr_t * attr, 
                        void * (*start_routine)(void *), 
                        void *arg);
        Parameters:
            thread: pointer to an unsigned integer value that returns the thread id of the thread created.
            attr: pointer to a structure that is used to define thread attributes like detached state, scheduling policy, stack address, etc. Set to NULL for default thread attributes.
            start_routine: pointer to a subroutine that is executed by the thread. The return type and parameter type of the subroutine must be of type void *. The function has a single attribute but if multiple values need to be passed to the function, a struct must be used.
            arg: pointer to void that contains the arguments to the function defined in the earlier argument
    */
    for(int i = 0; i < 10; i++) {
        pthread_create(&readers[i], NULL, reader, (void *)&num_labels[i]);
        pthread_create(&writers[i], NULL, writer, (void *)&num_labels[i]);
    }

    /*
       The pthread_join() function waits for the thread specified by
       thread to terminate.  If that thread has already terminated, then
       pthread_join() returns immediately.  The thread specified by
       thread must be joinable.
    */
    for(int i = 0; i < 10; i++) {
        pthread_join(readers[i], NULL);
        pthread_join(writers[i], NULL);
    }

    /*
       The pthread_mutex_destroy() function shall destroy the mutex
       object referenced by mutex; the mutex object becomes, in effect,
       uninitialized. An implementation may cause
       pthread_mutex_destroy() to set the object referenced by mutex to
       an invalid value.
    */
    pthread_mutex_destroy(&writer_mutex);
    pthread_mutex_destroy(&reader_mutex);
    
    /*
       sem_destroy() destroys the unnamed semaphore at the address
       pointed to by sem.
       Only a semaphore that has been initialized by sem_init(3) should
       be destroyed using sem_destroy().
    */
    sem_destroy(&queue_mutex);

    return 0;
}