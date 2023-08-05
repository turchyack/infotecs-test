#if !defined(__CHAT_TYPES_HPP_INCLUDED)
#define __CHAT_TYPES_HPP_INCLUDED

#include <sys/socket.h>
#include <netinet/in.h>

union SocketAddress {
    sockaddr_in sa_in;
    sockaddr sa;
};

#endif



