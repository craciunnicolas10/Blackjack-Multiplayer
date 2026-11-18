#include <ui/text_utils.h>

#include <cstddef>

namespace Frontend {
    sf::String trimmed(const sf::String& text) {
        std::size_t first = 0;
        std::size_t last = text.getSize();
        while (first < last && text[first] == U' ') {
            ++first;
        }
        while (last > first && text[last - 1] == U' ') {
            --last;
        }
        return text.substring(first, last - first);
    }
}  // namespace Frontend
