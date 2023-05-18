#ifndef UDP_TEST_DEVICE
#define UDP_TEST_DEVICE

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <signal.h>
#include <chrono>
#include <poll.h>

using namespace std;

#define MAX_MSG_LENGTH      65      /* NOTE: need to update this if longer messages need to be sent */
#define MAX_IP_ADD_LENGTH   10

#define VALID_IDENTIFIER    "TEST"
#define DELIM               ";"
#define CMD_DELIM           "="
#define START_CMD           "CMD=START"
#define STOP_CMD            "CMD=STOP"
#define DURATION_CMD        "DURATION"
#define RATE_CMD            "RATE"
#define STATUS_CMD          "STATUS"
#define MSG_CMD             "MSG"
#define START_ACK_CMD       "RESULT=STARTED"
#define STOP_ACK_CMD        "RESULT=STOPPED"
#define ERROR_ACK_CMD       "RESULT=ERROR"

#define MAX_VAL             100
#define RESISTANCE_VAL      4

#define POLLING_TIMEOUT     1

typedef enum
{
    STATE_IDLE,
    STATE_START,
    STATE_TEST_PROGRESS,
    STATE_END
} TestDeviceStates;

typedef struct
{
    bool socketInit;
    bool startMsgRx;
    bool timerInit;
} TestFlags;

class test_device
{
public:
    /* Constructor */
    test_device(const char * addr, const char * port, bool * timeout);

    /* Destructor */
    ~test_device();

    /* Main Running function */
    void run(void);

    /* Current state of test device var */
    TestDeviceStates state;

    /* Flags var */
    TestFlags flags;

private:
    /* Vars to hold self-socket detail */
    int udp_sock;
    char msgBuf[MAX_MSG_LENGTH];
    struct addrinfo * current_port_info;

    /* Var to hold test socket detail */
    struct sockaddr_in test_port_info;

    bool * timeout;

    /* Test setup vars */
    int test_rate;
    int test_duration;
    chrono::time_point<chrono::system_clock> test_start_time;
    struct pollfd fds;

    /* Socket operation functions */
    int socket_init(const char* addr, const char* port);
    int socket_wait_rx(void);
    int socket_send(const string * msg_tx_buf);
    int socket_close(void);

    /* Test-handling functions*/
    void init_timer(void);
    void deinit_timer(void);
};

#endif
