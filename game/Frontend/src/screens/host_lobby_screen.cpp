#include <screens/host_lobby_screen.h>

#include <string>
#include <utility>

#include <app/layout.h>
#include <game/network_session.h>
#include <screens/lobby_screen.h>
#include <screens/menu_screen.h>
#include <screens/screen_manager.h>
#include <screens/table_screen.h>
#include <ui/crisp_text.h>
#include <ui/player_rows.h>
#include <ui/text_layout.h>
#include <ui/text_utils.h>
#include <ui/theme.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr sf::Vector2f kInputSize{360.f, 56.f};
        constexpr sf::Vector2f kCreateSize{160.f, 56.f};
        constexpr float kRowGap = 20.f;
        constexpr float kRowWidth = kInputSize.x + kRowGap + kCreateSize.x;
        constexpr float kInputX = (kLogicalWidth - kRowWidth) / 2.f;
        constexpr float kInputY = 180.f;
        constexpr float kListY = 290.f;

        constexpr sf::Vector2f kBottomButtonSize{200.f, 64.f};
        constexpr float kBottomY = kLogicalHeight - kBottomButtonSize.y - 40.f;
    }  // namespace

    HostLobbyScreen::HostLobbyScreen(AppContext context)
        : m_context(context),
          m_title(context.font, utf8("Joc în rețea: gazdă"), Theme::kTextTitle),
          m_hint(context.font, utf8("Scrie-ți numele și creează jocul"), Theme::kTextBody),
          m_address(context.font, "", Theme::kTextLarge),
          m_error(context.font, "", Theme::kTextSmall + 2),
          m_nameInput(context.font, {kInputX, kInputY}, kInputSize,
                      LobbyScreen::kMaxNameLength),
          m_createButton(context.font, utf8("Creează"),
                         {kInputX + kInputSize.x + kRowGap, kInputY}, kCreateSize),
          m_backButton(context.font, utf8("Înapoi"),
                       {kLogicalWidth / 2.f - kBottomButtonSize.x - 20.f, kBottomY},
                       kBottomButtonSize),
          m_startButton(context.font, utf8("Start"), {kLogicalWidth / 2.f + 20.f, kBottomY},
                        kBottomButtonSize),
          m_events(std::make_shared<NetworkEvents>()) {
        m_title.setFillColor(Theme::kGold);
        centerHorizontally(m_title, 50.f);
        m_hint.setFillColor(Theme::kTextSecondary);
        centerHorizontally(m_hint, 125.f);
        m_address.setFillColor(Theme::kGold);
        m_error.setFillColor(Theme::kNegative);

        m_nameInput.setPlaceholder(utf8("Numele tău"));
        m_nameInput.setFocused(true);
        m_nameInput.setOnSubmit([this] { m_createRequested = true; });

        m_createButton.setOnClick([this] { m_createRequested = true; });
        m_backButton.setOnClick([this] { m_backRequested = true; });
        m_startButton.setOnClick([this] { m_startRequested = true; });

        // Anunțurile vin din m_host->Update() sau StartGame(); aici doar notăm ce s-a
        // schimbat, nu schimbăm ecranul în mijlocul lor.
        m_events->onPlayersChanged = [this](const std::vector<std::string>& names) {
            m_players = utf8All(names);
            refreshButtons();
        };
        m_events->onGameStarted = [this] { m_gameStarted = true; };

        refreshButtons();
    }

    void HostLobbyScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                m_backRequested = true;
            }
        }

        if (!m_host) {
            m_nameInput.handleEvent(event, m_context.window);
            m_createButton.handleEvent(event, m_context.window);
        }
        m_backButton.handleEvent(event, m_context.window);
        m_startButton.handleEvent(event, m_context.window);

        applyPendingActions();
    }

    void HostLobbyScreen::update(float dt) {
        m_nameInput.update(dt);
        if (m_host) {
            m_host->Update();
        }
        if (m_gameStarted) {
            openTable();
        }
    }

    void HostLobbyScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);
        drawText(target, m_title);
        drawText(target, m_hint);
        if (m_host) {
            drawText(target, m_address);
            drawPlayerRows(target, m_context.font, m_players, {kInputX, kListY}, kRowWidth);
        } else {
            m_nameInput.draw(target);
            m_createButton.draw(target);
        }
        drawText(target, m_error);
        m_backButton.draw(target);
        m_startButton.draw(target);
    }

    void HostLobbyScreen::applyPendingActions() {
        if (m_createRequested) {
            m_createRequested = false;
            createGame();
        }
        if (m_backRequested) {
            m_backRequested = false;
            // Închiderea gazdei le spune celor conectați că jocul s-a închis.
            m_context.screens.change(std::make_unique<MenuScreen>(m_context));
        } else if (m_startRequested) {
            m_startRequested = false;
            if (m_host && m_host->StartGame() && m_gameStarted) {
                openTable();
            }
        }
    }

    void HostLobbyScreen::createGame() {
        if (m_host) {
            return;
        }
        const sf::String name = trimmed(m_nameInput.getText());
        if (name.isEmpty()) {
            showError(utf8("Scrie-ți numele înainte să creezi jocul"));
            return;
        }
        m_host = Backend::INetworkHost::Produce(toUtf8String(name));
        if (!m_host) {
            showError(utf8("Nu am găsit niciun port liber (mai sunt jocuri deschise?)"));
            return;
        }
        m_host->AddListener(m_events);
        m_players = utf8All(m_host->GetPlayerNames());

        m_hint.setString(utf8("Ceilalți apasă „Intră într-un joc” și scriu adresa:"));
        centerHorizontally(m_hint, 125.f);
        const std::string address = m_host->GetAddress();
        m_address.setString(address.empty()
                                ? utf8("adresa acestui calculator nu a putut fi aflată")
                                : utf8(address));
        centerHorizontally(m_address, 175.f);
        showError("");
        refreshButtons();
    }

    void HostLobbyScreen::openTable() {
        m_gameStarted = false;
        m_host->RemoveListener(m_events.get());
        const std::vector<sf::String> names = utf8All(m_host->GetPlayerNames());
        m_context.screens.change(std::make_unique<TableScreen>(
            m_context, names, std::make_unique<NetworkSession>(std::move(m_host))));
    }

    void HostLobbyScreen::showError(const sf::String& message) {
        m_error.setString(message);
        centerHorizontally(m_error, kInputY + kInputSize.y + 18.f);
    }

    void HostLobbyScreen::refreshButtons() {
        m_startButton.setEnabled(m_host && m_host->CanStartGame());
    }
}  // namespace Frontend
