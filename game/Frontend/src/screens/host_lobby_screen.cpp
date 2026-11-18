#include <screens/host_lobby_screen.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include <SFML/Network/IpAddress.hpp>

#include <Backend/IGame.h>
#include <app/layout.h>
#include <net/host_session.h>
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
        constexpr sf::Vector2f kInputSize{360.f, 56.f};
        constexpr sf::Vector2f kCreateSize{160.f, 56.f};
        constexpr float kRowGap = 20.f;
        constexpr float kRowWidth = kInputSize.x + kRowGap + kCreateSize.x;
        constexpr float kInputX = (kLogicalWidth - kRowWidth) / 2.f;
        constexpr float kInputY = 180.f;
        constexpr float kListY = 290.f;

        constexpr sf::Vector2f kBottomButtonSize{200.f, 64.f};
        constexpr float kBottomY = kLogicalHeight - kBottomButtonSize.y - 40.f;

        // Câte calculatoare pot aștepta în același timp să-și trimită numele.
        constexpr std::size_t kMaxConnections = 8;
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
                        kBottomButtonSize) {
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

        refreshButtons();
    }

    void HostLobbyScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                m_backRequested = true;
            }
        }

        if (!m_hosting) {
            m_nameInput.handleEvent(event, m_context.window);
            m_createButton.handleEvent(event, m_context.window);
        }
        m_backButton.handleEvent(event, m_context.window);
        m_startButton.handleEvent(event, m_context.window);

        applyPendingActions();
    }

    void HostLobbyScreen::update(float dt) {
        m_nameInput.update(dt);
        if (m_hosting) {
            acceptConnections();
            receiveMessages();
        }
    }

    void HostLobbyScreen::draw(sf::RenderTarget& target) {
        m_background.draw(target);
        drawText(target, m_title);
        drawText(target, m_hint);
        if (m_hosting) {
            drawText(target, m_address);
            drawPlayerRows(target, m_context.font, playerNames(), {kInputX, kListY}, kRowWidth);
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
            startHosting();
        }
        if (m_backRequested) {
            m_backRequested = false;
            m_context.screens.change(std::make_unique<MenuScreen>(m_context));
        } else if (m_startRequested) {
            m_startRequested = false;
            if (joinedCount() > 0) {
                startGame();
            }
        }
    }

    void HostLobbyScreen::startHosting() {
        if (m_hosting) {
            return;
        }
        const sf::String name = trimmed(m_nameInput.getText());
        if (name.isEmpty()) {
            showError(utf8("Scrie-ți numele înainte să creezi jocul"));
            return;
        }
        std::optional<unsigned short> port;
        for (unsigned short i = 0; i < Net::kPortCount && !port; ++i) {
            const auto candidate = static_cast<unsigned short>(Net::kDefaultPort + i);
            if (m_listener.listen(candidate) == sf::Socket::Status::Done) {
                port = candidate;
            }
        }
        if (!port) {
            showError(utf8("Nu am găsit niciun port liber (mai sunt jocuri deschise?)"));
            return;
        }
        m_listener.setBlocking(false);
        m_hosting = true;
        m_hostName = name;

        m_hint.setString(utf8("Ceilalți apasă „Intră într-un joc” și scriu adresa:"));
        centerHorizontally(m_hint, 125.f);
        const std::optional<sf::IpAddress> address = sf::IpAddress::getLocalAddress();
        // Portul se scrie doar când nu e cel obișnuit; atunci trebuie scris și la conectare.
        const std::string portSuffix =
            *port == Net::kDefaultPort ? "" : ":" + std::to_string(*port);
        m_address.setString(address ? utf8(address->toString() + portSuffix)
                                    : utf8("adresa acestui calculator nu a putut fi aflată"));
        centerHorizontally(m_address, 175.f);
        showError("");
        refreshButtons();
    }

    void HostLobbyScreen::acceptConnections() {
        while (m_connections.size() < kMaxConnections) {
            auto socket = std::make_unique<sf::TcpSocket>();
            if (m_listener.accept(*socket) != sf::Socket::Status::Done) {
                return;
            }
            socket->setBlocking(false);
            m_connections.push_back({std::move(socket), std::nullopt});
        }
    }

    void HostLobbyScreen::receiveMessages() {
        bool listChanged = false;
        for (Connection& connection : m_connections) {
            sf::Packet packet;
            sf::Socket::Status status = connection.socket->receive(packet);
            while (status == sf::Socket::Status::Done && !connection.closed) {
                const std::optional<Net::MessageType> type = Net::readMessageType(packet);
                if (type == Net::MessageType::Join && !connection.name) {
                    handleJoin(connection, packet);
                    listChanged = listChanged || connection.name.has_value();
                }
                packet.clear();
                status = connection.socket->receive(packet);
            }
            if (status == sf::Socket::Status::Disconnected ||
                status == sf::Socket::Status::Error) {
                connection.closed = true;
                listChanged = listChanged || connection.name.has_value();
            }
        }
        std::erase_if(m_connections,
                      [](const Connection& connection) { return connection.closed; });
        if (listChanged) {
            broadcastPlayerList();
            refreshButtons();
        }
    }

    void HostLobbyScreen::handleJoin(Connection& connection, sf::Packet& packet) {
        std::uint32_t version = 0;
        sf::String rawName;
        if (!(packet >> version >> rawName)) {
            connection.closed = true;
            return;
        }
        if (version != Net::kProtocolVersion) {
            reject(connection, utf8("Gazda are altă versiune a jocului."));
            return;
        }
        if (joinedCount() + 1 >= static_cast<std::size_t>(Backend::kMaxPlayers)) {
            reject(connection, utf8("Masa e plină."));
            return;
        }
        const sf::String name = trimmed(rawName);
        if (name.isEmpty() || name.getSize() > LobbyScreen::kMaxNameLength) {
            reject(connection, utf8("Numele nu e bun."));
            return;
        }
        const std::vector<sf::String> names = playerNames();
        if (std::find(names.begin(), names.end(), name) != names.end()) {
            reject(connection, utf8("Există deja un jucător cu numele ăsta."));
            return;
        }
        connection.name = name;
    }

    void HostLobbyScreen::reject(Connection& connection, const sf::String& reason) {
        sf::Packet packet = Net::makeMessage(Net::MessageType::Rejected);
        packet << reason;
        Net::sendAll(*connection.socket, packet);
        connection.socket->disconnect();
        connection.closed = true;
    }

    void HostLobbyScreen::broadcastPlayerList() {
        const std::vector<sf::String> names = playerNames();
        sf::Packet packet = Net::makeMessage(Net::MessageType::PlayerList);
        packet << static_cast<std::uint8_t>(names.size());
        for (const sf::String& name : names) {
            packet << name;
        }
        for (Connection& connection : m_connections) {
            if (connection.name && !Net::sendAll(*connection.socket, packet)) {
                connection.closed = true;
            }
        }
    }

    std::vector<sf::String> HostLobbyScreen::playerNames() const {
        std::vector<sf::String> names{m_hostName};
        for (const Connection& connection : m_connections) {
            if (connection.name && !connection.closed) {
                names.push_back(*connection.name);
            }
        }
        return names;
    }

    std::size_t HostLobbyScreen::joinedCount() const { return playerNames().size() - 1; }

    void HostLobbyScreen::startGame() {
        const std::vector<sf::String> names = playerNames();
        std::vector<Net::RemotePlayer> remotePlayers;
        for (Connection& connection : m_connections) {
            if (!connection.name || connection.closed) {
                continue;  // n-a apucat să intre: rămâne pe dinafară
            }
            const std::size_t seat = remotePlayers.size() + 1;  // gazda e la locul 0
            sf::Packet packet = Net::makeMessage(Net::MessageType::GameStarting);
            packet << static_cast<std::uint8_t>(seat) << static_cast<std::uint8_t>(names.size());
            for (const sf::String& name : names) {
                packet << name;
            }
            // Dacă n-a mers, HostSession vede la primul mesaj că jucătorul a ieșit.
            Net::sendAll(*connection.socket, packet);
            remotePlayers.push_back({std::move(connection.socket), seat, *connection.name});
        }
        m_listener.close();
        m_context.screens.change(std::make_unique<TableScreen>(
            m_context, names,
            std::make_unique<Net::HostSession>(names, std::move(remotePlayers))));
    }

    void HostLobbyScreen::showError(const sf::String& message) {
        m_error.setString(message);
        centerHorizontally(m_error, kInputY + kInputSize.y + 18.f);
    }

    void HostLobbyScreen::refreshButtons() {
        m_startButton.setEnabled(m_hosting && joinedCount() > 0);
    }
}  // namespace Frontend
