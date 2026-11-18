#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Backend/IGame.h>
#include <Backend/INetworkListener.h>

namespace Backend {
    // Partea comuna a jocului in retea, la gazda (INetworkHost) si la jucator (INetworkClient).
    // Fiecare calculator joaca un singur loc. Nu foloseste fire de executie: Update() citeste
    // ce a venit din retea si anunta ascultatorii, deci trebuie apelata des (ex. la fiecare
    // frame).
    class INetworkSession {
       public:
        virtual ~INetworkSession() = default;

        virtual void AddListener(std::weak_ptr<INetworkListener> listener) = 0;
        virtual void RemoveListener(INetworkListener* listener) = 0;

        virtual void Update() = 0;

        virtual bool IsHost() const = 0;
        // Gazda prima, apoi ceilalti in ordinea intrarii (= locurile la masa).
        virtual std::vector<std::string> GetPlayerNames() const = 0;
        // Locul jucatorului de pe calculatorul asta; -1 cat timp nu se stie (inainte de start).
        virtual int GetLocalSeat() const = 0;
        // Jocul, dupa OnGameStarted (nullptr inainte). Are aceeasi interfata ca jocul local:
        // la gazda e jocul real, la jucator o copie tinuta la zi din retea, ale carei actiuni
        // (PlaceBet, Hit, ...) pleaca la gazda. Se apeleaza doar pentru locul de pe calculatorul
        // asta; gazda verifica oricum ca e randul lui.
        virtual std::shared_ptr<IGame> GetGame() const = 0;
    };
}  // namespace Backend
