/*
* This is a "Sensor" object threaded class. This spoofs a sensor that produces a sensor reading
* which in this case is just the elapsed time between sensor data every 0.1 seconds. This is pushed to a queue 
* from where it can be consumed. The thread times out after 1.5 seconds of running.
*/
#include "sensor.h"

using namespace chrono;

// Constructor
Sensor::Sensor(queue<int> *msgQ) {
    this->pQueue = msgQ;

    // Make sure to clear the queue that will be used at intialisation
    queue<int> empty;
    swap(*(this->pQueue), empty);

    // Save start time of thread before starting
    this->startTime = this->clk.now();

    // Start a thread running the Execute function of "this" object
    this->pThread = new thread([this] { Execute(); });
}

// Destructor
Sensor::~Sensor() {
    // Await end of thread execution and delete
    this->pThread->join();
    delete this->pThread;
}

void Sensor::Execute() {
    int elapsedUs;
    while (1) {
      // Check total sensor reading timeout has happened or not
        elapsedUs =
            duration_cast<microseconds>(this->clk.now() - this->startTime).count();
        if (elapsedUs >= SENSOR_TIMEOUT_US) {
            cout << "Sensor thread done\n";
            break;
        }

        // Append measurement to output queue
        this->pQueue->push(elapsedUs);

        // Add delay in loop for sensor readings
        this_thread::sleep_for(microseconds(SENSOR_READING_DELAY_US));
    }
}
