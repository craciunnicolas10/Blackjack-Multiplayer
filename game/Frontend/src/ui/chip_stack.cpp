#include <ui/chip_stack.h>

#include <algorithm>
#include <array>
#include <cstdint>

#include <audio/sound_player.h>

namespace Frontend {
    namespace {
        constexpr std::array<int, 3> kDenominations{100, 50, 10};

        float easeInOutCubic(float t) {
            if (t < 0.5f) {
                return 4.f * t * t * t;
            }
            const float inverse = -2.f * t + 2.f;
            return 1.f - inverse * inverse * inverse / 2.f;
        }
    }  // namespace

    ChipStack::ChipStack(const sf::Font& font) : m_font(&font) {}

    void ChipStack::setPosition(sf::Vector2f bottomCenter) {
        m_position = bottomCenter;
        if (!m_flying) {
            placeChips();
        }
    }

    sf::Vector2f ChipStack::getPosition() const { return m_position; }

    void ChipStack::setAmount(int amount) {
        m_amount = std::max(amount, 0);
        m_flying = false;
        rebuild();
    }

    int ChipStack::getAmount() const { return m_amount; }

    void ChipStack::sendTo(sf::Vector2f target) {
        if (m_chips.empty()) {
            return;
        }
        m_flying = true;
        m_flightTime = 0.f;
        m_flightTarget = target;
    }

    bool ChipStack::isAnimating() const { return m_flying; }

    void ChipStack::update(float dt) {
        if (!m_flying) {
            return;
        }
        m_flightTime += dt;

        // Pleacă întâi jetonul de sus, apoi cele de dedesubt.
        const std::size_t count = m_chips.size();
        for (std::size_t i = 0; i < count; ++i) {
            const float delay = kFlightStagger * static_cast<float>(count - 1 - i);
            const float t = std::clamp((m_flightTime - delay) / kFlightDuration, 0.f, 1.f);
            const sf::Vector2f start = restingPosition(i);
            m_chips[i].setPosition(start + (m_flightTarget - start) * easeInOutCubic(t));
            // Se estompează pe ultima parte a drumului.
            const float fade = std::clamp((1.f - t) / 0.3f, 0.f, 1.f);
            m_chips[i].setAlpha(static_cast<std::uint8_t>(255.f * fade));
        }

        const float total = kFlightDuration + kFlightStagger * static_cast<float>(count - 1);
        if (m_flightTime >= total) {
            m_flying = false;
            m_amount = 0;
            m_chips.clear();
            playSound(SoundId::Chip);
        }
    }

    void ChipStack::draw(sf::RenderTarget& target) const {
        for (const ChipView& chip : m_chips) {
            chip.draw(target);
        }
    }

    sf::Vector2f ChipStack::restingPosition(std::size_t index) const {
        return {m_position.x, m_position.y - kLayerOffset * static_cast<float>(index)};
    }

    void ChipStack::rebuild() {
        m_chips.clear();
        int remaining = m_amount;
        for (const int value : kDenominations) {
            while (remaining >= value && static_cast<int>(m_chips.size()) < kMaxChips) {
                m_chips.emplace_back(*m_font, value, kChipRadius);
                remaining -= value;
            }
        }
        // Rest care nu se împarte la 10 (ex. o plată 3:2): un jeton cu exact restul.
        if (remaining > 0 && remaining < kDenominations.back() &&
            static_cast<int>(m_chips.size()) < kMaxChips) {
            m_chips.emplace_back(*m_font, remaining, kChipRadius);
        }
        placeChips();
    }

    void ChipStack::placeChips() {
        for (std::size_t i = 0; i < m_chips.size(); ++i) {
            m_chips[i].setPosition(restingPosition(i));
            m_chips[i].setAlpha(255);
        }
    }
}  // namespace Frontend
