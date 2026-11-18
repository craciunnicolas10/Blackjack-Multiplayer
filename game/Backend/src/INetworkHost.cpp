#include <Backend/INetworkHost.h>

#include <Backend/net/network_host.h>
#include <Backend/net/protocol.h>

namespace Backend {
    std::shared_ptr<INetworkHost> INetworkHost::Produce(const std::string& hostName) {
        if (!IsValidName(hostName)) {
            return nullptr;
        }
        auto host = std::make_shared<NetworkHost>(hostName);
        if (!host->Listen()) {
            return nullptr;
        }
        return host;
    }
}  // namespace Backend
