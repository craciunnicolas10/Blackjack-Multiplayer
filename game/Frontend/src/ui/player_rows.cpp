#include <ui/player_rows.h>

#include <string>

#include <ui/crisp_text.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    void drawPlayerRows(sf::RenderTarget& target, const sf::Font& font,
                        const std::vector<sf::String>& names, sf::Vector2f topLeft, float width) {
        for (std::size_t i = 0; i < names.size(); ++i) {
            const float rowY = topLeft.y + static_cast<float>(i) * kPlayerRowHeight;

            sf::RectangleShape row({width, kPlayerRowHeight - 8.f});
            row.setPosition({topLeft.x, rowY});
            row.setFillColor(Theme::kPanelFill);
            target.draw(row);

            sf::Text name(font, utf8(std::to_string(i + 1) + ". ") + names[i],
                          Theme::kTextHeading);
            name.setPosition({topLeft.x + 16.f, rowY + 10.f});
            name.setFillColor(Theme::kTextPrimary);
            drawText(target, name);
        }
    }
}  // namespace Frontend
