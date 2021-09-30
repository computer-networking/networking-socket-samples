# tcp-idle-established-connection

On this sample the idea is to do the following exercises:

Exercise 1)

1. Trigger tcpdump on VM 1
```
tcpdump -i <interface> -nnvvX '(dst port 8990) or (src port 8990)'
```
2. Load the server into a VM 2.
3. Using `simple-tcp-socket/client.c` Load the client into a VM 1.
4. After the first exchange, where the client is blocked waiting for recv, update the firewall in VM2 to drop all packets on the port.
5. Observe how the keep alive works and when it times out the connection.

Exercise 2)

1. Trigger tcpdump on VM 1
```
tcpdump -i <interface> -nnvvX '(dst port 8990) or (src port 8990)'
```
2. Load the server into a VM 2.
3. Load the client into a VM 1.
4. After the first exchange, where the client is blocked waiting for recv, update the firewall in VM2 to drop all packets on the port.
5. Observe how the keep alive works and when it times out the connection.

### Notes

socket(7) and socket(2):
- *SO_KEEPALIVE*: Enable  sending  of  keep-alive  messages  on connection-oriented sockets.  Expects an integer boolean flag.
- When `SO_KEEPALIVE` is enabled on the socket the protocol checks in a protocol-specific manner if the other end is still alive.

tcp(7) :
- *TCP_USER_TIMEOUT*: This option can be set during any state of a TCP connection, but is effective only during the synchronized states of a connection (ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOSE-WAIT, CLOSING, and LAST-ACK).  Moreover, when used with the TCP keepalive (SO_KEEPALIVE) option, TCP_USER_TIMEOUT will override keepalive to determine when to close a connection due to keepalive failure. The option has no effect on when TCP retransmits a packet, nor when a keepalive probe is sent.
- *TCP_KEEPCNT*: The maximum number of keepalive probes TCP should send before dropping the connection. Global value at `/proc/sys/net/ipv4/tcp_keepalive_probes` (9 probes).
- *TCP_KEEPIDLE*: The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes, if the socket option SO_KEEPALIVE has been set on this socket. Global value at i`/proc/sys/net/ipv4/tcp_keepalive_time` (7200 seconds = 2 hs).
- *TCP_KEEPINTVL*: The time (in seconds) between individual keepalive probes. Global value at `/proc/sys/net/ipv4/tcp_keepalive_intvl` (75 seconds).

Global retry count: `/proc/sys/net/ipv4/tcp_retries2` (15 times).

Behavior when using TCP_USER_TIMEOUT in combination with SO_KEEPALIVE, from the kernlel's tpc timer implementation [net/ipv4/tcp_timer.c](https://github.com/torvalds/linux/blob/e3ae2365efc14269170a6326477e669332271ab3/net/ipv4/tcp_timer.c#L723-L730) we can see that [icsk_user_timeout is TCP_USER_TIMEOUT](https://github.com/torvalds/linux/blob/9e9fb7655ed585da8f468e29221f0ba194a5f613/net/ipv4/tcp.c#L4094-L4096)
```
		if ((icsk->icsk_user_timeout != 0 &&
		    elapsed >= msecs_to_jiffies(icsk->icsk_user_timeout) &&
		    icsk->icsk_probes_out > 0) ||
		    (icsk->icsk_user_timeout == 0 &&
		    icsk->icsk_probes_out >= keepalive_probes(tp))) {
```
This means it kills the connection on:
```
    (TCP_USER_TIMEMOUT != 0 AND ELAPSED > TCP_USER_TIMEMOUT AND PROBES > 0)
    OR
    (TCP_USER_TIMEMOUT = 0 AND PROBES > TCP_KEEPCNT)
```
Thus, setting TCP_USER_TIMEOUT makes to ignore TCP_KEEPCNT, but would still use TCP_KEEPIDLE to trigger the probes and TCP_KEEPINTVL for how often they run.

Other references:
- [The linux documentation project > TCP-Keepalive-HOWTO](https://tldp.org/HOWTO/TCP-Keepalive-HOWTO/usingkeepalive.html)
- [torvalds/linux TCP_USER_TIMEOUT commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=dca43c75e7e545694a9dd6288553f55c53e2a3a3)

### Conclusion

As TCP idle ESTABLISHED connections can be problematic stale connections, using TCP_USER_TIMEOUT in convination of SO_KEEPALIVE, would:

Start keepalive probes.
Good) Ensuring the connection even if idle, it is still healthy,
Bad) Add extra load into the network.

Force to timeout when the server is not healthy.
Good) Ensures to not waste time from the client side.
Bad) Non easy configuration. Obscure behavior on how TCP_USER_TIMEOUT overrides when to close a connection due to keep alive failures.
