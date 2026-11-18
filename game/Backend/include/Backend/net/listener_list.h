#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace Backend {
    // Lista de ascultatori a unui Observer: ii tine ca weak_ptr (nu ii tine in viata) si ii
    // anunta pe cei care mai exista.
    template <typename Listener>
    class ListenerList {
       public:
        void Add(std::weak_ptr<Listener> listener) {
            const auto added = listener.lock();
            if (!added) {
                return;
            }
            std::erase_if(m_listeners, [&](const auto& existing) {
                const auto locked = existing.lock();
                return !locked || locked == added;
            });
            m_listeners.push_back(std::move(listener));
        }

        void Remove(Listener* listener) {
            std::erase_if(m_listeners, [listener](const auto& existing) {
                const auto locked = existing.lock();
                return !locked || locked.get() == listener;
            });
        }

        // Pe o copie, ca un ascultator sa se poata scoate singur in timpul anuntului.
        template <typename Callback>
        void Notify(Callback&& callback) const {
            const std::vector<std::weak_ptr<Listener>> listeners = m_listeners;
            for (const auto& weakListener : listeners) {
                if (const auto listener = weakListener.lock()) {
                    callback(*listener);
                }
            }
        }

       private:
        std::vector<std::weak_ptr<Listener>> m_listeners;
    };
}  // namespace Backend
