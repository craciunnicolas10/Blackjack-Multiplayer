#include <app/app.h>

#include <optional>
#include <string>

#include <Backend/dummy/dummy.h>

namespace Frontend {
    int Application::run() {
        initializeSubsystems();

        mainLoop();

        return 0;
    }

    void Application::initializeSubsystems() {
        initializeWindow();
        initializeFonts();
    }

    void Application::initializeFonts() {
        if (!m_font.openFromFile("assets/fonts/NotoSansMono-Regular.ttf")) {
        }
    }

    void Application::initializeWindow() {
        m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({1280, 720}),
                                                      "Blackjack | Echipa Dealerii");
    }

    void Application::mainLoop() {
        auto text = std::to_string(Backend::dummyReturn42());
        sf::Text dummyText(m_font, text, 30);
        dummyText.setFillColor(sf::Color::White);
        dummyText.setPosition({50, 50});

        while (m_window->isOpen()) {
            while (const auto event = m_window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    m_window->close();
                } else if (const auto resizeEvent = event->getIf<sf::Event::Resized>()) {
                    const auto [width, height] = resizeEvent->size;
                    sf::View view(sf::FloatRect(
                        {0.f, 0.f}, {static_cast<float>(width), static_cast<float>(height)}));
                    m_window->setView(view);
                }
            }

            m_window->clear(sf::Color::Blue);
            m_window->draw(dummyText);
            m_window->display();
        }
    }
}  // namespace Frontend
