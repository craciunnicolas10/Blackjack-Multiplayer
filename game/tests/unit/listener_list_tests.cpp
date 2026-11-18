#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Backend/net/listener_list.h>

using namespace Backend;
using ::testing::NiceMock;

namespace {
    class Probe {
       public:
        virtual ~Probe() = default;
        virtual void Ping() = 0;
    };

    class MockProbe : public Probe {
       public:
        MOCK_METHOD(void, Ping, (), (override));
    };

    void PingAll(const ListenerList<Probe>& list) {
        list.Notify([](Probe& probe) { probe.Ping(); });
    }
}  // namespace

TEST(ListenerListTest, NotifiesAddedListeners) {
    ListenerList<Probe> list;
    auto first = std::make_shared<NiceMock<MockProbe>>();
    auto second = std::make_shared<NiceMock<MockProbe>>();
    list.Add(first);
    list.Add(second);
    EXPECT_CALL(*first, Ping()).Times(1);
    EXPECT_CALL(*second, Ping()).Times(1);

    PingAll(list);
}

TEST(ListenerListTest, AddingTheSameListenerTwiceNotifiesItOnce) {
    ListenerList<Probe> list;
    auto probe = std::make_shared<NiceMock<MockProbe>>();
    list.Add(probe);
    list.Add(probe);
    EXPECT_CALL(*probe, Ping()).Times(1);

    PingAll(list);
}

TEST(ListenerListTest, ExpiredListenerIsIgnoredWhenAdded) {
    ListenerList<Probe> list;
    std::weak_ptr<Probe> expired;
    {
        auto temporary = std::make_shared<NiceMock<MockProbe>>();
        expired = temporary;
    }

    list.Add(expired);

    EXPECT_NO_FATAL_FAILURE(PingAll(list));
}

TEST(ListenerListTest, ListenerThatExpiresLaterIsSkipped) {
    ListenerList<Probe> list;
    auto survivor = std::make_shared<NiceMock<MockProbe>>();
    {
        auto temporary = std::make_shared<NiceMock<MockProbe>>();
        list.Add(temporary);
    }
    list.Add(survivor);
    EXPECT_CALL(*survivor, Ping()).Times(1);

    PingAll(list);
}

TEST(ListenerListTest, RemovedListenerIsNotNotified) {
    ListenerList<Probe> list;
    auto removed = std::make_shared<NiceMock<MockProbe>>();
    auto kept = std::make_shared<NiceMock<MockProbe>>();
    list.Add(removed);
    list.Add(kept);
    EXPECT_CALL(*removed, Ping()).Times(0);
    EXPECT_CALL(*kept, Ping()).Times(1);

    list.Remove(removed.get());
    PingAll(list);
}

TEST(ListenerListTest, RemovingUnknownListenerChangesNothing) {
    ListenerList<Probe> list;
    auto probe = std::make_shared<NiceMock<MockProbe>>();
    auto stranger = std::make_shared<NiceMock<MockProbe>>();
    list.Add(probe);
    EXPECT_CALL(*probe, Ping()).Times(1);

    list.Remove(stranger.get());
    PingAll(list);
}

TEST(ListenerListTest, ListenerMayRemoveItselfDuringNotify) {
    ListenerList<Probe> list;
    auto probe = std::make_shared<NiceMock<MockProbe>>();
    list.Add(probe);
    EXPECT_CALL(*probe, Ping()).WillOnce([&]() { list.Remove(probe.get()); });

    PingAll(list);
    PingAll(list);
}

TEST(ListenerListTest, EmptyListNotifyDoesNothing) {
    ListenerList<Probe> list;

    EXPECT_NO_FATAL_FAILURE(PingAll(list));
}
