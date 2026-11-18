#include <ui/round_summary_panel.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include <app/layout.h>
#include <ui/crisp_text.h>
#include <ui/text_layout.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr float kPadding = 24.f;
        constexpr float kRowsTop = 110.f;  // față de marginea de sus a panoului
        constexpr float kButtonsGap = 20.f;
        constexpr sf::Vector2f kNextSize{280.f, 52.f};
        constexpr sf::Vector2f kMenuSize{160.f, 52.f};

        // Coloanele: nume, rezultat, câștig, bani (x față de marginea din stânga a panoului).
        constexpr float kColumns[] = {32.f, 250.f, 420.f, 510.f};
        constexpr unsigned kCellSize = Theme::kTextBody;

        sf::Vector2f panelSize(std::size_t rowCount) {
            const float rows = static_cast<float>(rowCount) * RoundSummaryPanel::kRowHeight;
            return {RoundSummaryPanel::kWidth,
                    kRowsTop + rows + kButtonsGap + kNextSize.y + kPadding};
        }

        sf::Vector2f panelPosition(std::size_t rowCount) {
            const sf::Vector2f size = panelSize(rowCount);
            return {(kLogicalWidth - size.x) / 2.f, (kLogicalHeight - size.y) / 2.f};
        }

        float buttonsY(std::size_t rowCount) {
            return panelPosition(rowCount).y + kRowsTop +
                   static_cast<float>(rowCount) * RoundSummaryPanel::kRowHeight + kButtonsGap;
        }

        float buttonsLeft() {
            const float width = kNextSize.x + kButtonsGap + kMenuSize.x;
            return (kLogicalWidth - width) / 2.f;
        }

        // Culorile rezultatelor sunt gândite pentru fundalul etichetelor; pe panoul închis
        // le deschidem puțin, ca textul să se citească.
        sf::Color lighter(sf::Color color) {
            const auto mix = [](std::uint8_t channel) {
                return static_cast<std::uint8_t>(channel + (255 - channel) * 35 / 100);
            };
            return {mix(color.r), mix(color.g), mix(color.b)};
        }

    }  // namespace

    RoundSummaryPanel::RoundSummaryPanel(const sf::Font& font, std::size_t rowCount)
        : m_position(panelPosition(rowCount)),
          m_size(panelSize(rowCount)),
          m_rowCount(rowCount),
          m_font(&font),
          m_dim({kLogicalWidth, kLogicalHeight}),
          m_background(m_size),
          m_title(font, utf8("Rezultatele rundei"), Theme::kTextHeading + 2),
          m_dealerLine(font, "", Theme::kTextSmall + 2),
          m_nextButton(font, utf8("Runda următoare"), {buttonsLeft(), buttonsY(rowCount)},
                       kNextSize),
          m_menuButton(font, utf8("Meniu"),
                       {buttonsLeft() + kNextSize.x + kButtonsGap, buttonsY(rowCount)},
                       kMenuSize) {
        m_dim.setFillColor(sf::Color(0, 0, 0, 130));

        m_background.setPosition(m_position);
        m_background.setFillColor(Theme::kPanelFill);
        m_background.setOutlineColor(Theme::kGold);
        m_background.setOutlineThickness(3.f);

        m_title.setFillColor(Theme::kGold);
        setTitle(m_title.getString());
        m_dealerLine.setFillColor(Theme::kTextSecondary);

        m_nextButton.setOnClick([this] {
            if (m_onNextRound) {
                m_onNextRound();
            }
        });
        m_menuButton.setOnClick([this] {
            if (m_onMenu) {
                m_onMenu();
            }
        });
    }

    void RoundSummaryPanel::open(const std::vector<RoundSummaryRow>& rows,
                                 std::optional<int> dealerScore) {
        m_gameOver = false;
        m_nextButton.setEnabled(true);
        setTitle(utf8("Rezultatele rundei"));

        if (dealerScore) {
            std::string line = "Dealer: " + std::to_string(*dealerScore);
            if (*dealerScore > 21) {
                line += ", peste 21";
            }
            m_dealerLine.setString(utf8(line));
        } else {
            m_dealerLine.setString("");
        }
        centerTextAt(m_dealerLine, {m_position.x + m_size.x / 2.f, m_position.y + 78.f});

        m_cells.clear();
        const std::size_t count = std::min(rows.size(), m_rowCount);
        for (std::size_t i = 0; i < count; ++i) {
            const RoundSummaryRow& row = rows[i];
            const float centerY =
                m_position.y + kRowsTop + (static_cast<float>(i) + 0.5f) * kRowHeight;

            const std::string net = row.net > 0 ? "+" + std::to_string(row.net)
                                                : std::to_string(row.net);
            const sf::Color netColor = row.net > 0   ? Theme::kPositive
                                       : row.net < 0 ? Theme::kNegative
                                                     : Theme::kTextMuted;

            const sf::String texts[] = {row.name, utf8(roundResultLabel(row.result)), utf8(net),
                                        utf8("Bani: " + std::to_string(row.money))};
            const sf::Color colors[] = {sf::Color::White, lighter(roundResultColor(row.result)),
                                        netColor, Theme::kTextSecondary};

            for (std::size_t column = 0; column < 4; ++column) {
                sf::Text cell(*m_font, texts[column], kCellSize);
                const sf::FloatRect bounds = cell.getLocalBounds();
                cell.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f});
                cell.setPosition({m_position.x + kColumns[column], centerY});
                cell.setFillColor(colors[column]);
                m_cells.push_back(std::move(cell));
            }
        }
    }

    void RoundSummaryPanel::showGameOver() {
        if (m_gameOver) {
            return;
        }
        m_gameOver = true;
        m_nextButton.setEnabled(false);
        setTitle(utf8("Jocul s-a terminat"));

        // Motivul, pe rândul cu scorul dealerului.
        const sf::String reason = utf8("nimeni nu mai are bani de pariu");
        const sf::String dealer = m_dealerLine.getString();
        m_dealerLine.setString(dealer.isEmpty() ? reason : dealer + utf8("  ·  ") + reason);
        centerTextAt(m_dealerLine, {m_position.x + m_size.x / 2.f, m_position.y + 78.f});
    }

    void RoundSummaryPanel::setTitle(const sf::String& title) {
        m_title.setString(title);
        centerTextAt(m_title, {m_position.x + m_size.x / 2.f, m_position.y + kPadding + 16.f});
    }

    void RoundSummaryPanel::setOnNextRound(std::function<void()> onNextRound) {
        m_onNextRound = std::move(onNextRound);
    }

    void RoundSummaryPanel::setOnMenu(std::function<void()> onMenu) {
        m_onMenu = std::move(onMenu);
    }

    void RoundSummaryPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Enter && !m_gameOver && m_onNextRound) {
                m_onNextRound();
            }
        }
        m_nextButton.handleEvent(event, window);
        m_menuButton.handleEvent(event, window);
    }

    void RoundSummaryPanel::draw(sf::RenderTarget& target) const {
        target.draw(m_dim);
        target.draw(m_background);
        drawText(target, m_title);
        drawText(target, m_dealerLine);

        // Rânduri cu fundal alternativ, ca să se urmărească ușor pe orizontală.
        for (std::size_t i = 0; i < m_cells.size() / 4; ++i) {
            if (i % 2 == 0) {
                sf::RectangleShape stripe({m_size.x - 2.f * 12.f, kRowHeight});
                stripe.setPosition({m_position.x + 12.f,
                                    m_position.y + kRowsTop + static_cast<float>(i) * kRowHeight});
                stripe.setFillColor(sf::Color(255, 255, 255, 14));
                target.draw(stripe);
            }
        }
        for (const sf::Text& cell : m_cells) {
            drawText(target, cell);
        }

        m_nextButton.draw(target);
        m_menuButton.draw(target);
    }
}  // namespace Frontend
