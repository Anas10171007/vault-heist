#pragma once
#include <string>

using namespace std;

namespace vh {

class JsonReader {
public:
    JsonReader(const string& input);

    bool consume(char expected);
    bool nextIs(char expected);
    bool readString(string& value);
    bool readInt(int& value);
    bool readBool(bool& value);
    bool skipValue();
    bool finished();

    const string& error() const;

private:
    string text;
    int position;
    string errorMessage;

    void skipWhitespace();
    bool fail(const string& message);
};

}
