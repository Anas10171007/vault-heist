#pragma once

#include <string>

#include "util/DynArray.h"

namespace vh {

// Session-only stars, shared by the campaign map and the level screen.
class SessionProgress {
public:
    void record(const std::string& id, int stars);
    int starsFor(const std::string& id) const;
    int totalStars() const;
    void clear();

private:
    DynArray<std::string> ids;
    DynArray<int> values;
};

}
