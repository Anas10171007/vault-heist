#pragma once

#include <string>

#include "model/tree/BinaryTree.h"
#include "util/DynArray.h"

namespace vh {

struct TreeProblemCode {
    static constexpr int ORDER = 1, HEIGHT = 2, SIZE = 3, BALANCE = 4;
    static constexpr int PARENT = 5, CYCLE = 6, SHARED = 7, VALUE = 8;
};

struct TreeProblem {
    int code = 0;
    int nodeId = 0;
    std::string message;

    TreeProblem() = default;
    TreeProblem(int problemCode, int id, const std::string& text)
        : code(problemCode), nodeId(id), message(text) {}
};

struct TreeValidationResult {
    DynArray<TreeProblem> problems;

    bool ok() const { return problems.empty(); }
    void add(int code, int nodeId, const std::string& message) {
        problems.push(TreeProblem(code, nodeId, message));
    }
};

class TreeValidator {
public:
    static TreeValidationResult validate(const BinaryTree& tree);
    static TreeValidationResult validateTree(const BinaryTree& tree);
    static TreeValidationResult validateTree(const TreeNode* root, bool checkOrdering, bool checkAVL);
};

}
