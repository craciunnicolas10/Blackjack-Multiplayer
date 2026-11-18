#include <screens/join_lobby_screen.h>

#include <string>
#include <utility>

#include <Backend/NetworkEnums.h>
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
        constexpr sf::Vector2f kInputSize{420.f, 56.f};
        constexpr float kInputX = (kLogicalWidth - kInputSize.x) / 2.f;
        constexpr float kNameY = 180.f;
        constexpr float kAddressY = 256.f;
        constexpr sf::Vector2f kConnectSize{260.f, 56.f};
        constexpr float kConnectY = 340.f;
        constexpr float kErrorY = 420.f;
        constexpr float kListY = 200.f;
        constexpr std::size_t kMaxAddressLength = 40;

        constexpr sf::Vector2f kBottomButtonSize{200.f, 64.f};
        constexpr float kBottomY = kLogicalHeight - kBottomButtonSize.y - 40.f;

        sf::String connectError(Backend::EConnectResult result) {
            switch (result) {
                case Backend::EConnectResult::InvalidName:
                    return utf8("Numele nu e bun");
                case Backend::EConnectResult::InvalidAddress:
                    return utf8("Adresa gazdei nu e bună");
                case Backend::EConnectResult::Unreachable:
                case Backend::EConnectResult::Connected:
                    break;
            }
            return utf8("Nu m-am putut conecta. E bună adresa? A creat gazda jocul?");
        }

        sf::String rejectionMessage(Backend::EJoinRejection reason) {
            switch (reason) {
                case Backend::EJoinRejection::DifferentVersion:
                    return utf8("Gazda are altă versiune a jocului.");
                case Backend::EJoinRejection::TableFull:
                    return utf8("Masa e plină.");
                case Backend::EJoinRejection::InvalidName:
                    return utf8("Numele nu e bun.");
                case Backend::EJoinRejection::NameTaken:
                    return utf8("Există deja un jucător cu numele ăsta.");
            }
            return utf8("Gazda a refuzat intrarea.");
        }
    }  // namespace

    JoinLobbyScreen::JoinLobbyScreen(AppContext context)
        : m_context(context),
          m_title(context.font, utf8("Joc în rețea: intră într-un joc"), Theme::kTextTitle),
          m_hint(context.font, "", Theme::kTextBody),
          m_error(context.font, "", Theme::kTextSmall + 2),
          m_nameInput(context.font, {kInputX, kNameY}, kInputSize, LobbyScreen::kMaxNameLength),
          m_addressInput(context.font, {kInputX, kAddressY}, kInputSize, kMaxAddressLength),
          m_connectButton(context.font, utf8("Conectează-te"),
                          {(kLogicalWidth - kConnectSize.x) / 2.f, kConnectY}, kConnectSize),
          m_backButton(context.font, utf8("Înapoi"),
                       {(kLogicalWidth - kBottomButtonSize.x) / 2.f, kBottomY},
                       kBottomButtonSize),
          m_events(std::make_shared<NetworkEvents>()) {
        m_title.setFillColor(Theme::kGold);
        centerHorizontally(m_title, 50.f);
        m_hint.setFillColor(Theme::kTextSecondary);
        showHint(utf8("Scrie-ți numele și adresa afișată pe calculatorul gazdei"));
        m_error.setFillColor(Theme::kNegative);

        m_nameInput.setPlaceholder(utf8("Numele tău"));
        m_nameInput.setFocused(true);
        m_nameInput.setOnSubmit([this] {
            m_nameInput.setFocused(false);
            m_addressInput.setFocused(true);
        });
        m_addressInput.setPlaceholder(utf8("Adresa gazdei"));
        m_addressInput.setOnSubmit([this] { m_connectRequested = true; });

        m_connectButton.setOnClick([this] { m_connectRequested = true; });
        m_backButton.setOnClick([this] { m_backRequested = true; });

        // Anunțurile vin din m_client->Update(); aici doar notăm ce s-a întâmplat. Legătura o
        // închidem după ce Update() s-a terminat (vezi update()).
        m_events->onPlayersChanged = [this](const std::vector<std::string>& names) {
            m_players = utf8All(names);
            showHint(utf8("Ai intrat. Se așteaptă ca gazda să pornească jocul."));
        };
        m_events->onJoinRejected = [this](Backend::EJoinRejection reason) {
            m_disconnectError = rejectionMessage(reason);
        };
        m_events->onHostLeft = [this] { m_disconnectError = utf8("Gazda a închis jocul"); };
        m_events->onGameStarted = [this] { m_gameStarted = true; };
    }

    bool JoinLobbyScreen::isTypingText() const {
        return !m_client && (m_nameInput.isFocused() || m_addressInput.isFocused());
    }

    void JoinLobbyScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                m_backRequested = true;
            } else if (key->code == sf::Keyboard::Key::Tab && !m_client) {
                const bool nameFocused = m_nameInput.isFocused();
                m_nameInput.setFocused(!nameFocused);
                m_addressInput.setFocused(nameFocused);
            }
        }

        if (!m_client) {
            m_nameInput.handleEvent(event, m_context.window);
            m_addressInput.handleEvent(event, m_context.window);
            m_connectButton.handleEvent(event, m_context.window);
        }
        m_backButton.handleEvent(event, m_context.window);

        applyPendingActions();
    }

    void JoinLobbyScreen::update(float dt) {
        m_nameInput.update(dt);
        m_addressInput.update(dt);
        if (!m_client) {
            return;
        }
        m_client->Update();
        if (m_disconnectError) {
            disconnect(*m_disconnectError);
        } else if (m_gameStarted) {
            openTable();
        }
    }

    void JoinLobbyScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);
        drawText(target, m_title);
        drawText(target, m_hint);
        if (m_client) {
            drawPlayerRows(target, m_context.font, m_players, {kInputX, kListY}, kInputSize.x);
        } else {
            m_nameInput.draw(target);
            m_addressInput.draw(target);
            m_connectButton.draw(target);
            drawText(target, m_error);
        }
        m_backButton.draw(target);
    }

    void JoinLobbyScreen::applyPendingActions() {
        if (m_connectRequested) {
            m_connectRequested = false;
            connect();
        }
        if (m_backRequested) {
            m_backRequested = false;
            // Închiderea legăturii îi spune gazdei că am plecat.
            m_context.screens.change(std::make_unique<MenuScreen>(m_context));
        }
    }

    void JoinLobbyScreen::connect() {
        if (m_client) {
            return;
        }
        const sf::String name = trimmed(m_nameInput.getText());
        if (name.isEmpty()) {
            showError(utf8("Scrie-ți numele înainte să te conectezi"));
            return;
        }
        // Fereastra stă câteva secunde dacă gazda nu răspunde.
        Backend::EConnectResult result = Backend::EConnectResult::Unreachable;
        m_client = Backend::INetworkClient::Produce(
            toUtf8String(trimmed(m_addressInput.getText())), toUtf8String(name), result);
        if (!m_client) {
            showError(connectError(result));
            return;
        }
        m_client->AddListener(m_events);
        m_players.clear();
        showError("");
        showHint(utf8("Se așteaptă răspunsul gazdei..."));
    }

    void JoinLobbyScreen::disconnect(const sf::String& error) {
        m_disconnectError.reset();
        m_client.reset();
        m_players.clear();
        showHint(utf8("Scrie-ți numele și adresa afișată pe calculatorul gazdei"));
        showError(error);
    }

    void JoinLobbyScreen::openTable() {
        m_gameStarted = false;
        m_client->RemoveListener(m_events.get());
        const std::vector<sf::String> names = utf8All(m_client->GetPlayerNames());
        m_context.screens.change(std::make_unique<TableScreen>(
            m_context, names, std::make_unique<NetworkSession>(std::move(m_client))));
    }

    void JoinLobbyScreen::showError(const sf::String& message) {
        m_error.setString(message);
        centerHorizontally(m_error, kErrorY);
    }

    void JoinLobbyScreen::showHint(const sf::String& message) {
        m_hint.setString(message);
        centerHorizontally(m_hint, 125.f);
    }
}  // namespace Frontend
