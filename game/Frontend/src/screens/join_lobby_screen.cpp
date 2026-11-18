#include <screens/join_lobby_screen.h>

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/System/Time.hpp>

#include <Backend/IGame.h>
#include <app/layout.h>
#include <net/client_session.h>
#include <net/protocol.h>
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

        // Cât așteptăm gazda să răspundă la conectare (fereastra stă între timp).
        const sf::Time kConnectTimeout = sf::seconds(3.f);

        // „192.168.1.5” sau „192.168.1.5:53001” (portul apare la gazdă doar când nu e cel
        // obișnuit). nullopt dacă portul scris nu e un număr bun.
        std::optional<std::pair<std::string, unsigned short>> splitAddress(
            const std::string& text) {
            const std::size_t colon = text.rfind(':');
            if (colon == std::string::npos) {
                return std::pair{text, Net::kDefaultPort};
            }
            unsigned short port = 0;
            const char* begin = text.data() + colon + 1;
            const char* end = text.data() + text.size();
            const auto [last, error] = std::from_chars(begin, end, port);
            if (error != std::errc{} || last != end || port == 0) {
                return std::nullopt;
            }
            return std::pair{text.substr(0, colon), port};
        }

        bool readNames(sf::Packet& packet, std::vector<sf::String>& names) {
            std::uint8_t count = 0;
            if (!(packet >> count) || count > Backend::kMaxPlayers) {
                return false;
            }
            names.assign(count, {});
            for (sf::String& name : names) {
                if (!(packet >> name) || name.getSize() > Net::kMaxNameLength) {
                    return false;
                }
            }
            return true;
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
                       kBottomButtonSize) {
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
    }

    bool JoinLobbyScreen::isTypingText() const {
        return !m_socket && (m_nameInput.isFocused() || m_addressInput.isFocused());
    }

    void JoinLobbyScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                m_backRequested = true;
            } else if (key->code == sf::Keyboard::Key::Tab && !m_socket) {
                const bool nameFocused = m_nameInput.isFocused();
                m_nameInput.setFocused(!nameFocused);
                m_addressInput.setFocused(nameFocused);
            }
        }

        if (!m_socket) {
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
        if (m_socket) {
            receiveMessages();
        }
    }

    void JoinLobbyScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);
        drawText(target, m_title);
        drawText(target, m_hint);
        if (m_socket) {
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
            // Închiderea socket-ului îi spune gazdei că am plecat.
            m_context.screens.change(std::make_unique<MenuScreen>(m_context));
        }
    }

    void JoinLobbyScreen::connect() {
        if (m_socket) {
            return;
        }
        const sf::String name = trimmed(m_nameInput.getText());
        if (name.isEmpty()) {
            showError(utf8("Scrie-ți numele înainte să te conectezi"));
            return;
        }
        const std::string addressText = toUtf8String(trimmed(m_addressInput.getText()));
        const auto hostAndPort = splitAddress(addressText);
        const std::optional<sf::IpAddress> address =
            hostAndPort && !hostAndPort->first.empty() ? sf::IpAddress::resolve(hostAndPort->first)
                                                       : std::nullopt;
        if (!address) {
            showError(utf8("Adresa gazdei nu e bună"));
            return;
        }

        auto socket = std::make_unique<sf::TcpSocket>();
        if (socket->connect(*address, hostAndPort->second, kConnectTimeout) !=
            sf::Socket::Status::Done) {
            showError(utf8("Nu m-am putut conecta. E bună adresa? A creat gazda jocul?"));
            return;
        }
        socket->setBlocking(false);

        sf::Packet join = Net::makeMessage(Net::MessageType::Join);
        join << Net::kProtocolVersion << name;
        if (!Net::sendAll(*socket, join)) {
            showError(utf8("Legătura cu gazda s-a întrerupt"));
            return;
        }
        m_socket = std::move(socket);
        m_players.clear();
        showError("");
        showHint(utf8("Se așteaptă răspunsul gazdei..."));
    }

    void JoinLobbyScreen::disconnect(const sf::String& error) {
        m_socket.reset();
        m_players.clear();
        showHint(utf8("Scrie-ți numele și adresa afișată pe calculatorul gazdei"));
        showError(error);
    }

    void JoinLobbyScreen::receiveMessages() {
        sf::Packet packet;
        sf::Socket::Status status = m_socket->receive(packet);
        while (status == sf::Socket::Status::Done) {
            switch (Net::readMessageType(packet).value_or(Net::MessageType::Join)) {
                case Net::MessageType::PlayerList:
                    if (readNames(packet, m_players)) {
                        showHint(utf8("Ai intrat. Se așteaptă ca gazda să pornească jocul."));
                    }
                    break;
                case Net::MessageType::Rejected: {
                    sf::String reason;
                    packet >> reason;
                    disconnect(reason);
                    return;
                }
                case Net::MessageType::GameStarting:
                    startGame(packet);
                    return;
                default:
                    break;
            }
            packet.clear();
            status = m_socket->receive(packet);
        }
        if (status == sf::Socket::Status::Disconnected || status == sf::Socket::Status::Error) {
            disconnect(utf8("Gazda a închis jocul"));
        }
    }

    void JoinLobbyScreen::startGame(sf::Packet& packet) {
        std::uint8_t seat = 0;
        std::vector<sf::String> names;
        if (!(packet >> seat) || !readNames(packet, names) || seat >= names.size()) {
            disconnect(utf8("Gazda a trimis ceva greșit"));
            return;
        }
        // Socket-ul trece la masă împreună cu ce a mai primit; mesajele jocului care vin
        // între timp așteaptă în el până le citește ClientSession.
        const std::size_t seatCount = names.size();
        m_context.screens.change(std::make_unique<TableScreen>(
            m_context, names,
            std::make_unique<Net::ClientSession>(std::move(m_socket), seat, seatCount)));
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
