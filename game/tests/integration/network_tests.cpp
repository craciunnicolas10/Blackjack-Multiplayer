#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <Backend/INetworkClient.h>
#include <Backend/INetworkHost.h>

#include "test_support.h"

using namespace Backend;
using namespace TestSupport;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::SaveArg;

class NetworkTest : public ::testing::Test {
   protected:
    struct Peer {
        std::string name;
        std::shared_ptr<INetworkClient> client;
        std::shared_ptr<NiceMock<MockNetworkListener>> listener;
        std::vector<std::string> names;
        std::optional<EJoinRejection> rejection;
        bool started = false;
        bool hostLeft = false;
    };

    void SetUp() override {
        hostListener = std::make_shared<NiceMock<MockNetworkListener>>();
        ON_CALL(*hostListener, OnPlayersChanged(_)).WillByDefault(SaveArg<0>(&hostNames));
        ON_CALL(*hostListener, OnGameStarted()).WillByDefault([this]() { hostStarted = true; });
        ON_CALL(*hostListener, OnPlayerLeft(_)).WillByDefault([this](const std::string& name) {
            leftName = name;
        });
        host = INetworkHost::Produce("Host");
        ASSERT_NE(host, nullptr);
        host->AddListener(hostListener);
    }

    std::string Address() const {
        const std::string address = host->GetAddress();
        const auto colon = address.find(':');
        return "127.0.0.1" + (colon == std::string::npos ? std::string() : address.substr(colon));
    }

    Peer* Connect(const std::string& name) {
        auto peer = std::make_unique<Peer>();
        peer->name = name;
        peer->listener = std::make_shared<NiceMock<MockNetworkListener>>();
        Peer* raw = peer.get();
        ON_CALL(*peer->listener, OnPlayersChanged(_)).WillByDefault(SaveArg<0>(&raw->names));
        ON_CALL(*peer->listener, OnJoinRejected(_)).WillByDefault([raw](EJoinRejection reason) {
            raw->rejection = reason;
        });
        ON_CALL(*peer->listener, OnGameStarted()).WillByDefault([raw]() { raw->started = true; });
        ON_CALL(*peer->listener, OnHostLeft()).WillByDefault([raw]() { raw->hostLeft = true; });

        EConnectResult result = EConnectResult::Unreachable;
        peer->client = INetworkClient::Produce(Address(), name, result);
        EXPECT_EQ(result, EConnectResult::Connected);
        EXPECT_NE(peer->client, nullptr);
        if (peer->client) {
            peer->client->AddListener(peer->listener);
        }
        peers.push_back(std::move(peer));
        return raw;
    }

    void Tick() {
        if (host) {
            host->Update();
        }
        for (auto& peer : peers) {
            if (peer->client) {
                peer->client->Update();
            }
        }
    }

    bool Pump(const std::function<bool()>& done,
              std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) {
        return PumpUntil([this]() { Tick(); }, done, timeout);
    }

    void StartGameWithOnePeer() {
        Peer* ana = Connect("Ana");
        ASSERT_NE(ana->client, nullptr);
        ASSERT_TRUE(Pump([&]() { return host->CanStartGame() && ana->names.size() == 2; }));
        ASSERT_TRUE(host->StartGame());
        ASSERT_TRUE(Pump([&]() { return hostStarted && ana->started; }));
        hostGame = host->GetGame();
        clientGame = ana->client->GetGame();
        ASSERT_NE(hostGame, nullptr);
        ASSERT_NE(clientGame, nullptr);
    }

    void BetBoth() {
        ASSERT_TRUE(hostGame->StartRound());
        ASSERT_TRUE(Pump([&]() { return clientGame->GetState() == EGameState::Betting; }));
        ASSERT_TRUE(hostGame->PlaceBet(0, 100));
        ASSERT_TRUE(clientGame->PlaceBet(1, 100));
        ASSERT_TRUE(Pump([&]() { return hostGame->GetState() != EGameState::Betting; }));
    }

    std::shared_ptr<INetworkHost> host;
    std::shared_ptr<NiceMock<MockNetworkListener>> hostListener;
    std::vector<std::unique_ptr<Peer>> peers;
    std::vector<std::string> hostNames;
    std::string leftName;
    bool hostStarted = false;
    std::shared_ptr<IGame> hostGame;
    std::shared_ptr<IGame> clientGame;
};

TEST(NetworkHostProduceTest, RejectsInvalidNames) {
    EXPECT_EQ(INetworkHost::Produce(""), nullptr);
    EXPECT_EQ(INetworkHost::Produce(std::string(200, 'a')), nullptr);
}

TEST(NetworkHostProduceTest, FallsBackToNextPortsAndRunsOut) {
    std::vector<std::shared_ptr<INetworkHost>> hosts;
    for (int i = 0; i < 10; ++i) {
        auto host = INetworkHost::Produce("Host" + std::to_string(i));
        ASSERT_NE(host, nullptr) << i;
        hosts.push_back(host);
    }

    EXPECT_EQ(INetworkHost::Produce("OneTooMany"), nullptr);

    hosts.pop_back();
    EXPECT_NE(INetworkHost::Produce("Again"), nullptr);
}

