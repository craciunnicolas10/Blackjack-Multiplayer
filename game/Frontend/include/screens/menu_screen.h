#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include <app/app_context.h>
#include <screens/screen.h>
#include <ui/button.h>

namespace Frontend {
    class MenuScreen : public Screen {
       public:
        explicit MenuScreen(AppContext context);

        void handleEvent(const sf::Event& event) override;
        void draw(sf::RenderTarget& target) override;

       private:
        AppContext m_context;
        sf::Text m_title;
        sf::Text m_subtitle;
        std::vector<Button> m_buttons;
    };
}  // namespace Frontend
