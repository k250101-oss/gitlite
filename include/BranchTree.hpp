#ifndef BRANCHTREE_HPP
#define BRANCHTREE_HPP

#include <string>
#include <iostream>
#include "SafeArray.hpp"

/**
 * @brief Node in the N-ary Branch Tree.
 * Represents a branch, its fork point (commit ID), and its parent branch.
 */
struct BranchNode {
    std::string branchName;
    std::string parentBranch;
    int forkCommitId;
    SafeArray<BranchNode*> children;

    BranchNode(const std::string& name, const std::string& parent = "", int forkId = 0)
        : branchName(name), parentBranch(parent), forkCommitId(forkId) {}

    ~BranchNode() {
        for (size_t i = 0; i < children.size(); ++i) {
            delete children[i];
        }
    }

    // Disallow simple copy to avoid double free; deep copy implemented in BranchTree
    BranchNode(const BranchNode&) = delete;
    BranchNode& operator=(const BranchNode&) = delete;
};

/**
 * @brief N-ary Tree modeling the branching topology in GitLite.
 * Provides DFS indentation display and BFS level-order traversal.
 * Strictly implements the Rule of Five.
 */
class BranchTree {
private:
    BranchNode* root_;

    BranchNode* findInternal(BranchNode* current, const std::string& name) const {
        if (!current) return nullptr;
        if (current->branchName == name) return current;
        for (size_t i = 0; i < current->children.size(); ++i) {
            BranchNode* found = findInternal(current->children[i], name);
            if (found) return found;
        }
        return nullptr;
    }

    BranchNode* cloneSubtree(const BranchNode* source) {
        if (!source) return nullptr;
        BranchNode* copy = new BranchNode(source->branchName, source->parentBranch, source->forkCommitId);
        for (size_t i = 0; i < source->children.size(); ++i) {
            copy->children.push_back(cloneSubtree(source->children[i]));
        }
        return copy;
    }

    void printDFSHelper(BranchNode* node, const std::string& currentBranch, int depth, std::ostream& os) const {
        if (!node) return;
        for (int i = 0; i < depth; ++i) {
            os << "    ";
        }
        if (depth > 0) {
            os << "|-- ";
        }
        os << node->branchName;
        if (node->branchName == currentBranch) {
            os << "  <- [CURRENT HEAD]";
        }
        if (node->forkCommitId > 0) {
            os << " (forked from " << node->parentBranch << " at commit #" << node->forkCommitId << ")";
        } else {
            os << " (root branch)";
        }
        os << "\n";

        for (size_t i = 0; i < node->children.size(); ++i) {
            printDFSHelper(node->children[i], currentBranch, depth + 1, os);
        }
    }

public:
    explicit BranchTree(const std::string& rootBranchName = "main") {
        root_ = new BranchNode(rootBranchName, "", 0);
    }

    ~BranchTree() {
        delete root_;
        root_ = nullptr;
    }

    // Copy Constructor (Deep copy of N-ary tree)
    BranchTree(const BranchTree& other) : root_(nullptr) {
        if (other.root_) {
            root_ = cloneSubtree(other.root_);
        }
    }

    // Copy Assignment Operator
    BranchTree& operator=(const BranchTree& other) {
        if (this != &other) {
            delete root_;
            root_ = nullptr;
            if (other.root_) {
                root_ = cloneSubtree(other.root_);
            }
        }
        return *this;
    }

    // Move Constructor
    BranchTree(BranchTree&& other) noexcept : root_(other.root_) {
        other.root_ = nullptr;
    }

    // Move Assignment
    BranchTree& operator=(BranchTree&& other) noexcept {
        if (this != &other) {
            delete root_;
            root_ = other.root_;
            other.root_ = nullptr;
        }
        return *this;
    }

    BranchNode* root() const { return root_; }

    BranchNode* find(const std::string& branchName) const {
        return findInternal(root_, branchName);
    }

    bool contains(const std::string& branchName) const {
        return find(branchName) != nullptr;
    }

    bool addBranch(const std::string& parentName, const std::string& newBranchName, int forkCommitId) {
        if (contains(newBranchName)) {
            return false; // Already exists
        }
        BranchNode* parentNode = find(parentName);
        if (!parentNode) {
            if (!root_) {
                root_ = new BranchNode(newBranchName, "", forkCommitId);
                return true;
            }
            return false; // Parent not found
        }
        BranchNode* newNode = new BranchNode(newBranchName, parentName, forkCommitId);
        parentNode->children.push_back(newNode);
        return true;
    }

    /**
     * @brief Traverses the branch tree using Depth-First Search (DFS) with visual indentation.
     */
    void printDFS(const std::string& activeBranch = "", std::ostream& os = std::cout) const {
        os << "=== Branch Hierarchy (DFS Tree View) ===\n";
        if (!root_) {
            os << "No branches found.\n";
            return;
        }
        printDFSHelper(root_, activeBranch, 0, os);
    }

    /**
     * @brief Traverses the branch tree using Breadth-First Search (BFS) level by level.
     */
    void printBFS(const std::string& activeBranch = "", std::ostream& os = std::cout) const {
        os << "=== Branch Topology (BFS Level-by-Level View) ===\n";
        if (!root_) {
            os << "No branches found.\n";
            return;
        }

        // BFS using dynamic array queue
        SafeArray<BranchNode*> queue;
        SafeArray<int> levels;
        queue.push_back(root_);
        levels.push_back(0);

        size_t frontIdx = 0;
        int currentLevel = -1;

        while (frontIdx < queue.size()) {
            BranchNode* curr = queue[frontIdx];
            int lvl = levels[frontIdx];
            ++frontIdx;

            if (lvl != currentLevel) {
                currentLevel = lvl;
                os << "\nLevel " << currentLevel << ":\n";
            }

            os << "  * " << curr->branchName;
            if (curr->branchName == activeBranch) {
                os << " [ACTIVE]";
            }
            if (!curr->parentBranch.empty()) {
                os << " (parent: " << curr->parentBranch << ", fork commit #" << curr->forkCommitId << ")";
            } else {
                os << " (root)";
            }
            os << "\n";

            for (size_t i = 0; i < curr->children.size(); ++i) {
                queue.push_back(curr->children[i]);
                levels.push_back(currentLevel + 1);
            }
        }
        os << "\n";
    }

    // Collect all branch names into SafeArray
    void getAllBranchNames(SafeArray<std::string>& list) const {
        if (!root_) return;
        SafeArray<BranchNode*> q;
        q.push_back(root_);
        size_t idx = 0;
        while (idx < q.size()) {
            BranchNode* node = q[idx++];
            list.push_back(node->branchName);
            for (size_t i = 0; i < node->children.size(); ++i) {
                q.push_back(node->children[i]);
            }
        }
    }
};

#endif // BRANCHTREE_HPP
