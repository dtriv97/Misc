#include <chrono>
#include <iostream>
#include <queue>
#include <thread>

/* Current fixed to 2 sensors -> TODO: Add capability for more sensors */
#define MAX_SENSORS 2
#define RECEIVER_TIMEOUT_US  200000

using namespace std;

class Receiver
{
    /* Sensor param struct */
    struct SensorDev
    {
        queue<int> *msgQ;
        bool valueRead;
    };

private:
    SensorDev sensors[MAX_SENSORS];
    chrono::system_clock clk;
    chrono::system_clock::time_point startTime;
    thread *pThread;

public:
    Receiver(queue<int> *sensors[]);
    ~Receiver();

    void Execute();
};
