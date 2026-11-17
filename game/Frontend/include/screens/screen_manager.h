#pragma once

#include <memory>

#include <screens/screen.h>

namespace Frontend {
    // Ține ecranul curent. Schimbarea se aplică abia la sfârșitul cadrului, ca un ecran
    // să poată cere trecerea la altul din propriul handleEvent fără să se distrugă singur.
    class ScreenManager {
       public:
        void change(std::unique_ptr<Screen> next);
        void applyPending();

        void requestQuit();
        bool quitRequested() const;

        Screen* current() const;

       private:
        std::unique_ptr<Screen> m_current;
        std::unique_ptr<Screen> m_pending;
        bool m_quitRequested = false;
    };
}  // namespace Frontend
