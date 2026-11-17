#pragma once
#include <string>
#include "util/DynArray.h"

using namespace std;

namespace vh {

struct LevelData {
    string source;
    string tree;
    DynArray<int> values;
    string shape;
    int size;
    int minValue;
    int maxValue;
    int disturb;
    bool hasTree;
    bool hasValues;
    bool hasShape;

    LevelData()
        : size(0), minValue(1), maxValue(99), disturb(0),
          hasTree(false), hasValues(false), hasShape(false) {}
};

struct LevelRules {
    string traversal;
    string from;
    string rotation;
    int a;
    int b;
    int target;
    int lo;
    int hi;
    int k;
    int count;
    DynArray<int> inserts;

    LevelRules()
        : a(0), b(0), target(0), lo(0), hi(0), k(0), count(0) {}
};

struct LevelDef {
    string id;
    string title;
    string mission;
    string structure;
    string task;
    int seed;
    int act;
    LevelData data;
    LevelRules rules;

    LevelDef() : seed(0), act(0) {}
};

}
