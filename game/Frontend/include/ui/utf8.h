#pragma once

#include <string>
#include <string_view>

#include <SFML/System/String.hpp>

namespace Frontend {
    // Convertește text UTF-8 (cu diacritice) în sf::String. Fără asta, SFML interpretează
    // un const char* după locale-ul sistemului și diacriticele ies greșit.
    inline sf::String utf8(std::string_view text) {
        return sf::String::fromUtf8(text.begin(), text.end());
    }

    // Invers: sf::String → UTF-8 în std::string (ex. numele trimise Backend-ului).
    inline std::string toUtf8String(const sf::String& text) {
        const auto utf8 = text.toUtf8();
        return {utf8.begin(), utf8.end()};
    }
}  // namespace Frontend
