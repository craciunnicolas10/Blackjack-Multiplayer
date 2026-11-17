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
        constexpr const char* kWindowTitle = "Blackjack | Echipa Dealerii";
        constexpr unsigned kIconSize = 64;
        // Din imaginea asului de pică (250x363) luăm pătratul din mijloc, cu pica mare.
        constexpr int kIconCrop = 190;
    }  // namespace

    int Application::run() {
        if (!initializeSubsystems()) {
            return 1;
        }

        mainLoop();

        return 0;
    }

    bool Application::initializeSubsystems() {
        initializeWindow();
        if (!initializeFonts() || !initializeCardTextures()) {
            return false;
        }
        initializeWindowIcon();
        initializeSounds();
        // Colțul din dreapta-jos e liber pe toate ecranele.
        m_muteIcon.setPosition(
            {kLogicalWidth - MuteIcon::kRadius - 14.f, kLogicalHeight - MuteIcon::kRadius - 14.f});
        return true;
    }

    // Erorile pentru fiecare fișier lipsă sunt scrise deja de CardTextures::load.
    bool Application::initializeCardTextures() { return m_cardTextures.load(); }

    // Fără sunete jocul merge mai departe; eroarea e scrisă de SoundPlayer::load.
    void Application::initializeSounds() { (void)m_sounds.load(); }

    void Application::initializeWindowIcon() {
        const sf::Texture& ace = m_cardTextures.get(kMaxRank, kSuitSpades);
        const sf::Vector2i aceSize(ace.getSize());

        sf::RenderTexture canvas;
        if (!canvas.resize({kIconSize, kIconSize})) {
            return;  // fără iconiță, fereastra o păstrează pe cea implicită
        }
        sf::Sprite sprite(ace,
                          sf::IntRect({(aceSize.x - kIconCrop) / 2, (aceSize.y - kIconCrop) / 2},
                                      {kIconCrop, kIconCrop}));
        sprite.setScale(
            {static_cast<float>(kIconSize) / kIconCrop, static_cast<float>(kIconSize) / kIconCrop});
        canvas.clear(sf::Color::White);
        canvas.draw(sprite);
        canvas.display();

        m_icon = canvas.getTexture().copyToImage();
        m_window->setIcon(m_icon);
    }

    void Application::toggleFullscreen() {
        m_fullscreen = !m_fullscreen;
        if (m_fullscreen) {
            m_window->create(sf::VideoMode::getDesktopMode(), kWindowTitle, sf::State::Fullscreen);
        } else {
            const sf::Vector2u size{static_cast<unsigned>(kLogicalWidth),
                                    static_cast<unsigned>(kLogicalHeight)};
            m_window->create(sf::VideoMode(size), kWindowTitle);
        }
        configureWindow();
    }

    void Application::configureWindow() {
        m_window->setFramerateLimit(60);
        m_window->setView(makeLetterboxView(m_window->getSize()));
        if (m_icon.getSize().x > 0) {
            m_window->setIcon(m_icon);
        }
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
        m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(size), kWindowTitle);
        configureWindow();
    }

    void Application::mainLoop() {
        const AppContext context{*m_window, m_font, m_screens, m_cardTextures};
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
                } else if (const auto* key = event->getIf<sf::Event::KeyPressed>();
                           key && key->code == sf::Keyboard::Key::F11) {
                    toggleFullscreen();
                } else if (key && key->code == sf::Keyboard::Key::M && !isTypingText()) {
                    // M pornește/oprește sunetul, dar nu cât timp se scrie un nume.
                    m_sounds.toggleMuted();
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
            if (m_sounds.isMuted()) {
                m_muteIcon.draw(*m_window);
            }
            m_window->display();
        }
    }

    bool Application::isTypingText() const {
        const Screen* screen = m_screens.current();
        return screen && screen->isTypingText();
    }
}  // namespace Frontend
