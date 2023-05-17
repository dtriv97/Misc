/*
* This is a "Receiver" threaded object. This receiver object accepts data input from 2 different sensor
* sources and displays the time they were received and the delta in when they were received. The receiver
* thread times out after 200ms if no data is received on either sensor queue.
*/
#include "receiver.h"

using namespace chrono;

Receiver::Receiver(queue<int> *sensors[]) {
	for (int i = 0; i < MAX_SENSORS; i++) {
		this->sensors[i].msgQ = sensors[i];
		this->sensors[i].valueRead = false;
	}
	this->pThread = new thread([this] { Execute(); });
}

Receiver::~Receiver()
{
	/* Wait for the thread execution to be done */
	this->pThread->join();
	delete this->pThread;
}

void Receiver::Execute()
{
	int sensorsRxed = 0;
	int rxedMsg[] = {0, 0};
	int elapsedUs;

	this->startTime = this->clk.now();

	// Take first reading of time event started same as total elapsed start
	system_clock::time_point measureStart = this->startTime;

	while (1)
	{
		// Check total sensor reading timeout has happened or not
		elapsedUs =
				duration_cast<microseconds>(this->clk.now() - measureStart).count();
		if (elapsedUs >= RECEIVER_TIMEOUT_US)
		{
			cout << "Receiver thread timed out, shutting down.\n";
			break;
		}

		// Check all sensors for any readings
		for (int i = 0; i < MAX_SENSORS; i++)
		{
			if ((!this->sensors[i].valueRead) && (!this->sensors[i].msgQ->empty()))
			{
				rxedMsg[sensorsRxed] = this->sensors[i].msgQ->front();
				this->sensors[i].msgQ->pop();
				this->sensors[i].valueRead = true;
				sensorsRxed++;
			}
		}

		// Check if all sensors have been read from
		if (sensorsRxed == MAX_SENSORS)
		{
			float elapsedTotal =
					duration_cast<microseconds>(this->clk.now() - this->startTime)
							.count() /
					1000.0f;
			cout << "Got measurements [t = " << elapsedTotal << "ms] with difference "
					 << rxedMsg[0] - rxedMsg[1] << endl;

			// Reset vars to read again
			sensorsRxed = 0;
			for (int i = 0; i < MAX_SENSORS; i++)
			{
				this->sensors[i].valueRead = false;
			}
			measureStart = this->clk.now();
		}
	}
}
