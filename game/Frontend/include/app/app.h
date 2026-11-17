#pragma once
#include <memory>

#include <SFML/Graphics.hpp>

#include <audio/sound_player.h>
#include <screens/screen_manager.h>
#include <ui/card_textures.h>
#include <ui/mute_icon.h>

namespace Frontend {
    class Application {
       public:
        int run();

       private:
        bool initializeSubsystems();

        bool initializeFonts();
        bool initializeCardTextures();
        void initializeSounds();
        void initializeWindow();
        // Iconița ferestrei: asul de pică. Are nevoie de texturile cărților.
        void initializeWindowIcon();

        // F11: comută între fereastră și ecran complet.
        void toggleFullscreen();
        // După ce fereastra e (re)creată: limită de FPS, view-ul logic, iconița.
        void configureWindow();

        void mainLoop();
        // Ecranul curent primește text (ex. un nume): tastele cu litere nu sunt scurtături.
        bool isTypingText() const;

        sf::Font m_font;
        CardTextures m_cardTextures;
        SoundPlayer m_sounds;
        MuteIcon m_muteIcon;
        sf::Image m_icon;
        bool m_fullscreen = false;
        std::unique_ptr<sf::RenderWindow> m_window;
        // Declarat ultimul: ecranele țin referințe la font și fereastră, deci se distrug primele.
        ScreenManager m_screens;
    };
}  // namespace Frontend
