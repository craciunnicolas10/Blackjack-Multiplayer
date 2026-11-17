#include <screens/table_screen.h>

#include <array>
#include <memory>
#include <string>

#include <app/layout.h>
#include <screens/menu_screen.h>
#include <screens/screen_manager.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr sf::Vector2f kCardSize{90.f, 126.f};
        constexpr float kCardOffset = 40.f;  // cât e mutată fiecare carte față de cea dinainte

        constexpr sf::Vector2f kDealerPosition{kLogicalWidth / 2.f, 70.f};
        constexpr std::array<sf::Vector2f, 4> kSeatPositions{{
            {200.f, 380.f},
            {493.f, 410.f},
            {787.f, 410.f},
            {1080.f, 380.f},
        }};
    }  // namespace

    TableScreen::TableScreen(AppContext context) : m_context(context) {
        const sf::Vector2f actionSize{150.f, 56.f};
        const float actionY = kLogicalHeight - actionSize.y - 24.f;
        const float actionsWidth = 3 * actionSize.x + 2 * 20.f;
        float actionX = (kLogicalWidth - actionsWidth) / 2.f;

        for (const char* label : {"Hit", "Stand", "Double"}) {
            Button button(context.font, utf8(label), {actionX, actionY}, actionSize);
            // Butoanele se activează când jocul real spune că e rândul unui jucător.
            button.setEnabled(false);
            m_actionButtons.push_back(std::move(button));
            actionX += actionSize.x + 20.f;
        }

        Button menu(context.font, utf8("Meniu"), {24.f, 24.f}, {140.f, 48.f});
        menu.setOnClick([this] { goToMenu(); });
        m_otherButtons.push_back(std::move(menu));
    }

    void TableScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                goToMenu();
                return;
            }
        }

        for (Button& button : m_actionButtons) {
            button.handleEvent(event, m_context.window);
        }
        for (Button& button : m_otherButtons) {
            button.handleEvent(event, m_context.window);
        }
    }

    void TableScreen::draw(sf::RenderTarget& target) {
        sf::RectangleShape background({kLogicalWidth, kLogicalHeight});
        background.setFillColor(sf::Color(15, 90, 50));
        target.draw(background);

        // Dealerul: 2 cărți, a doua cu fața în jos.
        drawLabel(target, utf8("Dealer"), {kDealerPosition.x, kDealerPosition.y - 46.f});
        const float dealerHandWidth = kCardSize.x + kCardOffset;
        const float dealerX = kDealerPosition.x - dealerHandWidth / 2.f;
        drawCardPlaceholder(target, {dealerX, kDealerPosition.y}, false);
        drawCardPlaceholder(target, {dealerX + kCardOffset, kDealerPosition.y}, true);

        // Cele 4 locuri ale jucătorilor.
        for (std::size_t seat = 0; seat < kSeatPositions.size(); ++seat) {
            const sf::Vector2f center = kSeatPositions.at(seat);
            const float handWidth = kCardSize.x + kCardOffset;
            const float handX = center.x - handWidth / 2.f;
            drawCardPlaceholder(target, {handX, center.y}, false);
            drawCardPlaceholder(target, {handX + kCardOffset, center.y}, false);
            drawLabel(target, utf8("Loc " + std::to_string(seat + 1)),
                      {center.x, center.y + kCardSize.y + 10.f});
        }

        for (const Button& button : m_actionButtons) {
            button.draw(target);
        }
        for (const Button& button : m_otherButtons) {
            button.draw(target);
        }
    }

    // TODO(C): înlocuiește cu CardView când e gata.
    void TableScreen::drawCardPlaceholder(sf::RenderTarget& target, sf::Vector2f position,
                                          bool faceDown) const {
        sf::RectangleShape card(kCardSize);
        card.setPosition(position);
        // Fața ascunsă e albastră, ca spatele unei cărți; restul sunt gri.
        card.setFillColor(faceDown ? sf::Color(40, 70, 150) : sf::Color(200, 200, 200));
        card.setOutlineColor(sf::Color(60, 60, 60));
        card.setOutlineThickness(2.f);
        target.draw(card);
    }

    void TableScreen::drawLabel(sf::RenderTarget& target, const sf::String& text,
                                sf::Vector2f centerTop) const {
        sf::Text label(m_context.font, text, 22);
        const sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
        label.setPosition(centerTop);
        label.setFillColor(sf::Color::White);
        target.draw(label);
    }

    void TableScreen::goToMenu() {
        m_context.screens.change(std::make_unique<MenuScreen>(m_context));
    }
}  // namespace Frontend