TEST(NetworkClientProduceTest, RejectsInvalidName) {
    EConnectResult result = EConnectResult::Connected;

    const auto client = INetworkClient::Produce("127.0.0.1", "", result);

    EXPECT_EQ(client, nullptr);
    EXPECT_EQ(result, EConnectResult::InvalidName);
}

TEST(NetworkClientProduceTest, RejectsInvalidAddress) {
    EConnectResult result = EConnectResult::Connected;

    const auto client = INetworkClient::Produce("127.0.0.1:abc", "Ana", result);

    EXPECT_EQ(client, nullptr);
    EXPECT_EQ(result, EConnectResult::InvalidAddress);
}

TEST(NetworkClientProduceTest, ReportsUnreachableHost) {
    EConnectResult result = EConnectResult::Connected;

    const auto client = INetworkClient::Produce("127.0.0.1:1", "Ana", result);

    EXPECT_EQ(client, nullptr);
    EXPECT_EQ(result, EConnectResult::Unreachable);
}

TEST_F(NetworkTest, HostStartsAloneInTheLobby) {
    EXPECT_TRUE(host->IsHost());
    EXPECT_EQ(host->GetPlayerNames(), std::vector<std::string>{"Host"});
    EXPECT_EQ(host->GetGame(), nullptr);
    EXPECT_FALSE(host->CanStartGame());
    EXPECT_FALSE(host->StartGame());
}

TEST_F(NetworkTest, JoiningPlayersAppearInTheLobbyEverywhere) {
    Peer* ana = Connect("Ana");
    ASSERT_NE(ana->client, nullptr);

    ASSERT_TRUE(Pump([&]() { return hostNames.size() == 2 && ana->names.size() == 2; }));

    const std::vector<std::string> expected = {"Host", "Ana"};
    EXPECT_EQ(hostNames, expected);
    EXPECT_EQ(ana->names, expected);
    EXPECT_EQ(host->GetPlayerNames(), expected);
    EXPECT_EQ(ana->client->GetPlayerNames(), expected);
    EXPECT_TRUE(host->CanStartGame());
    EXPECT_FALSE(ana->client->IsHost());
    EXPECT_EQ(ana->client->GetLocalSeat(), -1);
    EXPECT_EQ(ana->client->GetGame(), nullptr);
}

TEST_F(NetworkTest, DuplicateNameIsRejected) {
    Peer* clone = Connect("Host");
    ASSERT_NE(clone->client, nullptr);

    ASSERT_TRUE(Pump([&]() { return clone->rejection.has_value(); }));

    EXPECT_EQ(*clone->rejection, EJoinRejection::NameTaken);
    EXPECT_EQ(host->GetPlayerNames(), std::vector<std::string>{"Host"});
}

TEST_F(NetworkTest, FullTableRejectsExtraPlayers) {
    Peer* first = Connect("A");
    Peer* second = Connect("B");
    Peer* third = Connect("C");
    ASSERT_TRUE(Pump([&]() {
        return first->names.size() == 4 && second->names.size() == 4 && third->names.size() == 4;
    }));

    Peer* extra = Connect("D");
    ASSERT_NE(extra->client, nullptr);
    ASSERT_TRUE(Pump([&]() { return extra->rejection.has_value(); }));

    EXPECT_EQ(*extra->rejection, EJoinRejection::TableFull);
    EXPECT_EQ(host->GetPlayerNames().size(), static_cast<std::size_t>(kMaxPlayers));
}

TEST_F(NetworkTest, PlayerLeavingTheLobbyIsRemovedFromTheList) {
    Peer* ana = Connect("Ana");
    ASSERT_TRUE(Pump([&]() { return hostNames.size() == 2; }));

    ana->client.reset();

    EXPECT_TRUE(Pump([&]() { return hostNames.size() == 1; }));
}

TEST_F(NetworkTest, HostLeavingTheLobbyIsReportedToPlayers) {
    Peer* ana = Connect("Ana");
    ASSERT_TRUE(Pump([&]() { return ana->names.size() == 2; }));

    host.reset();

    EXPECT_TRUE(Pump([&]() { return ana->hostLeft; }));
}

TEST_F(NetworkTest, ClientSeesTheHostsRoundStart) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());

    ASSERT_TRUE(hostGame->StartRound());

    EXPECT_TRUE(Pump([&]() { return clientGame->GetState() == EGameState::Betting; }));
    EXPECT_TRUE(clientGame->CanPlaceBet(1));
}

