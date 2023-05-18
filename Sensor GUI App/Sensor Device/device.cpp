#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <string.h>
#include <unistd.h>

// LINUX Specific includes 
// TODO: Add Windows Compatiblity
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/time.h>

#include "device.h"

using namespace std;

/*----------- Local vars ------------------*/

bool timeout_flag;
bool * pTimeout = &timeout_flag;


/*----------- Timer Signal Handler ------------------*/
static void handler(int signum)
{
    timeout_flag = true;
}

/*----------- Class test_device ------------------*/
test_device::test_device(const char * addr, const char * port, bool * timeout)
{
    /*check input port and address values*/
    if (this->socket_init(addr, port) == 0)
    {
        this->flags.socketInit = true;
        cout << "Socket setup successfully!\n";

        /* Initial state set */
        this->state = STATE_IDLE;

        this->fds = {};

        /* Reset flags */
        this->flags.startMsgRx = false;
        this->flags.timerInit = false;
        
        this->timeout = timeout;
        *this->timeout = false;
    }
    else
    {
        this->flags.socketInit = false;
    }
}

int test_device::socket_init(const char* addr, const char* port)
{
    /* create blank pointer to addrinfo struct to hold current port details */
    this->current_port_info = new struct addrinfo;

    /* create hints address info struct to find valid open port for specified address */
    struct addrinfo hints;

    /* Blank UDP Socket holding var */
    this->udp_sock = (int)NULL;

    hints.ai_family = AF_INET;              /* IPV4 address family */
    hints.ai_socktype = SOCK_DGRAM;         /* Datagram socket-type */
    hints.ai_protocol = IPPROTO_UDP;        /* Protocol is UDP */

    if((getaddrinfo(addr, port, &hints, &(this->current_port_info)) != 0) || (this->current_port_info == NULL))
    {
        /* Handle port not found */
        cout << "Cannot find valid open port for specified Address and Port combination!\n";
        return -1;
    }

    this->udp_sock = socket(this->current_port_info->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    
    if(this->udp_sock == -1)
    {
        /* Handle error in opening socket */
        cout << "Cannot create socket instance!\n";
        return -1;
    }

    if (bind(this->udp_sock, this->current_port_info->ai_addr, this->current_port_info->ai_addrlen) != 0)
    {
        cout << "Cannot bind port to socket!\n";
        this->socket_close();
        return -1;
    }

    return 0;
}

int test_device::socket_send(const string * msg_tx_buf)
{
    const socklen_t sock_len = sizeof(this->test_port_info);
    return sendto(this->udp_sock, (void *)msg_tx_buf, strlen((char *)msg_tx_buf), 0, (sockaddr *)&(this->test_port_info), sock_len);
}

int test_device::socket_wait_rx(void)
{
    char * ptr = &(this->msgBuf[0]);
    socklen_t sock_len = sizeof(this->test_port_info);
    return recvfrom(this->udp_sock, ptr, MAX_MSG_LENGTH, 0, (sockaddr *)&(this->test_port_info), &sock_len);
}

int test_device::socket_close (void)
{
    if (this->udp_sock)
    {
        /* Check socket exists before closing */
        close(this->udp_sock);
        return 0;
    }
    /* Return fault value since socket doesn't exist -> cannot close */
    return -1;
}

void test_device::init_timer(void)
{
    /* Set itimer to timeout based on input test rate */
    struct itimerval idle;
    int rate_in_usecs = (this->test_rate * 1000);   // Convert rate in ms to microseconds

    idle.it_value.tv_sec  = int(rate_in_usecs / (1000000));
    idle.it_value.tv_usec = int(rate_in_usecs % (1000000));

    idle.it_interval.tv_sec = int(rate_in_usecs / (1000000));
    idle.it_interval.tv_usec = int(rate_in_usecs % (1000000));

    /* Set itimer to run in real-time mode with test-rate timeout val */
    setitimer(ITIMER_REAL, &idle, NULL);

    /* Attach class function to the SIGALRM signal generated on timeout of interrupt timer */
    struct sigaction sa;
    sa.sa_handler = handler;    /* handler function to operate on timeout */
    sigemptyset(&sa.sa_mask);   /* no masks applied */
    sa.sa_flags = SA_RESTART;   /* restart operation of function if interrupt blocked */

    /* Attach the sa sigaction struct operands to the SIGALRM signal */
    sigaction(SIGALRM, &sa, 0);

    /* Initialise seed for random number generation */
    srand((unsigned)time(NULL));

    /* signal initialisation complete */
    this->flags.timerInit = true;

    /* Set timeout flag true to trigger the first sending of status packet */
    *this->timeout = true;
}

void test_device::deinit_timer(void)
{
    /* Disable itimer with 0 value timeout */
    struct itimerval idle;

    idle.it_value.tv_sec  = 0;
    idle.it_value.tv_usec = 0;
    idle.it_interval.tv_sec = 0;
    idle.it_interval.tv_usec = 0;

    /* Set itimer to run in real-time mode with test-rate timeout val */
    setitimer(ITIMER_REAL, &idle, NULL);

    /* Remove class function to the SIGALRM signal generated on timeout of interrupt timer */
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;    /* Default Handler */

    sigaction(SIGALRM, &sa, 0);

    /* Reset flag for next server loop */
    *this->timeout = false;

    this->flags.timerInit = false;
}

void test_device::run(void)
{
    switch(this->state)
    {
        case STATE_IDLE:
        {
            /* Add a waiting RX if the initial start message not received */
            cout << "WAITING FOR START CMD\n";
            this->socket_wait_rx();
            char * savePtrPkt;
            char * savePtrCmd;
            char * ptr = strtok_r(&(this->msgBuf[0]), DELIM, &savePtrPkt);

            if (strcmp(ptr, VALID_IDENTIFIER) != 0)
            {
                /*Non-test device packet received*/
                return;
            }

            ptr = strtok_r(NULL, DELIM, &savePtrPkt);

            if (strcmp(ptr, START_CMD) != 0)
            {
                /*handle non-start command*/
                char err_pkt[MAX_MSG_LENGTH];
                sprintf(err_pkt, "%s;%s;%s=Invalid command. START command required\n", VALID_IDENTIFIER, ERROR_ACK_CMD, MSG_CMD);
                this->socket_send((string *)&err_pkt);
                return;
            }
                
            ptr = strtok_r(NULL, DELIM, &savePtrPkt);
            
            char * durPtr = strtok_r(ptr, CMD_DELIM, &savePtrCmd);

            if (strcmp(durPtr, DURATION_CMD) != 0)
            {
                /* Handle invalid duration value */
                char err_pkt[MAX_MSG_LENGTH];
                sprintf(err_pkt, "%s;%s;%s=Invalid duration paramater\n", VALID_IDENTIFIER, ERROR_ACK_CMD, MSG_CMD);
                this->socket_send((string *)&err_pkt);
                return;
            }
            else
            {
                durPtr = strtok_r(NULL, CMD_DELIM, &savePtrCmd);
                this->test_duration = stoi(durPtr);
                // cout << this->test_duration << '\n';
            }

            ptr = strtok_r(NULL, DELIM, &savePtrPkt);

            char * ratePtr = strtok_r(ptr, CMD_DELIM, &savePtrCmd);

            if (strcmp(ratePtr, RATE_CMD) != 0)
            {
                /* Handle invalid range value */
                char err_pkt[MAX_MSG_LENGTH];
                sprintf(err_pkt, "%s;%s;%s=Invalid rate paramater\n", VALID_IDENTIFIER, ERROR_ACK_CMD, MSG_CMD);
                this->socket_send((string *)&err_pkt);
                return;
            }
            else
            {
                ratePtr = strtok_r(NULL, CMD_DELIM, &savePtrCmd);
                this->test_rate = stoi(ratePtr);
                // cout << this->test_rate << '\n';
            }

            this->state = STATE_TEST_PROGRESS;
            // cout << "Updated run state\n";
            this->test_start_time = chrono::system_clock::now();

            char start_ack_pkt[MAX_MSG_LENGTH];
            sprintf(start_ack_pkt, "%s;%s;", VALID_IDENTIFIER, START_ACK_CMD);
            this->socket_send((string *)&start_ack_pkt);
            // cout << "The last error message is: %s\n" << strerror(errno);
            break;
        }
        
        case STATE_TEST_PROGRESS:
        {
            if (!this->flags.timerInit)
            {
                // cout << "initialising the timer\n";
                this->init_timer();
                
                /* Setup polling on UDP Socket to check incoming packets for stop command */                
                this->fds.fd = this->udp_sock;
                this->fds.events = POLLIN;
            }
            
            /* Poll UDP Socket and check if data waiting to be recieved */
            int poll_ret = poll(&(this->fds), 1, POLLING_TIMEOUT);

            /* If data rxed, parse to check stop command */
            if((poll_ret > 0)&& (this->fds.revents & POLLIN))
            {
                this->socket_wait_rx();
                char * ptr = strtok(&(this->msgBuf[0]), DELIM);
                if (strcmp(ptr, VALID_IDENTIFIER) == 0)
                {
                    ptr = strtok(NULL, DELIM);
                    if (strcmp(ptr, STOP_CMD) == 0)
                    {
                        this->deinit_timer();
                        this->state = STATE_END;
                    }
                }
            }

            if(*this->timeout)
            {
                // cout << "timeout triggered\n";
                int millivolts = int(rand() % (MAX_VAL + 1));

                /* Use spoofed resistance value to calc milliamps*/
                int milliamps = int(millivolts / RESISTANCE_VAL);
                
                chrono::duration<double, milli> elapsedTime = chrono::system_clock::now() - (this->test_start_time);
                
                /*Construct status update packet*/
                char status_pkt[MAX_MSG_LENGTH];
                sprintf(status_pkt, "%s;TIME=%f;MV=%d;MA=%d;", STATUS_CMD, elapsedTime.count(), millivolts, milliamps);
                this->socket_send((string *)&status_pkt);

                *this->timeout = false;

                if (elapsedTime.count() >= (this->test_duration * 1000))
                /* Test duration over, terminate data sending */
                {
                    this->deinit_timer();
                    this->state = STATE_END;
                }
            }
            
            break;
        }
        
        case STATE_END:
        {
            /*Construct test stopped packet*/
            char stop_pkt[MAX_MSG_LENGTH];
            sprintf(stop_pkt, "%s;%s;", VALID_IDENTIFIER, STOP_ACK_CMD);
            this->socket_send((string *)&stop_pkt);

            cout << "TEST COMPLETED\n";

            this->state = STATE_IDLE;
            break;
        }
        
        default:
            break;
    }
}

/*----------- Main ------------------*/
int main(int argc, char* argv[])
{
    if(argc > 2)
    {
        char * ip_add = argv[1];
        char * port_num = argv[2];

        cout << "Setting up test device for address " << ip_add << ":" << port_num << '\n';

        test_device * t_dev = new test_device(ip_add, port_num, pTimeout);

        if (!t_dev->flags.socketInit) return EXIT_FAILURE;

        while(1)
        {
            t_dev->run();
        }

        return EXIT_SUCCESS;
    }

    cout << "IP ADDRESS/PORT NOT ENTERED FOR TEST DEVICE!\n";
    return EXIT_FAILURE;
}
