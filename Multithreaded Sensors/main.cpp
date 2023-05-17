#include "receiver.h"
#include "sensor.h"
#include <iostream>

using namespace std;

// TODO: Make a makefile

int main()
{
    /* Create queues to send out data from sensors */
    queue<int> q1;
    queue<int> q2;

    /* Create queue array to pass to receiver thread*/
    queue<int> *qs[] = {&q1, &q2};

    /* Create sensor instances */
    Sensor *s1 = new Sensor(&q1);
    Sensor *s2 = new Sensor(&q2);

    /* Reciever instance */
    Receiver *rc = new Receiver(qs);

    /* Delete order must be sensors first and then receiver */
    delete s1;
    delete s2;
    delete rc;
}
