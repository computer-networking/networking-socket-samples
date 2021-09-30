#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
#define TARGET_IP "127.0.0.1"
#define RECV_BUFFER_LENGTH 2000

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

    // Clear the structure
    memset(&server_sockaddr, 0, structure_address_size);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        handle_error("socket");

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
    }
}
