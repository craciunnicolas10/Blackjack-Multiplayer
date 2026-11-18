#pragma once

namespace Frontend {
    class TableScreen;

    // De unde vine jocul pentru o masă: local (LocalSession) sau în rețea
    // (Net::HostSession la gazdă, Net::ClientSession la ceilalți). Masa o deține și o apelează;
    // restul mesei e la fel în toate cazurile.
    class GameSession {
       public:
        virtual ~GameSession() = default;

        // Leagă jocul de masă și pornește prima rundă. Apelată o dată, de constructorul mesei.
        virtual void start(TableScreen& table) = 0;
        // În fiecare frame (jocul în rețea își citește aici mesajele).
        virtual void update() {}
    };
}  // namespace Frontend
