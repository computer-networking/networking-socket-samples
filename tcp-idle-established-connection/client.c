#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

/*
    From tcp(7):
    A  newly created TCP socket has no remote or local address and is not fully specified.  To create an outgoing
    TCP connection use connect(2) to establish a connection to another TCP socket.  To receive new incoming  con‐
    nections, first bind(2) the socket to a local address and port and then call listen(2) to put the socket into
    the listening state.  After that a new socket for each incoming connection can be accepted  using  accept(2).
    A  socket which has had accept(2) or connect(2) successfully called on it is fully specified and may transmit
    data.  Data cannot be transmitted on listening or not yet connected sockets.
*/

// https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html

#define handle_error(msg) { perror(msg); exit(EXIT_FAILURE); }

#define TARGET_PORT 8990
//#define TARGET_IP "127.0.0.1"
#define TARGET_IP "192.168.88.221"
#define RECV_BUFFER_LENGTH 2000
#define ENABLE_KEEPALIVE 1
#define KEEPALIVE_IDLE_SECONDS 20
#define KEEPALIVE_INTERVAL 10
#define KEEPALIVE_MAX_COUNT 2
#define TCP_USER_TIMEOUT_MILLISECONDS 5000

void safe_print (FILE * stream, char * msg) {
    if (fprintf(stream, msg) < 0)
        handle_error("fprintf");
}

int main () {

    int sfd;
    struct sockaddr_in server_sockaddr;
    size_t  structure_address_size = sizeof(struct sockaddr_in);
    int recv_count;
    char client_buffer[RECV_BUFFER_LENGTH];
    char * message;
    int keepalive_enable = ENABLE_KEEPALIVE;
    int keepalive_seconds = KEEPALIVE_IDLE_SECONDS;
    int keepalive_interval_seconds = KEEPALIVE_INTERVAL;
    int keepalive_max_count = KEEPALIVE_MAX_COUNT;
    int tcp_user_timeout = TCP_USER_TIMEOUT_MILLISECONDS;

    // Clear the structure
    memset(&server_sockaddr, 0, structure_address_size);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        handle_error("socket");

    // Set Socket options
    if (setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive_enable, sizeof(keepalive_enable)) == -1)
        handle_error("setsockopt - SO_KEEPALIVE");

    if (setsockopt(sfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepalive_seconds, sizeof(keepalive_seconds)) == -1)
        handle_error("setsockopt - TCP_KEEPIDLE");

    if (setsockopt(sfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepalive_interval_seconds, sizeof(keepalive_interval_seconds)) == -1)
        handle_error("setsockopt - TCP_KEEPINTVL");

    if (setsockopt(sfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepalive_max_count, sizeof(keepalive_max_count)) == -1)
        handle_error("setsockopt - TCP_KEEPCNT");

    if (setsockopt(sfd, SOL_TCP, TCP_USER_TIMEOUT, (void *)&tcp_user_timeout, sizeof(tcp_user_timeout)) == -1)
        handle_error("setsockopt - TCP_USER_TIMEOUT");
    // ------------------

    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(TARGET_PORT);
    server_sockaddr.sin_addr.s_addr = inet_addr(TARGET_IP);

    if (connect(sfd, (struct sockaddr *) &server_sockaddr, structure_address_size) == -1) {
        handle_error("connect");
    } else {
        message = "CLIENT QUERY";

        if (send(sfd, message, strlen(message), 0) == -1)
            handle_error("send");

        if ((recv_count = recv(sfd, &client_buffer, RECV_BUFFER_LENGTH, 0)) == -1)
            handle_error("recv");

        if (recv_count == 0) {
            safe_print(stderr, "recv: got 0 bytes. Connection closed");
            exit(EXIT_SUCCESS);
        } else {
            safe_print(stderr, "recv: got package");
            safe_print(stdout, client_buffer);
        }

        if (close(sfd) == -1)
            handle_error("close");
    }
}
