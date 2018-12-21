#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct
{
    pthread_mutex_t ourMutex;
    int SeatsInTrain;
    int PassengerInStation;
    int passengerOnBoard;
    int passengerEnter;
    ///conditionVariable for passenger
    pthread_cond_t conditionVariableForPassenger;
    ///conditionVariable for train to wait all passengers to be on board
    pthread_cond_t conditionVariableForTrain;

} station;
///multivalued struct to pass it to arg of function in pthread create calling
typedef struct
{
    station *s;
    int count;
} multiValue;
void station_init(station *ourStation)
{
///mutex initialization
    pthread_mutex_init(&(ourStation->ourMutex),NULL);
    ourStation->SeatsInTrain=0;
    ourStation->PassengerInStation=0;
    ourStation->passengerOnBoard=0;
    ourStation->passengerEnter=0;
///one condition variable for passenger to wait for the train
    pthread_cond_init(&(ourStation->conditionVariableForPassenger),NULL);
///one condition variable for train to wait all passengers to be on board
    pthread_cond_init(&(ourStation->conditionVariableForTrain),NULL);

}
void station_on_board(station *ourStation)
{
///we make mutex loack bec. passengerOnBoard is a shared resource between passengers so we have to protect it so we increment passengers who seated
    pthread_mutex_lock(&(ourStation->ourMutex));
    printf("passenger a3d\n");
    ourStation->passengerOnBoard++;
    ///make a check if all passengers on board the train while leave !
    if(ourStation->passengerEnter==ourStation->passengerOnBoard)
    {
        printf("train masha\n");
        pthread_cond_signal(&(ourStation->conditionVariableForTrain));
    }
    pthread_mutex_unlock(&(ourStation->ourMutex));

}
void station_load_train(station * ourStation,int count)
{
    printf("train gah\n");
    ourStation->SeatsInTrain=count;
    ///critical section that passengers enter the train if theere are passengers waiting and there are availae seats
    pthread_mutex_lock(&(ourStation->ourMutex));
    while(ourStation->PassengerInStation && ourStation->SeatsInTrain)
    {
        printf("passengerEnter %d\n",ourStation->passengerEnter);
        pthread_cond_signal(&(ourStation->conditionVariableForPassenger));
        ourStation->PassengerInStation--;
        ourStation->SeatsInTrain--;
        ourStation->passengerEnter++;
    }
    ///check if not all passengers on board let train wait for them
    if(ourStation->passengerEnter!=ourStation->passengerOnBoard)
    {
        printf("Train mastny\n");
        /// pthread_mutex_lock(&(ourStation->ourMutex));
        pthread_cond_wait(&(ourStation->conditionVariableForTrain),&(ourStation->ourMutex));
    }
    pthread_mutex_unlock(&(ourStation->ourMutex));


}
///just an function to be called in thread create
void * FunctionToCallLoad(void *arg)
{
    multiValue *value= (multiValue *)(arg);
    station_load_train(value->s,value->count);

}

void station_wait_for_train(station *ourStation)
{
    printf("passenger d5l\n");
    ///critical section because PassengerInStations is a shared resource so we need to protect this value it indicate that a passenger enter the station
    pthread_mutex_lock(&(ourStation->ourMutex));
    ourStation->PassengerInStation++;
    pthread_cond_wait(&(ourStation->conditionVariableForPassenger),&(ourStation->ourMutex));
    pthread_mutex_unlock(&(ourStation->ourMutex));
    ///after entered the train will call on board
    station_on_board(ourStation);

}
void * FunctionToCallWait(void * arg)
{

    station *s= (station *)(arg);
    station_wait_for_train(s);
}

int main()
{

    station *ourStation;
    ourStation=(station *)malloc(sizeof(station));
    pthread_t passenger[18];
    station_init(ourStation);
    int i;
    for(i=0; i<18; i++)
    {
        pthread_create(&passenger[i],NULL,FunctionToCallWait,(void *)ourStation);
    }
    sleep(2);
    pthread_t train[4];
    int countingNumber=5;
    multiValue * combo= (multiValue *)malloc(sizeof (multiValue));
    combo->s=ourStation;
    for(i=0; i<4; i++)
    {
        combo->count=countingNumber;
        pthread_create(&train[i],NULL,FunctionToCallLoad,(void *)combo);
        sleep(2);
    }
    return 0;
}
/*
WHO MUTEX WORKS !
The mutex object referenced by mutex shall be locked by calling pthread_mutex_lock().
If the mutex is already locked, the calling thread shall block until the mutex becomes available.
This operation shall return with the mutex object referenced by mutex in the locked state with the calling thread as its owner.
*/
/*
COND_WAIT WITH MUTEX
cond is a condition variable that is shared by threads.
To change it, a thread must hold the mutex associated with the condition variable.
The pthread_cond_wait() function releases this mutex before suspending the thread and obtains it again before returning.
*/
/*
COND_SIGNAL WITH MUTEX
 If more than one thread is blocked on a condition variable, the scheduling policy determines the order in which threads are unblocked.
 When each thread unblocked as a result of a pthread_cond_signal()returns from its call to pthread_cond_wait(),
the thread owns the mutex with which it called pthread_cond_wait() or pthread_cond_timedwait().
The thread(s) that are unblocked contend for the mutex according to the scheduling policy (if applicable),
and as if each had called pthread_mutex_lock().
*/
