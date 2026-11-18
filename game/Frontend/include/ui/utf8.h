#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <SFML/System/String.hpp>

namespace Frontend {
    // Convertește text UTF-8 (cu diacritice) în sf::String. Fără asta, SFML interpretează
    // un const char* după locale-ul sistemului și diacriticele ies greșit.
    inline sf::String utf8(std::string_view text) {
        return sf::String::fromUtf8(text.begin(), text.end());
    }

    // Aceeași conversie pentru o listă (ex. numele jucătorilor primite de la Backend).
    inline std::vector<sf::String> utf8All(const std::vector<std::string>& texts) {
        std::vector<sf::String> result;
        result.reserve(texts.size());
        for (const std::string& text : texts) {
            result.push_back(utf8(text));
        }
        return result;
    }

    // Invers: sf::String → UTF-8 în std::string (ex. numele trimise Backend-ului).
    inline std::string toUtf8String(const sf::String& text) {
        const auto utf8 = text.toUtf8();
        return {utf8.begin(), utf8.end()};
    }
}  // namespace Frontend
