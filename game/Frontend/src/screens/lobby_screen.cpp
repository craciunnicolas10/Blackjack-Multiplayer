#include <screens/lobby_screen.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <app/layout.h>
#include <screens/menu_screen.h>
#include <screens/screen_manager.h>
#include <screens/table_screen.h>
#include <ui/crisp_text.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        // Rândul de adăugare: câmpul de text + butonul „Adaugă”, centrate.
        constexpr sf::Vector2f kInputSize{360.f, 56.f};
        constexpr sf::Vector2f kAddSize{140.f, 56.f};
        constexpr float kRowGap = 20.f;
        constexpr float kInputX = (kLogicalWidth - (kInputSize.x + kRowGap + kAddSize.x)) / 2.f;
        constexpr float kInputY = 180.f;

        // Lista de jucători.
        constexpr float kListX = kInputX;
        constexpr float kListWidth = kInputSize.x + kRowGap + kAddSize.x;
        constexpr float kListY = 290.f;
        constexpr float kListRowHeight = 64.f;
        constexpr sf::Vector2f kRemoveSize{48.f, 48.f};

        constexpr sf::Vector2f kBottomButtonSize{200.f, 64.f};
        constexpr float kBottomY = kLogicalHeight - kBottomButtonSize.y - 40.f;

        void centerHorizontally(sf::Text& text, float y) {
            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
            text.setPosition({kLogicalWidth / 2.f, y});
        }

        sf::String trimmed(const sf::String& text) {
            std::size_t first = 0;
            std::size_t last = text.getSize();
            while (first < last && text[first] == U' ') {
                ++first;
            }
            while (last > first && text[last - 1] == U' ') {
                --last;
            }
            return text.substring(first, last - first);
        }
    }  // namespace

    LobbyScreen::LobbyScreen(AppContext context)
        : m_context(context),
          m_title(context.font, utf8("Jucători"), Theme::kTextTitle),
          m_hint(context.font,
                 utf8("Adaugă între 1 și " + std::to_string(kMaxPlayers) + " jucători"),
                 Theme::kTextBody),
          m_error(context.font, "", Theme::kTextSmall + 2),
          m_nameInput(context.font, {kInputX, kInputY}, kInputSize, kMaxNameLength),
          m_addButton(context.font, utf8("Adaugă"), {kInputX + kInputSize.x + kRowGap, kInputY},
                      kAddSize),
          m_backButton(context.font, utf8("Înapoi"),
                       {kLogicalWidth / 2.f - kBottomButtonSize.x - 20.f, kBottomY},
                       kBottomButtonSize),
          m_startButton(context.font, utf8("Start"), {kLogicalWidth / 2.f + 20.f, kBottomY},
                        kBottomButtonSize) {
        m_title.setFillColor(Theme::kGold);
        centerHorizontally(m_title, 50.f);

        m_hint.setFillColor(Theme::kTextSecondary);
        centerHorizontally(m_hint, 125.f);

        m_error.setFillColor(Theme::kNegative);

        m_nameInput.setPlaceholder(utf8("Numele jucătorului"));
        m_nameInput.setFocused(true);
        m_nameInput.setOnSubmit([this] { m_addRequested = true; });

        m_addButton.setOnClick([this] { m_addRequested = true; });
        m_backButton.setOnClick([this] { m_backRequested = true; });
        m_startButton.setOnClick([this] { m_startRequested = true; });

        rebuildPlayerRows();
    }

    void LobbyScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                m_backRequested = true;
            }
        }

        m_nameInput.handleEvent(event, m_context.window);
        m_addButton.handleEvent(event, m_context.window);
        m_backButton.handleEvent(event, m_context.window);
        m_startButton.handleEvent(event, m_context.window);
        for (Button& button : m_removeButtons) {
            button.handleEvent(event, m_context.window);
        }

        applyPendingActions();
    }

    void LobbyScreen::update(float dt) { m_nameInput.update(dt); }

    void LobbyScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);

        drawText(target, m_title);
        drawText(target, m_hint);
        m_nameInput.draw(target);
        m_addButton.draw(target);
        drawText(target, m_error);

        for (std::size_t i = 0; i < m_players.size(); ++i) {
            const float rowY = kListY + static_cast<float>(i) * kListRowHeight;

            sf::RectangleShape row({kListWidth, kListRowHeight - 8.f});
            row.setPosition({kListX, rowY});
            row.setFillColor(Theme::kPanelFill);
            target.draw(row);

            sf::Text name(m_context.font, utf8(std::to_string(i + 1) + ". "), Theme::kTextHeading);
            name.setString(name.getString() + m_players.at(i));
            name.setPosition({kListX + 16.f, rowY + 10.f});
            name.setFillColor(Theme::kTextPrimary);
            drawText(target, name);
        }
        for (const Button& button : m_removeButtons) {
            button.draw(target);
        }

        m_backButton.draw(target);
        m_startButton.draw(target);
    }

    void LobbyScreen::applyPendingActions() {
        if (m_addRequested) {
            m_addRequested = false;
            tryAddPlayer();
        }

        if (m_removeRequested) {
            const std::size_t index = *m_removeRequested;
            m_removeRequested.reset();
            removePlayer(index);
        }

        if (m_backRequested) {
            m_backRequested = false;
            m_context.screens.change(std::make_unique<MenuScreen>(m_context));
        } else if (m_startRequested) {
            m_startRequested = false;
            if (!m_players.empty()) {
                m_context.screens.change(std::make_unique<TableScreen>(m_context, m_players));
            }
        }
    }

    void LobbyScreen::tryAddPlayer() {
        if (m_players.size() >= kMaxPlayers) {
            showError(utf8("Masa e plină: maxim " + std::to_string(kMaxPlayers) + " jucători"));
            return;
        }

        const sf::String name = trimmed(m_nameInput.getText());
        if (name.isEmpty()) {
            showError(utf8("Scrie un nume înainte să adaugi jucătorul"));
            return;
        }
        if (std::find(m_players.begin(), m_players.end(), name) != m_players.end()) {
            showError(utf8("Există deja un jucător cu numele ăsta"));
            return;
        }

        m_players.push_back(name);
        m_nameInput.clear();
        m_nameInput.setFocused(true);  // click-ul pe „Adaugă” i-a luat focusul
        showError("");
        rebuildPlayerRows();
    }

    void LobbyScreen::removePlayer(std::size_t index) {
        if (index >= m_players.size()) {
            return;
        }
        m_players.erase(m_players.begin() + static_cast<std::ptrdiff_t>(index));
        showError("");
        rebuildPlayerRows();
    }

    void LobbyScreen::rebuildPlayerRows() {
        m_removeButtons.clear();
        for (std::size_t i = 0; i < m_players.size(); ++i) {
            const float rowY = kListY + static_cast<float>(i) * kListRowHeight;
            const sf::Vector2f position{kListX + kListWidth - kRemoveSize.x - 4.f,
                                        rowY + (kListRowHeight - 8.f - kRemoveSize.y) / 2.f};

            Button remove(m_context.font, "X", position, kRemoveSize);
            remove.setOnClick([this, i] { m_removeRequested = i; });
            m_removeButtons.push_back(std::move(remove));
        }

        const bool full = m_players.size() >= kMaxPlayers;
        m_addButton.setEnabled(!full);
        m_startButton.setEnabled(!m_players.empty());
    }

    void LobbyScreen::showError(const sf::String& message) {
        m_error.setString(message);
        centerHorizontally(m_error, kInputY + kInputSize.y + 18.f);
    }
}  // namespace Frontend
