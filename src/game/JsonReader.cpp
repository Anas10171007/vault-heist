#include "game/JsonReader.h"

namespace vh {

JsonReader::JsonReader(const string& input) : text(input), position(0) {}

void JsonReader::skipWhitespace() {
    while (position < static_cast<int>(text.size())) {
        char c = text[position];
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') break;
        position++;
    }
}

bool JsonReader::fail(const string& message) {
    if (errorMessage.empty()) {
        errorMessage = message + " at character " + to_string(position);
    }
    return false;
}

bool JsonReader::consume(char expected) {
    skipWhitespace();
    if (position >= static_cast<int>(text.size()) || text[position] != expected) {
        return fail(string("expected '") + expected + "'");
    }
    position++;
    return true;
}

bool JsonReader::nextIs(char expected) {
    skipWhitespace();
    return position < static_cast<int>(text.size()) && text[position] == expected;
}

bool JsonReader::readString(string& value) {
    skipWhitespace();
    if (position >= static_cast<int>(text.size()) || text[position] != '"') {
        return fail("expected a string");
    }
    position++;
    value.clear();

    while (position < static_cast<int>(text.size()) && text[position] != '"') {
        if (text[position] == '\\') return fail("backslashes are not supported in strings");
        value += text[position];
        position++;
    }

    if (position >= static_cast<int>(text.size())) return fail("unfinished string");
    position++;
    return true;
}

bool JsonReader::readInt(int& value) {
    skipWhitespace();
    bool negative = false;
    if (position < static_cast<int>(text.size()) && text[position] == '-') {
        negative = true;
        position++;
    }
    if (position >= static_cast<int>(text.size()) || text[position] < '0' || text[position] > '9') {
        return fail("expected a whole number");
    }

    value = 0;
    while (position < static_cast<int>(text.size()) && text[position] >= '0' && text[position] <= '9') {
        value = value * 10 + (text[position] - '0');
        position++;
    }
    if (negative) value = -value;
    return true;
}

bool JsonReader::readBool(bool& value) {
    skipWhitespace();
    if (text.compare(position, 4, "true") == 0) {
        value = true;
        position += 4;
        return true;
    }
    if (text.compare(position, 5, "false") == 0) {
        value = false;
        position += 5;
        return true;
    }
    return fail("expected true or false");
}

bool JsonReader::skipValue() {
    skipWhitespace();
    if (position >= static_cast<int>(text.size())) return fail("expected a value");

    if (text[position] == '"') {
        string ignored;
        return readString(ignored);
    }
    if (text[position] == '{') {
        if (!consume('{')) return false;
        if (nextIs('}')) return consume('}');
        while (true) {
            string key;
            if (!readString(key) || !consume(':') || !skipValue()) return false;
            if (nextIs('}')) return consume('}');
            if (!consume(',')) return false;
        }
    }
    if (text[position] == '[') {
        if (!consume('[')) return false;
        if (nextIs(']')) return consume(']');
        while (true) {
            if (!skipValue()) return false;
            if (nextIs(']')) return consume(']');
            if (!consume(',')) return false;
        }
    }
    if (text[position] == 't' || text[position] == 'f') {
        bool ignored = false;
        return readBool(ignored);
    }
    if (text[position] == '-' || (text[position] >= '0' && text[position] <= '9')) {
        int ignored = 0;
        return readInt(ignored);
    }
    return fail("unsupported JSON value");
}

bool JsonReader::finished() {
    skipWhitespace();
    return position == static_cast<int>(text.size());
}

const string& JsonReader::error() const {
    return errorMessage;
}

}
