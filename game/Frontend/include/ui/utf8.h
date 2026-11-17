#pragma once

#include <string_view>

#include <SFML/System/String.hpp>

namespace Frontend {
    // Convertește text UTF-8 (cu diacritice) în sf::String. Fără asta, SFML interpretează
    // un const char* după locale-ul sistemului și diacriticele ies greșit.
    inline sf::String utf8(std::string_view text) {
        return sf::String::fromUtf8(text.begin(), text.end());
    }
}  // namespace Frontend
