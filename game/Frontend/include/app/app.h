#pragma once
#include <memory>

#include <SFML/Graphics.hpp>

namespace Frontend {
    class Application {
       public:
        int run();

       private:
        void initializeSubsystems();

        void initializeFonts();
        void initializeWindow();

        void mainLoop();

        sf::Font m_font;
        std::unique_ptr<sf::RenderWindow> m_window;
    };
}  // namespace Frontend
