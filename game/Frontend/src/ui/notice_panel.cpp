#include <ui/notice_panel.h>

#include <utility>

#include <app/layout.h>
#include <ui/crisp_text.h>
#include <ui/text_layout.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr sf::Vector2f kSize{680.f, 230.f};
        constexpr sf::Vector2f kPosition{(kLogicalWidth - kSize.x) / 2.f,
                                         (kLogicalHeight - kSize.y) / 2.f};
        constexpr sf::Vector2f kButtonSize{200.f, 52.f};
    }  // namespace

    NoticePanel::NoticePanel(const sf::Font& font)
        : m_dim({kLogicalWidth, kLogicalHeight}),
          m_background(kSize),
          m_title(font, "", Theme::kTextHeading + 2),
          m_message(font, "", Theme::kTextBody),
          m_menuButton(font, utf8("Meniu"),
                       {kPosition.x + (kSize.x - kButtonSize.x) / 2.f,
                        kPosition.y + kSize.y - kButtonSize.y - 24.f},
                       kButtonSize) {
        m_dim.setFillColor(sf::Color(0, 0, 0, 150));
        m_background.setPosition(kPosition);
        m_background.setFillColor(Theme::kPanelFill);
        m_background.setOutlineColor(Theme::kGold);
        m_background.setOutlineThickness(3.f);
        m_title.setFillColor(Theme::kGold);
        m_message.setFillColor(Theme::kTextSecondary);
        m_menuButton.setOnClick([this] {
            if (m_onMenu) {
                m_onMenu();
            }
        });
    }

    void NoticePanel::open(const sf::String& title, const sf::String& message) {
        m_title.setString(title);
        centerTextAt(m_title, {kPosition.x + kSize.x / 2.f, kPosition.y + 48.f});
        m_message.setString(message);
        centerTextAt(m_message, {kPosition.x + kSize.x / 2.f, kPosition.y + 102.f});
    }

    void NoticePanel::setOnMenu(std::function<void()> onMenu) { m_onMenu = std::move(onMenu); }

    void NoticePanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        m_menuButton.handleEvent(event, window);
    }

    void NoticePanel::draw(sf::RenderTarget& target) const {
        target.draw(m_dim);
        target.draw(m_background);
        drawText(target, m_title);
        drawText(target, m_message);
        m_menuButton.draw(target);
    }
}  // namespace Frontend
