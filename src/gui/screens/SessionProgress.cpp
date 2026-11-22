#include "gui/screens/SessionProgress.h"

namespace vh {

void SessionProgress::record(const std::string& id, int stars) {
    for (int i = 0; i < ids.size(); i++) {
        if (ids[i] == id) {
            if (stars > values[i]) values[i] = stars;
            return;
        }
    }
    ids.push(id);
    values.push(stars);
}

int SessionProgress::starsFor(const std::string& id) const {
    for (int i = 0; i < ids.size(); i++) {
        if (ids[i] == id) return values[i];
    }
    return 0;
}

int SessionProgress::totalStars() const {
    int result = 0;
    for (int i = 0; i < values.size(); i++) result += values[i];
    return result;
}

void SessionProgress::clear() {
    ids.clear();
    values.clear();
}

}
