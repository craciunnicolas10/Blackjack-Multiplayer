#include <screens/table_screen.h>

#include <algorithm>
#include <string>
#include <utility>

#include <app/layout.h>
#include <game/fake_game.h>
#include <screens/menu_screen.h>
#include <screens/screen_manager.h>
#include <ui/card_textures.h>
#include <ui/card_view.h>
#include <ui/utf8.h>

namespace Frontend {
    namespace {
        constexpr sf::Vector2f kDealerPosition{kLogicalWidth / 2.f, 70.f};  // mijloc-sus al mâinii
        constexpr float kSeatY = 380.f;
        // Colțul stânga-sus al pachetului; de aici pleacă toate cărțile.
        constexpr sf::Vector2f kDeckPosition{kLogicalWidth - 150.f, 40.f};
        constexpr int kDeckLayers = 4;

        constexpr float kDealInterval = 0.25f;  // secunde între două cărți împărțite

        constexpr sf::Vector2f kActionSize{150.f, 56.f};
        constexpr float kActionGap = 20.f;
        constexpr float kActionY = kLogicalHeight - kActionSize.y - 24.f;
        constexpr float kActionsLeft = (kLogicalWidth - 3 * kActionSize.x - 2 * kActionGap) / 2.f;

        constexpr float kScoreRadius = 17.f;
        constexpr sf::Vector2f kResultSize{150.f, 32.f};
        constexpr float kResultGap = 16.f;  // între etichetă și cărți

        const sf::Color kGold(240, 210, 120);

        sf::Vector2f actionPosition(int index) {
            return {kActionsLeft + static_cast<float>(index) * (kActionSize.x + kActionGap),
                    kActionY};
        }

        const char* resultLabel(RoundResult result) {
            switch (result) {
                case RoundResult::Win:
                    return "Câștig";
                case RoundResult::Lose:
                    return "Pierdere";
                case RoundResult::Push:
                    return "Egal";
                case RoundResult::Blackjack:
                    return "Blackjack!";
            }
            return "";
        }

        sf::Color resultColor(RoundResult result) {
            switch (result) {
                case RoundResult::Win:
                    return sf::Color(40, 140, 60);
                case RoundResult::Lose:
                    return sf::Color(170, 40, 40);
                case RoundResult::Push:
                    return sf::Color(100, 100, 100);
                case RoundResult::Blackjack:
                    return sf::Color(190, 140, 20);
            }
            return sf::Color::Black;
        }
    }  // namespace

    TableScreen::TableScreen(AppContext context, std::vector<sf::String> playerNames)
        : m_context(context),
          m_playerNames(std::move(playerNames)),
          m_scores(m_playerNames.size()),
          m_results(m_playerNames.size()),
          m_dealerHand(context.cards),
          m_hitButton(context.font, utf8("Hit"), actionPosition(0), kActionSize),
          m_standButton(context.font, utf8("Stand"), actionPosition(1), kActionSize),
          m_doubleButton(context.font, utf8("Double"), actionPosition(2), kActionSize),
          m_menuButton(context.font, utf8("Meniu"), {24.f, 24.f}, {140.f, 48.f}),
          m_bettingPanel(context.font, {(kLogicalWidth - BettingPanel::kSize.x) / 2.f, 80.f}) {
        m_seats.reserve(m_playerNames.size());
        for (const sf::String& name : m_playerNames) {
            m_seats.emplace_back(context.font, context.cards, name);
        }
        for (std::size_t seat = 0; seat < m_seats.size(); ++seat) {
            m_seats[seat].setPosition(seatPosition(seat));
        }
        m_dealerHand.setPosition(kDealerPosition);

        m_hitButton.setOnClick([this] { requestAction(Action::Hit); });
        m_standButton.setOnClick([this] { requestAction(Action::Stand); });
        m_doubleButton.setOnClick([this] { requestAction(Action::DoubleDown); });
        m_menuButton.setOnClick([this] { m_menuRequested = true; });
        m_bettingPanel.setOnConfirm([this](int bet) { m_pendingBet = bet; });
        refreshActionButtons();

        m_game = std::make_unique<FakeGame>(*this);
        m_game->startRound();
    }

