#include <screens/menu_screen.h>

#include <memory>

#include <app/layout.h>
#include <screens/lobby_screen.h>
#include <screens/screen_manager.h>
#include <ui/crisp_text.h>
#include <ui/theme.h>
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
          m_banner(context.font, context.cards),
          m_subtitle(context.font, utf8("Echipa Dealerii"), Theme::kTextHeading) {
        m_banner.setPosition({kLogicalWidth / 2.f, 150.f});

        m_subtitle.setFillColor(Theme::kTextSecondary);
        centerHorizontally(m_subtitle, 262.f);

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

    void MenuScreen::update(float dt) { m_banner.update(dt); }

    void MenuScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);
        m_banner.draw(target);
        drawText(target, m_subtitle);
        for (const Button& button : m_buttons) {
            button.draw(target);
        }
    }
}  // namespace Frontend
