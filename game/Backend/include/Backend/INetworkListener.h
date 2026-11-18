#pragma once

#include <string>
#include <vector>

#include <Backend/NetworkEnums.h>

namespace Backend {
    // Observer pentru jocul in retea: ce se intampla in afara mesei (lobby, start, iesiri).
    // Ce se intampla la masa vine, ca in jocul local, prin IGameListener pe GetGame().
    // Se inregistreaza cu INetworkSession::AddListener.
    class INetworkListener {
       public:
        virtual ~INetworkListener() = default;

        // Lista jucatorilor din lobby s-a schimbat: gazda prima, apoi in ordinea intrarii
        // (asta e si ordinea locurilor la masa).
        virtual void OnPlayersChanged(const std::vector<std::string>& names) = 0;
        // Doar la jucator: gazda a refuzat intrarea; legatura s-a inchis.
        virtual void OnJoinRejected(EJoinRejection reason) = 0;
        // Jocul a pornit: GetGame() si GetLocalSeat() sunt gata.
        virtual void OnGameStarted() = 0;
        // Nimeni nu mai are bani de pariu: nu mai urmeaza nicio runda.
        virtual void OnGameOver() = 0;
        // Un jucator a iesit in timpul jocului; jocul s-a oprit pentru toti.
        virtual void OnPlayerLeft(const std::string& name) = 0;
        // Doar la jucator: gazda a inchis jocul sau legatura cu ea s-a rupt.
        virtual void OnHostLeft() = 0;
    };
}  // namespace Backend
