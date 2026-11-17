#include <ui/betting_panel.h>

#include <algorithm>
#include <string>
#include <utility>

#include <audio/sound_player.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr float kChipRadius = 32.f;
        constexpr float kChipGap = 24.f;   // între jetoane
        constexpr float kClearGap = 30.f;  // între ultimul jeton și „Șterge”
        constexpr sf::Vector2f kClearSize{130.f, 50.f};
        constexpr sf::Vector2f kConfirmSize{220.f, 52.f};
        constexpr float kChipsCenterY = 176.f;  // față de marginea de sus a panoului
        constexpr float kConfirmY = 218.f;

        const sf::Color kGold(240, 210, 120);

        void centerTextAt(sf::Text& text, sf::Vector2f center) {
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin(bounds.position + bounds.size / 2.f);
            text.setPosition(center);
        }

        // Rândul cu jetoane + „Șterge”, centrat în panou.
        float buttonsRowLeft(sf::Vector2f panelPosition) {
            const float chipCount = static_cast<float>(BettingPanel::kChipValues.size());
            const float width = chipCount * 2.f * kChipRadius + (chipCount - 1.f) * kChipGap +
                                kClearGap + kClearSize.x;
            return panelPosition.x + (BettingPanel::kSize.x - width) / 2.f;
        }

        // Centrul jetonului `index`.
        sf::Vector2f chipCenter(sf::Vector2f panelPosition, std::size_t index) {
            return {buttonsRowLeft(panelPosition) + kChipRadius +
                        static_cast<float>(index) * (2.f * kChipRadius + kChipGap),
                    panelPosition.y + kChipsCenterY};
        }

        sf::Vector2f clearPosition(sf::Vector2f panelPosition) {
            const float chipCount = static_cast<float>(BettingPanel::kChipValues.size());
            return {buttonsRowLeft(panelPosition) + chipCount * 2.f * kChipRadius +
                        (chipCount - 1.f) * kChipGap + kClearGap,
                    panelPosition.y + kChipsCenterY - kClearSize.y / 2.f};
        }
    }  // namespace

    BettingPanel::BettingPanel(const sf::Font& font, sf::Vector2f position)
        : m_position(position),
          m_background(kSize),
          m_title(font, "", 28),
          m_amount(font, "", 34),
          m_info(font, "", 20),
          m_clearButton(font, utf8("Șterge"), clearPosition(position), kClearSize),
          m_confirmButton(font, utf8("Confirmă"),
                          {position.x + (kSize.x - kConfirmSize.x) / 2.f, position.y + kConfirmY},
                          kConfirmSize) {
        m_background.setPosition(position);
        m_background.setFillColor(sf::Color(10, 40, 22, 235));
        m_background.setOutlineColor(kGold);
        m_background.setOutlineThickness(3.f);

        m_title.setFillColor(sf::Color::White);
        m_amount.setFillColor(kGold);
        m_info.setFillColor(sf::Color(190, 190, 190));

        for (std::size_t i = 0; i < kChipValues.size(); ++i) {
            const int value = kChipValues[i];
            ChipButton chip(font, value, chipCenter(position, i), kChipRadius);
            chip.setOnClick([this, value] { addChip(value); });
            m_chipButtons.push_back(std::move(chip));
        }
        m_clearButton.setOnClick([this] { clearBet(); });
        m_confirmButton.setOnClick([this] { confirm(); });

        refresh();
    }

    void BettingPanel::open(const sf::String& playerName, int money, int minBet, int maxBet,
                            int initialBet) {
        m_money = money;
        m_minBet = minBet;
        m_maxBet = maxBet;
        m_bet = std::clamp(initialBet, 0, effectiveMax());

        m_title.setString(playerName + utf8(", pune pariul"));
        refresh();
    }

    int BettingPanel::getBet() const { return m_bet; }

    void BettingPanel::setOnConfirm(std::function<void(int)> onConfirm) {
        m_onConfirm = std::move(onConfirm);
    }

    void BettingPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Enter) {
                confirm();
            } else if (key->code == sf::Keyboard::Key::Backspace) {
                clearBet();
            }
        }

        for (ChipButton& chip : m_chipButtons) {
            chip.handleEvent(event, window);
        }
        m_clearButton.handleEvent(event, window);
        m_confirmButton.handleEvent(event, window);
    }

    void BettingPanel::draw(sf::RenderTarget& target) const {
        target.draw(m_background);
        target.draw(m_title);
        target.draw(m_amount);
        target.draw(m_info);
        for (const ChipButton& chip : m_chipButtons) {
            chip.draw(target);
        }
        m_clearButton.draw(target);
        m_confirmButton.draw(target);
    }

    int BettingPanel::effectiveMax() const { return std::max(std::min(m_maxBet, m_money), 0); }

    bool BettingPanel::canConfirm() const {
        return m_bet >= m_minBet && m_bet > 0 && m_bet <= effectiveMax();
    }

    void BettingPanel::addChip(int value) {
        if (m_bet + value <= effectiveMax()) {
            m_bet += value;
            playSound(SoundId::Chip);
            refresh();
        }
    }

    void BettingPanel::clearBet() {
        m_bet = 0;
        refresh();
    }

    void BettingPanel::confirm() {
        if (canConfirm() && m_onConfirm) {
            m_onConfirm(m_bet);
        }
    }

    void BettingPanel::refresh() {
        const float centerX = m_position.x + kSize.x / 2.f;

        centerTextAt(m_title, {centerX, m_position.y + 32.f});

        m_amount.setString(utf8("Pariu: " + std::to_string(m_bet)));
        centerTextAt(m_amount, {centerX, m_position.y + 78.f});

        m_info.setString(utf8("Rămân: " + std::to_string(m_money - m_bet) + "  ·  minim " +
                              std::to_string(m_minBet) + ", maxim " +
                              std::to_string(effectiveMax())));
        centerTextAt(m_info, {centerX, m_position.y + 118.f});

        for (std::size_t i = 0; i < m_chipButtons.size(); ++i) {
            m_chipButtons[i].setEnabled(m_bet + kChipValues[i] <= effectiveMax());
        }
        m_clearButton.setEnabled(m_bet > 0);
        m_confirmButton.setEnabled(canConfirm());
    }
}  // namespace Frontend
