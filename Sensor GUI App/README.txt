Dhairya Trivedi

Sensor GUI Display App

NOTE: This is based on a LINUX Environment
TODO: Add windows environment features

Steps to run:
1. Run "sudo Python3 app.py" in a terminal window opened in the source code directory

2. This should pop-up a QT app window

3. In a separate terminal instance (in the source code directory again), run the command
"make clean && make all DEVICE_IP=/*ip_address* DEVICE_PORT=/*port*/"

4.This should generate the simulated device test with a socket at the specified IP Address (ip_address)
and port. If these arguments are not entered, the code will default to IP=127.0.0.1 and PORT=8080

5.Once the server is setup successfully, it should print out "WAITING FOR START CMD..." in the terminal

6.Now enter the relevant IP Address and port into the QT App, and set the duration/rate and press start.
When entering the ip address, be sure to not use trailing zeros (such as 127.00.00.01, instead use 127.0.0.1).

7. The chart should start to automatically update with values simulated from the device, and be displayed live

8. When duration is finished, the chart will stop updating, and the start button will become active, making 
it possible to start another session

9. If the duration hasn't finished, the user can press stop to halt the device at that moment.

If the device has been generated once using "make all", then the same output binary can be run directly
without recompiling and building. For this use the command "make run". The same ip address and port arguments
can be passed through to run a server.