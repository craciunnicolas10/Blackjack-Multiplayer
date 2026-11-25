#pragma once
#include <platform.h>

#include <memory>
#include <string>

#include <Backend/GameEnums.h>
#include <Backend/IDeck.h>
#include <Backend/IGameListener.h>
#include <Backend/IPlayer.h>

namespace Backend {
    inline constexpr int kMaxPlayers = 4;
    inline constexpr int kStartingBalance = 10000;
    inline constexpr int kMinBet = 10;
    inline constexpr int kMaxBet = 10000000000000;

    // Punctul de intrare in logica jocului. Se creeaza cu IGame::Produce().
    //
    // Desfasurarea unei runde:
    //   AddPlayer (1-4 jucatori, doar in WaitingForPlayers)
    //   StartRound          -> Betting (din WaitingForPlayers sau RoundOver; strange runda veche)
    //   PlaceBet pentru fiecare jucator care are cel putin kMinBet; dupa ultimul pariu cartile
    //   se impart singure -> PlayersTurn
    //   Hit / Stand / DoubleDown pentru GetCurrentPlayerIndex(), pe rand
    //   dealerul joaca      -> DealerTurn, apoi plata -> RoundOver
    //
    // Functiile care intorc bool intorc false (si nu schimba nimic) daca actiunea nu e permisa
    // in starea curenta. Cat timp dealerul are o carte cu fata in jos, GetDealer() nu o arata
    // nici in GetHand(), nici in GetScore().
    //
    // BLACKJACK_API pe clasa, ca Produce() sa poata fi apelata din Frontend.
    class BLACKJACK_API IGame {
       public:
        virtual ~IGame() = default;

        // Joc cu un pachet de 52 de carti, amestecat aleator la fiecare runda.
        static std::shared_ptr<IGame> Produce();
        // Joc cu pachetul dat (pentru teste). Intoarce nullptr daca deck e nullptr.
        static std::shared_ptr<IGame> Produce(std::shared_ptr<IDeck> deck);

        virtual void AddListener(std::weak_ptr<IGameListener> listener) = 0;
        virtual void RemoveListener(IGameListener* listener) = 0;

        virtual bool AddPlayer(const std::string& name) = 0;
        virtual bool PlaceBet(int playerIndex, int amount) = 0;
        virtual bool StartRound() = 0;

        virtual bool Hit() = 0;
        virtual bool Stand() = 0;
        virtual bool DoubleDown() = 0;

        virtual EGameState GetState() const = 0;
        virtual int GetPlayerCount() const = 0;
        // -1 cand nu e randul nimanui.
        virtual int GetCurrentPlayerIndex() const = 0;
        // nullptr pentru un index invalid.
        virtual const IPlayer* GetPlayer(int playerIndex) const = 0;
        virtual const IPlayer* GetDealer() const = 0;

        // Jucatorul mai are de pus pariul in runda asta: se pariaza, n-a pariat inca si are cel
        // putin kMinBet (cine nu are e sarit).
        virtual bool CanPlaceBet(int playerIndex) const = 0;
        // Jucatorul la rand poate dubla acum: are doar primele doua carti si bani cat pariul.
        virtual bool CanDoubleDown() const = 0;
    };
}  // namespace Backend
