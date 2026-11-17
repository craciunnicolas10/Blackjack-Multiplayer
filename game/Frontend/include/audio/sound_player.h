#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Clock.hpp>

namespace Frontend {
    enum class SoundId { Card, Chip, Win };

    // Sunetele jocului. Nu folosim fișiere audio: sunetele sunt generate din cod la pornire
    // (zgomot și tonuri sintetizate), deci nu avem nicio problemă de licență.
    // Application deține un SoundPlayer; restul codului cântă prin playSound(), care nu face
    // nimic dacă nu există niciun SoundPlayer (ex. în teste).
    class SoundPlayer {
       public:
        SoundPlayer();
        ~SoundPlayer();
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer& operator=(const SoundPlayer&) = delete;

        // Generează sunetele. Întoarce false dacă vreunul nu a putut fi creat.
        bool load();

        void play(SoundId id);
        void setVolume(float volume);  // 0–100

       private:
        static constexpr std::size_t kSoundCount = 3;
        static constexpr std::size_t kVoices = 16;  // câte sunete pot suna în același timp

        // Declarate înaintea vocilor: vocile țin referințe la ele, deci se distrug primele.
        std::array<sf::SoundBuffer, kSoundCount> m_buffers;
        std::vector<sf::Sound> m_voices;
        // Același sunet cerut de mai multe ori în același moment (ex. doi jucători câștigă
        // deodată) se aude o singură dată, nu de două ori mai tare.
        std::array<float, kSoundCount> m_lastPlayed{};
        sf::Clock m_clock;
        float m_volume = 70.f;
    };

    // Cântă prin SoundPlayer-ul aplicației, dacă există.
    void playSound(SoundId id);
}  // namespace Frontend
