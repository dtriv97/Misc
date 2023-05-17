#include <chrono>
#include <iostream>
#include <queue>
#include <thread>

#define SENSOR_TIMEOUT_US           1500000
#define SENSOR_READING_DELAY_US     100000

using namespace std;

class Sensor {
private:
    thread *pThread;
    queue<int> *pQueue;
    chrono::system_clock clk;
    chrono::system_clock::time_point startTime;

public:
    Sensor(queue<int> *msgQ);
    ~Sensor();
    void Execute();
    void WaitExecution();
};