TEST_F(NetworkTest, BetsFromBothSidesStartTheDeal) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());

    ASSERT_NO_FATAL_FAILURE(BetBoth());

    EXPECT_EQ(hostGame->GetPlayer(1)->GetBet(), 100);
    ASSERT_TRUE(Pump([&]() { return clientGame->GetState() != EGameState::Betting; }));
    EXPECT_EQ(clientGame->GetPlayer(0)->GetBet(), 100);
    EXPECT_EQ(clientGame->GetPlayer(1)->GetBet(), 100);
    EXPECT_EQ(clientGame->GetPlayer(1)->GetBalance(), kStartingBalance - 100);
}

TEST_F(NetworkTest, ClientNeverSeesTheDealerHoleCard) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());
    ASSERT_NO_FATAL_FAILURE(BetBoth());
    if (hostGame->GetState() != EGameState::PlayersTurn) {
        GTEST_SKIP();
    }

    ASSERT_TRUE(Pump([&]() { return clientGame->GetState() == EGameState::PlayersTurn; }));

    EXPECT_EQ(clientGame->GetDealer()->GetHand().size(), 1u);
    EXPECT_EQ(clientGame->GetPlayer(0)->GetHand().size(), 2u);
}

TEST_F(NetworkTest, ActionOutOfTurnIsIgnoredByTheHost) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());
    ASSERT_NO_FATAL_FAILURE(BetBoth());
    if (hostGame->GetState() != EGameState::PlayersTurn ||
        hostGame->GetCurrentPlayerIndex() != 0) {
        GTEST_SKIP();
    }

    EXPECT_TRUE(clientGame->Hit());
    Pump([]() { return false; }, std::chrono::milliseconds(200));

    EXPECT_EQ(hostGame->GetCurrentPlayerIndex(), 0);
    EXPECT_EQ(hostGame->GetPlayer(0)->GetHand().size(), 2u);
    EXPECT_EQ(hostGame->GetPlayer(1)->GetHand().size(), 2u);
}

TEST_F(NetworkTest, FullRoundIsPlayedAcrossTheNetworkAndStaysInSync) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());
    ASSERT_NO_FATAL_FAILURE(BetBoth());

    for (int guard = 0; guard < 10 && hostGame->GetState() == EGameState::PlayersTurn; ++guard) {
        const int current = hostGame->GetCurrentPlayerIndex();
        if (current == 0) {
            ASSERT_TRUE(hostGame->Stand());
        } else {
            ASSERT_TRUE(clientGame->Stand());
            ASSERT_TRUE(Pump([&]() {
                return hostGame->GetState() != EGameState::PlayersTurn ||
                       hostGame->GetCurrentPlayerIndex() != current;
            }));
        }
    }

    ASSERT_EQ(hostGame->GetState(), EGameState::RoundOver);
    ASSERT_TRUE(Pump([&]() { return clientGame->GetState() == EGameState::RoundOver; }));
    for (int i = 0; i < 2; ++i) {
        EXPECT_EQ(clientGame->GetPlayer(i)->GetBalance(), hostGame->GetPlayer(i)->GetBalance());
        EXPECT_EQ(clientGame->GetPlayer(i)->GetHand(), hostGame->GetPlayer(i)->GetHand());
    }
    EXPECT_EQ(clientGame->GetDealer()->GetHand(), hostGame->GetDealer()->GetHand());
}

TEST_F(NetworkTest, HostCanStartAnotherRoundAfterTheFirstEnds) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());
    ASSERT_NO_FATAL_FAILURE(BetBoth());
    for (int guard = 0; guard < 10 && hostGame->GetState() == EGameState::PlayersTurn; ++guard) {
        const int current = hostGame->GetCurrentPlayerIndex();
        if (current == 0) {
            ASSERT_TRUE(hostGame->Stand());
        } else {
            ASSERT_TRUE(clientGame->Stand());
            ASSERT_TRUE(Pump([&]() {
                return hostGame->GetState() != EGameState::PlayersTurn ||
                       hostGame->GetCurrentPlayerIndex() != current;
            }));
        }
    }
    ASSERT_EQ(hostGame->GetState(), EGameState::RoundOver);

    ASSERT_TRUE(hostGame->StartRound());

    EXPECT_TRUE(Pump([&]() { return clientGame->GetState() == EGameState::Betting; }));
    EXPECT_TRUE(clientGame->GetPlayer(1)->GetHand().empty());
}

TEST_F(NetworkTest, PlayerLeavingDuringTheGameIsReportedToTheHost) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());

    peers[0]->client.reset();

    EXPECT_TRUE(Pump([&]() { return !leftName.empty(); }));
    EXPECT_EQ(leftName, "Ana");
}

TEST_F(NetworkTest, HostLeavingDuringTheGameIsReportedToPlayers) {
    ASSERT_NO_FATAL_FAILURE(StartGameWithOnePeer());
    hostGame.reset();

    host.reset();

    EXPECT_TRUE(Pump([&]() { return peers[0]->hostLeft; }));
}
