#include <audio/sound_player.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numbers>
#include <random>

namespace Frontend {
    namespace {
        constexpr unsigned kSampleRate = 44100;
        constexpr float kTwoPi = 2.f * std::numbers::pi_v<float>;
        // Sub atâtea secunde, același sunet nu se mai pornește încă o dată.
        constexpr float kRepeatWindow = 0.04f;

        SoundPlayer* g_activePlayer = nullptr;

        std::size_t indexOf(SoundId id) { return static_cast<std::size_t>(id); }

        // Umple `duration` secunde cu valorile date de `sample(t)` (între -1 și 1).
        std::vector<std::int16_t> synthesize(float duration,
                                             const std::function<float(float)>& sample) {
            const auto count = static_cast<std::size_t>(duration * kSampleRate);
            std::vector<std::int16_t> samples(count);
            for (std::size_t i = 0; i < count; ++i) {
                const float t = static_cast<float>(i) / kSampleRate;
                const float value = std::clamp(sample(t), -1.f, 1.f);
                samples[i] = static_cast<std::int16_t>(value * 32767.f);
            }
            return samples;
        }

        // Carte: un „fâșș” scurt de zgomot filtrat, ca hârtia care alunecă pe postav.
        std::vector<std::int16_t> cardSound() {
            std::mt19937 rng(7);
            std::uniform_real_distribution<float> noise(-1.f, 1.f);
            float low = 0.f;
            float previous = 0.f;
            return synthesize(0.12f, [&](float t) {
                // Filtru trece-bandă simplu: scoatem basul (diferență) și vârfurile (medie).
                const float white = noise(rng);
                const float high = white - previous;
                previous = white;
                low += 0.35f * (high - low);
                const float envelope = t < 0.006f ? t / 0.006f : std::exp(-(t - 0.006f) / 0.028f);
                return 0.9f * low * envelope;
            });
        }

        // Jeton: două „clinchete” scurte, ca un jeton pus peste altul.
        std::vector<std::int16_t> chipSound() {
            std::mt19937 rng(11);
            std::uniform_real_distribution<float> noise(-1.f, 1.f);
            const auto click = [&](float t) {
                if (t < 0.f) {
                    return 0.f;
                }
                const float tone = std::sin(kTwoPi * 2900.f * t) +
                                   0.6f * std::sin(kTwoPi * 4300.f * t) +
                                   0.4f * std::sin(kTwoPi * 6100.f * t);
                return 0.35f * tone * std::exp(-t / 0.012f) +
                       0.3f * noise(rng) * std::exp(-t / 0.003f);
            };
            return synthesize(
                0.13f, [&](float t) { return 0.45f * (click(t) + 0.6f * click(t - 0.04f)); });
        }

        // Câștig: un arpegiu scurt, do–mi–sol–do.
        std::vector<std::int16_t> winSound() {
            constexpr float kNotes[] = {523.25f, 659.25f, 783.99f, 1046.5f};
            constexpr float kNoteGap = 0.09f;
            return synthesize(0.95f, [&](float t) {
                float total = 0.f;
                for (std::size_t i = 0; i < std::size(kNotes); ++i) {
                    const float local = t - kNoteGap * static_cast<float>(i);
                    if (local < 0.f) {
                        continue;
                    }
                    const float phase = kTwoPi * kNotes[i] * local;
                    const float tone = std::sin(phase) + 0.3f * std::sin(2.f * phase) +
                                       0.1f * std::sin(3.f * phase);
                    const float envelope =
                        local < 0.005f ? local / 0.005f : std::exp(-(local - 0.005f) / 0.18f);
                    total += 0.2f * tone * envelope;
                }
                return total;
            });
        }
    }  // namespace

    SoundPlayer::SoundPlayer() {
        m_lastPlayed.fill(-1.f);
        g_activePlayer = this;
    }

    SoundPlayer::~SoundPlayer() {
        if (g_activePlayer == this) {
            g_activePlayer = nullptr;
        }
    }

    bool SoundPlayer::load() {
        const std::vector<std::int16_t> samples[kSoundCount] = {cardSound(), chipSound(),
                                                                winSound()};
        bool ok = true;
        for (std::size_t i = 0; i < kSoundCount; ++i) {
            if (!m_buffers[i].loadFromSamples(samples[i].data(), samples[i].size(), 1, kSampleRate,
                                              {sf::SoundChannel::Mono})) {
                std::cerr << "Nu am putut crea sunetul " << i << '\n';
                ok = false;
            }
        }

        m_voices.clear();
        m_voices.reserve(kVoices);
        for (std::size_t i = 0; i < kVoices; ++i) {
            m_voices.emplace_back(m_buffers[0]);
        }
        return ok;
    }

    void SoundPlayer::play(SoundId id) {
        const std::size_t index = indexOf(id);
        if (m_muted || index >= kSoundCount || m_voices.empty()) {
            return;
        }

        const float now = m_clock.getElapsedTime().asSeconds();
        if (m_lastPlayed[index] >= 0.f && now - m_lastPlayed[index] < kRepeatWindow) {
            return;
        }
        m_lastPlayed[index] = now;

        // O voce liberă; dacă toate sună, nu mai pornim încă un sunet.
        for (sf::Sound& voice : m_voices) {
            if (voice.getStatus() == sf::Sound::Status::Stopped) {
                voice.setBuffer(m_buffers[index]);
                voice.setVolume(m_volume);
                voice.play();
                return;
            }
        }
    }

    void SoundPlayer::setVolume(float volume) { m_volume = std::clamp(volume, 0.f, 100.f); }

    void SoundPlayer::setMuted(bool muted) {
        m_muted = muted;
        if (m_muted) {
            for (sf::Sound& voice : m_voices) {
                voice.stop();
            }
        }
    }

    bool SoundPlayer::isMuted() const { return m_muted; }

    void SoundPlayer::toggleMuted() { setMuted(!m_muted); }

    void playSound(SoundId id) {
        if (g_activePlayer) {
            g_activePlayer->play(id);
        }
    }
}  // namespace Frontend
