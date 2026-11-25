#pragma once

#include <SFML/Network/TcpListener.hpp>

namespace Backend {
    // Un sf::TcpListener al carui port poate fi ocupat din nou imediat dupa ce s-a inchis un
    // joc. Fara asta, sistemul tine portul blocat inca vreun minut dupa ce gazda inchide
    // conexiunile, si un joc nou creat intre timp nu-l mai poate folosi.
    class ReusableListener : public sf::TcpListener {
       public:
        // Ca listen(port), dar cu SO_REUSEADDR (pe Linux / macOS). Pe Windows ramane listen()
        // obisnuit: acolo SO_REUSEADDR ar lasa alt program sa ocupe acelasi port.
        // Un port pe care asculta deja alt program ramane refuzat.
        bool ListenReusable(unsigned short port);
    };
}  // namespace Backend
