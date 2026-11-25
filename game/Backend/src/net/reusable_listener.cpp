#include <Backend/net/reusable_listener.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace Backend {
    bool ReusableListener::ListenReusable(unsigned short port) {
#ifdef _WIN32
        return listen(port) == sf::Socket::Status::Done;
#else
        // Ce face listen() in SFML, plus SO_REUSEADDR inainte de bind.
        close();
        create();
        const int handle = getNativeHandle();
        if (handle < 0) {
            return false;
        }
        const int reuse = 1;
        setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            ::listen(handle, SOMAXCONN) != 0) {
            close();
            return false;
        }
        return true;
#endif
    }
}  // namespace Backend
