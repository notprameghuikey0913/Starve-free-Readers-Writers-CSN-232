#include<iostream>
#include<thread>
#include<mutex>
#include<math.h>

/*
    A mutex (named for "mutual exclusion") is a binary semaphore with an ownership restriction:
    it can be unlocked (the post operation) only by whoever locked it (the wait operation). 
    Thus a mutex offers a somewhat stronger protection than an ordinary semaphore.
*/
std::mutex reader_mutex, writer_mutex, queue_mutex;
//Resource
int resource = 1;
//Number of Readers
unsigned int readers = 0;

void reader(int reader_index)
{
    queue_mutex.lock();
    reader_mutex.lock();
    if (readers == 0)
        writer_mutex.lock();
    readers++;

    queue_mutex.unlock();
    reader_mutex.unlock();

    printf("Reader %d reads resource as %d\n", reader_index,resource);

    reader_mutex.lock();
    readers--;
    if (readers == 0)
        writer_mutex.unlock();
    reader_mutex.unlock();
}

void writer(int writer_index)
{
    queue_mutex.lock();
    writer_mutex.lock();
    queue_mutex.unlock();

    resource = pow(2, writer_index);
    printf("Writer %d modifies resource as %d\n", writer_index,resource);

    writer_mutex.unlock();
}

int main()
{
    std::thread readers[10], writers[10];

    for (int i=0; i<10; ++i)
    {
        readers[i] = std::thread(reader,i+1);
        writers[i] = std::thread(writer,i+1);
    }
    
    for (int i=0; i<10; ++i)
    {
        readers[i].join();
        writers[i].join();
    }

    return 0;
}
