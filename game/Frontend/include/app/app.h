#pragma once
#include <memory>

#include <SFML/Graphics.hpp>

#include <screens/screen_manager.h>

namespace Frontend {
    class Application {
       public:
        int run();

       private:
        bool initializeSubsystems();

        bool initializeFonts();
        void initializeWindow();

        void mainLoop();

        sf::Font m_font;
        std::unique_ptr<sf::RenderWindow> m_window;
        // Declarat ultimul: ecranele țin referințe la font și fereastră, deci se distrug primele.
        ScreenManager m_screens;
    };
}  // namespace Frontend
