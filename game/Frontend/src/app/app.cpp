#include <app/app.h>

#include <iostream>
#include <memory>
#include <optional>

#include <app/app_context.h>
#include <app/layout.h>
#include <screens/menu_screen.h>

namespace Frontend {
    namespace {
        constexpr const char* kFontPath = "assets/fonts/NotoSansMono-Regular.ttf";
    }

    int Application::run() {
        if (!initializeSubsystems()) {
            return 1;
        }

        mainLoop();

        return 0;
    }

    bool Application::initializeSubsystems() {
        initializeWindow();
        return initializeFonts();
    }

    bool Application::initializeFonts() {
        if (!m_font.openFromFile(kFontPath)) {
            // Calea e relativă la folderul curent: porniți aplicația din out/install/<preset>/bin.
            std::cerr << "Nu am putut incarca fontul: " << kFontPath << '\n';
            return false;
        }
        return true;
    }

    void Application::initializeWindow() {
        const sf::Vector2u size{static_cast<unsigned>(kLogicalWidth),
                                static_cast<unsigned>(kLogicalHeight)};
        m_window =
            std::make_unique<sf::RenderWindow>(sf::VideoMode(size), "Blackjack | Echipa Dealerii");
        m_window->setFramerateLimit(60);
        m_window->setView(makeLetterboxView(m_window->getSize()));
    }

    void Application::mainLoop() {
        const AppContext context{*m_window, m_font, m_screens};
        m_screens.change(std::make_unique<MenuScreen>(context));
        m_screens.applyPending();

        sf::Clock clock;
        while (m_window->isOpen()) {
            const float dt = clock.restart().asSeconds();

            while (const std::optional event = m_window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    m_window->close();
                } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    m_window->setView(makeLetterboxView(resized->size));
                } else if (Screen* screen = m_screens.current()) {
                    screen->handleEvent(*event);
                }
            }

            if (m_screens.quitRequested()) {
                m_window->close();
                break;
            }
            m_screens.applyPending();

            Screen* screen = m_screens.current();
            if (screen) {
                screen->update(dt);
            }

            // Negru = benzile din jurul zonei de joc când fereastra are altă proporție.
            m_window->clear(sf::Color::Black);
            if (screen) {
                screen->draw(*m_window);
            }
            m_window->display();
        }
    }
}  // namespace Frontend
