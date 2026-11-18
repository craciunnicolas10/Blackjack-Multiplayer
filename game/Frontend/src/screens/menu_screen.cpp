#include <screens/menu_screen.h>

#include <memory>

#include <app/layout.h>
#include <screens/host_lobby_screen.h>
#include <screens/join_lobby_screen.h>
#include <screens/lobby_screen.h>
#include <screens/screen_manager.h>
#include <ui/crisp_text.h>
#include <ui/text_layout.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    MenuScreen::MenuScreen(AppContext context)
        : m_context(context),
          m_banner(context.font, context.cards),
          m_subtitle(context.font, utf8("Echipa Dealerii"), Theme::kTextHeading) {
        m_banner.setPosition({kLogicalWidth / 2.f, 150.f});

        m_subtitle.setFillColor(Theme::kTextSecondary);
        centerHorizontally(m_subtitle, 262.f);

        const sf::Vector2f buttonSize{340.f, 60.f};
        const float buttonX = (kLogicalWidth - buttonSize.x) / 2.f;
        constexpr float kFirstButtonY = 330.f;
        constexpr float kButtonStep = 78.f;
        const auto buttonY = [](int index) {
            return kFirstButtonY + static_cast<float>(index) * kButtonStep;
        };

        // Toți jucătorii la calculatorul ăsta, pe rând.
        Button play(context.font, utf8("Joacă"), {buttonX, buttonY(0)}, buttonSize);
        play.setOnClick([context] {
            context.screens.change(std::make_unique<LobbyScreen>(context));
        });
        m_buttons.push_back(std::move(play));

        // În rețea: câte un jucător pe calculator; unul creează jocul, ceilalți intră.
        Button host(context.font, utf8("Creează joc în rețea"), {buttonX, buttonY(1)},
                    buttonSize);
        host.setOnClick([context] {
            context.screens.change(std::make_unique<HostLobbyScreen>(context));
        });
        m_buttons.push_back(std::move(host));

        Button join(context.font, utf8("Intră într-un joc"), {buttonX, buttonY(2)}, buttonSize);
        join.setOnClick([context] {
            context.screens.change(std::make_unique<JoinLobbyScreen>(context));
        });
        m_buttons.push_back(std::move(join));

        Button quit(context.font, utf8("Ieșire"), {buttonX, buttonY(3)}, buttonSize);
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
