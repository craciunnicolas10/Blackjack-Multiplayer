#pragma once

#include <SFML/Graphics/Color.hpp>

// Stilul comun al interfeței: culorile și mărimile de text folosite în toate ecranele.
// Când vrem să schimbăm aspectul (ex. alt auriu), îl schimbăm doar aici.
namespace Frontend::Theme {
    // --- Culori de bază ---
    inline constexpr sf::Color kGold{240, 210, 120};        // titluri, contururi, evidențieri
    inline constexpr sf::Color kGoldBright{255, 225, 130};  // conturul etichetei „Blackjack!”
    inline constexpr sf::Color kTextPrimary{255, 255, 255};
    inline constexpr sf::Color kTextSecondary{220, 220, 220};  // bani, pariu, informații
    inline constexpr sf::Color kTextMuted{190, 190, 190};      // indicii, limite
    inline constexpr sf::Color kPositive{130, 235, 130};       // sumă care crește, câștig
    inline constexpr sf::Color kNegative{245, 120, 110};       // sumă care scade, erori
    inline constexpr sf::Color kDanger{170, 40, 40};           // peste 21, pierdere
    inline constexpr sf::Color kShadow{0, 0, 0, 90};

    // --- Masa ---
    inline constexpr sf::Color kFeltCenter{30, 125, 72};  // postavul, la mijloc
    inline constexpr sf::Color kFeltEdge{6, 52, 28};      // postavul, spre margini
    inline constexpr sf::Color kTableLine{240, 210, 120, 150};
    inline constexpr sf::Color kTableText{240, 210, 120, 190};

    // --- Panouri și locuri ---
    inline constexpr sf::Color kPanelFill{10, 40, 22, 235};  // panoul de pariu, de rezultat
    inline constexpr sf::Color kSeatFill{0, 0, 0, 110};
    inline constexpr sf::Color kSeatOutline{255, 255, 255, 50};
    inline constexpr sf::Color kSeatActiveFill{90, 70, 15, 220};

    // --- Butoane ---
    inline constexpr sf::Color kButtonFill{170, 125, 25};
    inline constexpr sf::Color kButtonHover{200, 150, 30};
    inline constexpr sf::Color kButtonPressed{150, 110, 20};
    inline constexpr sf::Color kButtonDisabledFill{60, 60, 60};
    inline constexpr sf::Color kButtonDisabledOutline{90, 90, 90};
    inline constexpr sf::Color kButtonDisabledText{130, 130, 130};

    // --- Mărimi de text (în pixeli logici, la 1280x720) ---
    inline constexpr unsigned kTextHuge = 72;     // titlul din meniu
    inline constexpr unsigned kTextTitle = 48;    // titlul unui ecran
    inline constexpr unsigned kTextLarge = 34;    // sume importante (pariul din panou)
    inline constexpr unsigned kTextHeading = 28;  // titlul unui panou
    inline constexpr unsigned kTextButton = 26;
    inline constexpr unsigned kTextBody = 22;   // nume, etichete
    inline constexpr unsigned kTextSmall = 18;  // bani, pariu, scor
    inline constexpr unsigned kTextTiny = 14;   // indicii pe butoane
}  // namespace Frontend::Theme