    // Aici, nu în header, pentru că acolo FakeGame e doar declarat.
    TableScreen::~TableScreen() = default;

    void TableScreen::handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            switch (key->code) {
                case sf::Keyboard::Key::Escape:
                    m_menuRequested = true;
                    break;
                case sf::Keyboard::Key::H:
                    requestAction(Action::Hit);
                    break;
                case sf::Keyboard::Key::S:
                    requestAction(Action::Stand);
                    break;
                case sf::Keyboard::Key::D:
                    requestAction(Action::DoubleDown);
                    break;
                default:
                    break;
            }
        }

        m_hitButton.handleEvent(event, m_context.window);
        m_standButton.handleEvent(event, m_context.window);
        m_doubleButton.handleEvent(event, m_context.window);
        m_menuButton.handleEvent(event, m_context.window);
        if (m_bettingSeat) {
            m_bettingPanel.handleEvent(event, m_context.window);
        }

        applyPendingActions();
    }

    void TableScreen::update(float dt) {
        m_dealQueue = std::max(m_dealQueue - dt, 0.f);

        if (m_pendingReveal) {
            m_pendingReveal->delay -= dt;
            if (m_pendingReveal->delay <= 0.f) {
                const PendingReveal reveal = *m_pendingReveal;
                m_pendingReveal.reset();
                for (std::size_t i = 0; i < m_dealerHand.getCardCount(); ++i) {
                    if (!m_dealerHand.getCard(i).isKnown()) {
                        m_dealerHand.revealCard(i, reveal.rank, reveal.suit);
                        break;
                    }
                }
            }
        }

        m_dealerHand.update(dt);
        for (SeatView& seat : m_seats) {
            seat.update(dt);
        }

        if (m_game) {
            m_game->update(dt);
        }
    }

    void TableScreen::draw(sf::RenderTarget& target) {
        sf::RectangleShape background({kLogicalWidth, kLogicalHeight});
        background.setFillColor(sf::Color(15, 90, 50));
        target.draw(background);

        drawDeck(target);

        drawLabel(target, utf8("Dealer"), {kDealerPosition.x, kDealerPosition.y - 46.f});
        m_dealerHand.draw(target);
        // Scorul dealerului apare după ce s-a întors cartea, ca să nu dea de gol cartea ascunsă.
        const bool dealerSettled = !m_pendingReveal && !m_dealerHand.isAnimating();
        if (m_dealerScore && m_dealerHand.getCardCount() > 0 && dealerSettled) {
            drawScore(target, m_dealerHand, *m_dealerScore);
        }

        // Rezultatele apar abia după ce au ajuns toate cărțile, altfel s-ar vedea înainte de
        // cartea care a decis runda.
        const bool showResults = !isAnimating();
        for (std::size_t seat = 0; seat < m_seats.size(); ++seat) {
            const HandView& hand = m_seats[seat].getHand();
            m_seats[seat].draw(target);
            // Scorul apare după ce ajung cărțile, ca să nu arate o carte care e încă pe drum.
            if (m_scores[seat] && hand.getCardCount() > 0 && !hand.isAnimating()) {
                drawScore(target, hand, *m_scores[seat]);
            }
            if (showResults && m_results[seat]) {
                drawResult(target, hand, *m_results[seat]);
            }
        }

        m_hitButton.draw(target);
        m_standButton.draw(target);
        m_doubleButton.draw(target);
        m_menuButton.draw(target);

        if (m_bettingSeat) {
            m_bettingPanel.draw(target);
        }
    }

    std::size_t TableScreen::getSeatCount() const { return m_seats.size(); }

    void TableScreen::setPlayerMoney(std::size_t seat, int money) {
        m_seats.at(seat).setMoney(money);
    }

    void TableScreen::setPlayerBet(std::size_t seat, int bet) { m_seats.at(seat).setBet(bet); }

    void TableScreen::setPlayerScore(std::size_t seat, std::optional<int> score) {
        m_scores.at(seat) = score;
    }

    void TableScreen::setDealerScore(std::optional<int> score) { m_dealerScore = score; }

    void TableScreen::dealPlayerCard(std::size_t seat, int rank, int suit) {
        m_seats.at(seat).getHand().dealCard(rank, suit, true, kDeckPosition, takeDealDelay());
        if (m_currentSeat == seat) {
            refreshActionButtons();
        }
    }

    void TableScreen::dealDealerCard(int rank, int suit, bool faceDown) {
        if (faceDown) {
            m_dealerHand.dealHiddenCard(kDeckPosition, takeDealDelay());
        } else {
            m_dealerHand.dealCard(rank, suit, true, kDeckPosition, takeDealDelay());
        }
    }

    void TableScreen::revealDealerCard(int rank, int suit) {
        // Se întoarce după ce au ajuns cărțile deja împărțite, iar cărțile cerute după ea
        // (dealerul trage) vin după întoarcere.
        m_pendingReveal = PendingReveal{rank, suit, m_dealQueue};
        m_dealQueue += CardView::kFlipDuration + kDealInterval;
    }

    void TableScreen::setCurrentSeat(std::optional<std::size_t> seat) {
        if (seat && *seat >= m_seats.size()) {
            seat.reset();
        }
        m_currentSeat = seat;
        refreshSeatHighlight();
        refreshActionButtons();
    }

    std::optional<std::size_t> TableScreen::getCurrentSeat() const { return m_currentSeat; }

    void TableScreen::startBetting(std::size_t seat, int money, int minBet, int maxBet,
                                   int initialBet) {
        if (seat >= m_seats.size()) {
            return;
        }
        m_bettingSeat = seat;
        m_bettingPanel.open(m_playerNames[seat], money, minBet, maxBet, initialBet);
        refreshSeatHighlight();
        refreshActionButtons();
    }

    void TableScreen::endBetting() {
        m_bettingSeat.reset();
        m_pendingBet.reset();
        refreshSeatHighlight();
        refreshActionButtons();
    }

    std::optional<std::size_t> TableScreen::getBettingSeat() const { return m_bettingSeat; }

    void TableScreen::showResult(std::size_t seat, RoundResult result) {
        m_results.at(seat) = result;
    }

    void TableScreen::clearRound() {
        for (std::size_t seat = 0; seat < m_seats.size(); ++seat) {
            m_seats[seat].getHand().clear();
            m_scores[seat].reset();
            m_results[seat].reset();
        }
        m_dealerHand.clear();
        m_dealerScore.reset();
        m_pendingReveal.reset();
        m_dealQueue = 0.f;
        endBetting();
        setCurrentSeat(std::nullopt);
    }

    bool TableScreen::isAnimating() const {
        if (m_dealQueue > 0.f || m_pendingReveal || m_dealerHand.isAnimating()) {
            return true;
        }
        return std::any_of(m_seats.begin(), m_seats.end(),
                           [](const SeatView& seat) { return seat.getHand().isAnimating(); });
    }

    void TableScreen::setOnHit(std::function<void()> onHit) { m_onHit = std::move(onHit); }

    void TableScreen::setOnStand(std::function<void()> onStand) {
        m_onStand = std::move(onStand);
    }

    void TableScreen::setOnDoubleDown(std::function<void()> onDoubleDown) {
        m_onDoubleDown = std::move(onDoubleDown);
    }

    void TableScreen::setOnBet(std::function<void(int)> onBet) { m_onBet = std::move(onBet); }

    // Locurile sunt împărțite egal pe lățimea mesei, oricâți jucători ar fi.
    sf::Vector2f TableScreen::seatPosition(std::size_t seat) const {
        const float slots = static_cast<float>(m_seats.size() + 1);
        return {kLogicalWidth * static_cast<float>(seat + 1) / slots, kSeatY};
    }

    float TableScreen::takeDealDelay() {
        const float delay = m_dealQueue;
        m_dealQueue += kDealInterval;
        return delay;
    }

    void TableScreen::refreshActionButtons() {
        const bool myTurn = m_currentSeat.has_value() && !m_bettingSeat.has_value();
        m_hitButton.setEnabled(myTurn);
        m_standButton.setEnabled(myTurn);
        // Double se poate cere doar pe primele două cărți. Dacă regula e alta, se schimbă aici.
        m_doubleButton.setEnabled(myTurn &&
                                  m_seats[*m_currentSeat].getHand().getCardCount() == 2);
    }

    void TableScreen::refreshSeatHighlight() {
        for (std::size_t seat = 0; seat < m_seats.size(); ++seat) {
            const bool active = seat == m_currentSeat || seat == m_bettingSeat;
            // setActive repornește pulsul, așa că îl apelăm doar când se schimbă ceva.
            if (m_seats[seat].isActive() != active) {
                m_seats[seat].setActive(active);
            }
        }
    }

    void TableScreen::requestAction(Action action) {
        const Button& button = action == Action::Hit     ? m_hitButton
                               : action == Action::Stand ? m_standButton
                                                         : m_doubleButton;
        if (button.isEnabled()) {
            m_pendingAction = action;
        }
    }

    void TableScreen::applyPendingActions() {
        if (m_menuRequested) {
            m_menuRequested = false;
            m_pendingAction.reset();
            m_pendingBet.reset();
            goToMenu();
            return;
        }

        if (m_pendingBet) {
            const int bet = *m_pendingBet;
            m_pendingBet.reset();
            if (m_onBet) {
                m_onBet(bet);
            }
        }

        if (!m_pendingAction) {
            return;
        }
        const Action action = *m_pendingAction;
        m_pendingAction.reset();

        const std::function<void()>& callback = action == Action::Hit     ? m_onHit
                                                : action == Action::Stand ? m_onStand
                                                                          : m_onDoubleDown;
        if (callback) {
            callback();
        }
    }

    void TableScreen::goToMenu() {
        m_context.screens.change(std::make_unique<MenuScreen>(m_context));
    }

    void TableScreen::drawDeck(sf::RenderTarget& target) const {
        // Câteva cărți cu spatele în sus, ușor decalate, ca să pară un teanc.
        for (int layer = kDeckLayers - 1; layer >= 0; --layer) {
            CardView card(m_context.cards);
            const float offset = 2.f * static_cast<float>(layer);
            card.setPosition({kDeckPosition.x + offset, kDeckPosition.y + offset});
            card.draw(target);
        }
    }

    void TableScreen::drawLabel(sf::RenderTarget& target, const sf::String& text,
                                sf::Vector2f centerTop) const {
        sf::Text label(m_context.font, text, 22);
        const sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
        label.setPosition(centerTop);
        label.setFillColor(sf::Color::White);
        target.draw(label);
    }

    // Bulina cu scorul, pe colțul din dreapta-sus al mâinii. Roșie peste 21.
    void TableScreen::drawScore(sf::RenderTarget& target, const HandView& hand, int score) const {
        const sf::FloatRect bounds = hand.getBounds();
        const sf::Vector2f center{bounds.position.x + bounds.size.x, bounds.position.y};

        sf::CircleShape bubble(kScoreRadius);
        bubble.setOrigin({kScoreRadius, kScoreRadius});
        bubble.setPosition(center);
        bubble.setFillColor(score > 21 ? sf::Color(170, 40, 40) : sf::Color(20, 20, 20));
        bubble.setOutlineColor(kGold);
        bubble.setOutlineThickness(2.f);
        target.draw(bubble);

        sf::Text text(m_context.font, std::to_string(score), 18);
        const sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.position + textBounds.size / 2.f);
        text.setPosition(center);
        text.setFillColor(sf::Color::White);
        target.draw(text);
    }

    // Eticheta cu rezultatul, centrată deasupra mâinii.
    void TableScreen::drawResult(sf::RenderTarget& target, const HandView& hand,
                                 RoundResult result) const {
        const sf::FloatRect bounds = hand.getBounds();
        const sf::Vector2f center{bounds.position.x + bounds.size.x / 2.f,
                                  bounds.position.y - kResultGap - kResultSize.y / 2.f};

        sf::RectangleShape badge(kResultSize);
        badge.setOrigin(kResultSize / 2.f);
        badge.setPosition(center);
        badge.setFillColor(resultColor(result));
        badge.setOutlineColor(sf::Color::White);
        badge.setOutlineThickness(2.f);
        target.draw(badge);

        sf::Text text(m_context.font, utf8(resultLabel(result)), 20);
        const sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.position + textBounds.size / 2.f);
        text.setPosition(center);
        text.setFillColor(sf::Color::White);
        target.draw(text);
    }
}  // namespace Frontend
