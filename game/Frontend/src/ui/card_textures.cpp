#include <ui/card_textures.h>

#include <iostream>
#include <string>

namespace Frontend {
    namespace {
        constexpr const char* kCardsFolder = "assets/cards/";

        // Numele fișierelor: "<rang>_of_<culoare>.png", de ex. "10_of_hearts.png",
        // "ace_of_spades.png".
        std::string rankName(int rank) {
            switch (rank) {
                case 11:
                    return "jack";
                case 12:
                    return "queen";
                case 13:
                    return "king";
                case 14:
                    return "ace";
                default:
                    return std::to_string(rank);
            }
        }

        const char* suitName(int suit) {
            constexpr const char* kNames[kSuitCount] = {"clubs", "diamonds", "hearts", "spades"};
            return kNames[suit];
        }

        bool isValid(int rank, int suit) {
            return rank >= kMinRank && rank <= kMaxRank && suit >= 0 && suit < kSuitCount;
        }

        std::size_t indexOf(int rank, int suit) {
            return static_cast<std::size_t>((rank - kMinRank) * kSuitCount + suit);
        }

        bool loadTexture(sf::Texture& texture, const std::string& path) {
            // Calea e relativă la folderul curent: porniți aplicația din out/install/<preset>/bin.
            if (!texture.loadFromFile(path)) {
                std::cerr << "Nu am putut incarca imaginea cartii: " << path << '\n';
                return false;
            }
            // Imaginile (250x363) sunt micșorate la afișare; fără smooth ies zimțate.
            texture.setSmooth(true);
            return true;
        }
    }  // namespace

    bool CardTextures::load() {
        bool ok = loadTexture(m_back, std::string(kCardsFolder) + "back.png");

        for (int rank = kMinRank; rank <= kMaxRank; ++rank) {
            for (int suit = 0; suit < kSuitCount; ++suit) {
                const std::string path =
                    std::string(kCardsFolder) + rankName(rank) + "_of_" + suitName(suit) + ".png";
                // Continuăm și după o eroare, ca să apară în consolă toate fișierele lipsă.
                ok = loadTexture(m_faces[indexOf(rank, suit)], path) && ok;
            }
        }
        return ok;
    }

    const sf::Texture& CardTextures::get(int rank, int suit) const {
        if (!isValid(rank, suit)) {
            return m_back;
        }
        return m_faces[indexOf(rank, suit)];
    }

    const sf::Texture& CardTextures::back() const { return m_back; }
}  // namespace Frontend
