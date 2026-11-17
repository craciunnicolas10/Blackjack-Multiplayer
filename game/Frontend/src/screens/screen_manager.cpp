#include <screens/screen_manager.h>

#include <utility>

namespace Frontend {
    void ScreenManager::change(std::unique_ptr<Screen> next) { m_pending = std::move(next); }

    void ScreenManager::applyPending() {
        if (m_pending) {
            m_current = std::move(m_pending);
        }
    }

    void ScreenManager::requestQuit() { m_quitRequested = true; }

    bool ScreenManager::quitRequested() const { return m_quitRequested; }

    Screen* ScreenManager::current() const { return m_current.get(); }
}  // namespace Frontend
