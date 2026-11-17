#include <screens/menu_screen.h>

#include <memory>

#include <app/layout.h>
#include <screens/lobby_screen.h>
#include <screens/screen_manager.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        void centerHorizontally(sf::Text& text, float y) {
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
            text.setPosition({kLogicalWidth / 2.f, y});
        }
    }  // namespace

    MenuScreen::MenuScreen(AppContext context)
        : m_context(context),
          m_title(context.font, utf8("Blackjack"), 72),
          m_subtitle(context.font, utf8("Echipa Dealerii"), 28) {
        m_title.setFillColor(sf::Color(240, 210, 120));
        centerHorizontally(m_title, 150.f);

        m_subtitle.setFillColor(sf::Color(200, 200, 200));
        centerHorizontally(m_subtitle, 250.f);

        const sf::Vector2f buttonSize{280.f, 64.f};
        const float buttonX = (kLogicalWidth - buttonSize.x) / 2.f;

        Button play(context.font, utf8("Joacă"), {buttonX, 360.f}, buttonSize);
        play.setOnClick([context] {
            context.screens.change(std::make_unique<LobbyScreen>(context));
        });
        m_buttons.push_back(std::move(play));

        Button quit(context.font, utf8("Ieșire"), {buttonX, 450.f}, buttonSize);
        quit.setOnClick([context] { context.screens.requestQuit(); });
        m_buttons.push_back(std::move(quit));
    }

    void MenuScreen::handleEvent(const sf::Event& event) {
        for (Button& button : m_buttons) {
            button.handleEvent(event, m_context.window);
        }
    }

    void MenuScreen::draw(sf::RenderTarget& target) {
        sf::RectangleShape background({kLogicalWidth, kLogicalHeight});
        background.setFillColor(sf::Color(20, 60, 35));
        target.draw(background);

        target.draw(m_title);
        target.draw(m_subtitle);
        for (const Button& button : m_buttons) {
            button.draw(target);
        }
    }
}  // namespace Frontend
