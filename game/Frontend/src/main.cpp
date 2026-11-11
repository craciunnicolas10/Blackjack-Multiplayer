#include <SFML/Graphics.hpp>

#include <Backend/dummy/dummy.h>

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Blackjack");

    sf::Font font;
    if (!font.openFromFile("assets/NotoSansMono-Regular.ttf")) {
        return 1;
    }

    auto dummy = std::to_string(Backend::dummyReturn42());
    sf::Text text(font, dummy, 36);
    text.setFillColor(sf::Color::White);
    text.setPosition({50.f, 50.f});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        window.clear(sf::Color(20, 100, 40));
        window.draw(text);
        window.display();
    }
}
