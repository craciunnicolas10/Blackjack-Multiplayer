#include <ui/crisp_text.h>

#include <algorithm>
#include <cmath>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>

namespace Frontend {
    void drawText(sf::RenderTarget& target, const sf::Text& text, sf::RenderStates states) {
        // Câți pixeli reali are o unitate logică (1 în fereastra normală, 1,5 la 1080p etc.).
        const sf::View& view = target.getView();
        const sf::IntRect viewport = target.getViewport(view);
        const float pixelsPerUnit = static_cast<float>(viewport.size.y) / view.getSize().y;

        const unsigned size = text.getCharacterSize();
        if (size == 0 || pixelsPerUnit <= 0.f || std::abs(pixelsPerUnit - 1.f) < 0.01f) {
            target.draw(text, states);
            return;
        }

        // Literele, generate la mărimea reală; `factor` = de câte ori sunt mai mari.
        const auto crispSize = static_cast<unsigned>(
            std::max(1L, std::lround(static_cast<float>(size) * pixelsPerUnit)));
        const float factor = static_cast<float>(crispSize) / static_cast<float>(size);

        sf::Text crisp = text;
        crisp.setCharacterSize(crispSize);
        // Originea e în coordonatele locale ale textului, care cresc odată cu literele.
        crisp.setOrigin(text.getOrigin() * factor);
        crisp.setScale(text.getScale() / factor);
        crisp.setOutlineThickness(text.getOutlineThickness() * factor);

        // Textul drept se aliniază la un pixel întreg, altfel literele cad între pixeli și
        // se estompează din nou. Textul rotit (ex. cel de pe arcul mesei) rămâne cum e.
        if (text.getRotation() == sf::Angle::Zero) {
            const sf::Vector2f anchor =
                (states.transform * crisp.getTransform()).transformPoint({0.f, 0.f});
            const sf::Vector2i pixel = target.mapCoordsToPixel(anchor, view);
            const sf::Vector2f snapped = target.mapPixelToCoords(pixel, view);
            crisp.move(snapped - anchor);
        }

        target.draw(crisp, states);
    }
}  // namespace Frontend
