#include "checknetwork.h"

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

CheckNetwork::CheckNetwork(QObject *parent) : QObject(parent)
{

}

bool CheckNetwork::isHostAndPortConnect(const QString &host, const QString &port)
{
    addrinfo *result;
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;   // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    char addressString[INET6_ADDRSTRLEN];
    const char *retval = nullptr;
    if (0 != getaddrinfo(host.toUtf8().toStdString().c_str(), port.toUtf8().toStdString().c_str(), &hints, &result)) {
        return false;
    }
    for (addrinfo *addr = result; addr != nullptr; addr = addr->ai_next) {
        int handle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (handle == -1) {
            continue;
        }
        if (::connect(handle, addr->ai_addr, addr->ai_addrlen) != -1) {
            switch(addr->ai_family) {
                case AF_INET:
                    retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr), addressString, INET6_ADDRSTRLEN);
                    break;
                case AF_INET6:
                    retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in6 *>(addr->ai_addr)->sin6_addr), addressString, INET6_ADDRSTRLEN);
                    break;
                default:
                    // unknown family
                    retval = nullptr;
            }
            close(handle);
            break;
        }
    }
    freeaddrinfo(result);
    return retval != nullptr;
}
