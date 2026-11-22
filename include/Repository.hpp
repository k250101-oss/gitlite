#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "SafeArray.hpp"
#include "DoublyLinkedList.hpp"
#include "Stack.hpp"
#include "HashTable.hpp"
#include "BranchTree.hpp"
#include "Commit.hpp"
#include "DiffEngine.hpp"
#include "SearchEngine.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;

/**
 * @brief Snapshot of repository state used for Undo and Redo operations.
 */
struct UndoState {
    std::string actionName;
    std::string branch;
    int commitId;
    SafeArray<std::string> workingFileLines;

    UndoState() : actionName(""), branch("main"), commitId(0) {}
    UndoState(const std::string& act, const std::string& b, int cid, const SafeArray<std::string>& lines)
        : actionName(act), branch(b), commitId(cid), workingFileLines(lines) {}
};

/**
 * @brief Core GitLite Repository Engine.
 * Coordinates custom data structures:
 *  - SafeArray for lines and collections
 *  - DoublyLinkedList for linear commit histories per branch
 *  - Stack for undo and redo
 *  - HashTable for branch name resolution and commit lookup
 *  - BranchTree (N-ary tree) for branch topology
 *  - DiffEngine for LCS diff
 *  - SearchEngine for KMP and MergeSort
 */
class Repository {
private:
    std::string repoPath_;
    std::string gitliteDir_;
    std::string commitsDir_;
    std::string trackedFile_;
    std::string currentBranch_;
    std::string author_;
    int nextCommitId_;

    HashTable<std::string, DoublyLinkedList<Commit>> branchHistories_;
    BranchTree branchTree_;
    Stack<UndoState> undoStack_;
    Stack<UndoState> redoStack_;

    bool isInitialized_;

    // Helper: Saves repository metadata to .gitlite/config.txt
    void saveConfig() {
        std::ofstream fout(gitliteDir_ + "/config.txt");
        if (fout.is_open()) {
            fout << "TRACKED_FILE=" << trackedFile_ << "\n";
            fout << "CURRENT_BRANCH=" << currentBranch_ << "\n";
            fout << "AUTHOR=" << author_ << "\n";
            fout << "NEXT_COMMIT_ID=" << nextCommitId_ << "\n";
        }
    }

    // Helper: Saves branch tree structure to .gitlite/branches.txt
    void saveBranches() {
        std::ofstream fout(gitliteDir_ + "/branches.txt");
        if (!fout.is_open()) return;

        SafeArray<std::string> branches;
        branchTree_.getAllBranchNames(branches);
        for (size_t i = 0; i < branches.size(); ++i) {
            BranchNode* node = branchTree_.find(branches[i]);
            if (node) {
                fout << node->branchName << "," << node->parentBranch << "," << node->forkCommitId << "\n";
            }
        }
    }

    void saveStackToFile(const Stack<UndoState>& st, const std::string& filepath) {
        std::ofstream fout(filepath);
        if (!fout.is_open()) return;
        fout << st.size() << "\n";
        for (size_t i = 0; i < st.size(); ++i) {
            const UndoState& u = st[i];
            fout << "ACTION=" << u.actionName << "\n";
            fout << "BRANCH=" << u.branch << "\n";
            fout << "COMMIT_ID=" << u.commitId << "\n";
            fout << "LINES=" << u.workingFileLines.size() << "\n";
            for (size_t l = 0; l < u.workingFileLines.size(); ++l) {
                fout << u.workingFileLines[l] << "\n";
            }
        }
    }

    void loadStackFromFile(Stack<UndoState>& st, const std::string& filepath) {
        st.clear();
        std::ifstream fin(filepath);
        if (!fin.is_open()) return;
        std::string line;
        if (!std::getline(fin, line)) return;
        if (line.empty()) return;
        size_t count = 0;
        try { count = std::stoul(line); } catch (...) { return; }
        for (size_t i = 0; i < count; ++i) {
            UndoState u;
            size_t numLines = 0;
            for (int k = 0; k < 4; ++k) {
                if (!std::getline(fin, line)) break;
                if (!line.empty() && line.back() == '\r') line.pop_back();
                size_t eqPos = line.find('=');
                if (eqPos == std::string::npos) continue;
                std::string key = line.substr(0, eqPos);
                std::string val = line.substr(eqPos + 1);
                if (key == "ACTION") u.actionName = val;
                else if (key == "BRANCH") u.branch = val;
                else if (key == "COMMIT_ID") u.commitId = std::stoi(val);
                else if (key == "LINES") numLines = std::stoul(val);
            }
            for (size_t l = 0; l < numLines; ++l) {
                if (std::getline(fin, line)) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    u.workingFileLines.push_back(line);
                }
            }
            st.push(u);
        }
    }

    void saveStacks() {
        saveStackToFile(undoStack_, gitliteDir_ + "/undo.txt");
        saveStackToFile(redoStack_, gitliteDir_ + "/redo.txt");
    }

    void loadStacks() {
        loadStackFromFile(undoStack_, gitliteDir_ + "/undo.txt");
        loadStackFromFile(redoStack_, gitliteDir_ + "/redo.txt");
    }

public:
    Repository(const std::string& rootPath = ".")
        : repoPath_(rootPath),
          gitliteDir_(rootPath + "/.gitlite"),
          commitsDir_(rootPath + "/.gitlite/commits"),
          trackedFile_(""),
          currentBranch_("main"),
          author_("Adeel, Yahya & Asmaar"),
          nextCommitId_(1),
          branchHistories_(16),
          branchTree_("main"),
          isInitialized_(false) {
        loadRepository();
    }

    bool isInitialized() const noexcept { return isInitialized_; }
    const std::string& getTrackedFile() const noexcept { return trackedFile_; }
    const std::string& getCurrentBranch() const noexcept { return currentBranch_; }
    const std::string& getAuthor() const noexcept { return author_; }

    bool setAuthor(const std::string& newAuthor) {
        if (newAuthor.empty()) return false;
        author_ = newAuthor;
        saveConfig();
        std::cout << Utils::Color::GREEN << "[SUCCESS] Default author updated to: \"" << author_ << "\""
                  << Utils::Color::RESET << "\n";
        return true;
    }

    const HashTable<std::string, DoublyLinkedList<Commit>>& getBranchHistories() const { return branchHistories_; }
    const BranchTree& getBranchTree() const { return branchTree_; }

    /**
     * @brief Initializes a new GitLite repository for the specified file.
     */
    bool init(const std::string& filename) {
        if (fs::exists(gitliteDir_)) {
            std::cout << Utils::Color::YELLOW << "GitLite repository already initialized in this directory."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        try {
            fs::create_directories(commitsDir_);
        } catch (const std::exception& e) {
            std::cerr << Utils::Color::RED << "Error creating .gitlite directory: " << e.what()
                      << Utils::Color::RESET << "\n";
            return false;
        }

        trackedFile_ = filename;
        currentBranch_ = "main";
        nextCommitId_ = 1;
        isInitialized_ = true;

        branchHistories_.clear();
        branchHistories_.insert("main", DoublyLinkedList<Commit>());
        branchTree_ = BranchTree("main");

        saveConfig();
        saveBranches();

        // If the tracked file doesn't exist, create an initial placeholder file
        std::string fullFilePath = repoPath_ + "/" + trackedFile_;
        if (!fs::exists(fullFilePath)) {
            std::ofstream fout(fullFilePath);
            fout << "# " << trackedFile_ << "\nInitial draft.\n";
            fout.close();
        }

        std::cout << Utils::Color::GREEN << "Initialized empty GitLite repository tracking '" << trackedFile_ << "'."
                  << Utils::Color::RESET << "\n";
        return true;
    }

    /**
     * @brief Deletes the GitLite repository by removing the hidden .gitlite directory.
     * Keeps the tracked file intact by default.
     */
    bool deleteRepo(bool keepTrackedFile = true) {
        if (!fs::exists(gitliteDir_)) {
            std::cout << Utils::Color::YELLOW << "No GitLite repository found in this directory (.gitlite folder does not exist)."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        try {
            fs::remove_all(gitliteDir_);
            isInitialized_ = false;
            std::string oldTracked = trackedFile_;
            trackedFile_.clear();
            currentBranch_ = "main";
            nextCommitId_ = 1;
            branchHistories_.clear();
            undoStack_.clear();
            redoStack_.clear();

            std::cout << Utils::Color::GREEN << "[SUCCESS] GitLite repository deleted (.gitlite directory removed)."
                      << Utils::Color::RESET << "\n";
            if (keepTrackedFile && !oldTracked.empty()) {
                std::cout << "Your tracked file '" << oldTracked << "' was preserved.\n";
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << Utils::Color::RED << "[ERROR] Failed to delete repository: " << e.what()
                      << Utils::Color::RESET << "\n";
            return false;
        }
    }

    /**
     * @brief Loads an existing repository from .gitlite/
     */
    bool loadRepository() {
        if (!fs::exists(gitliteDir_) || !fs::exists(gitliteDir_ + "/config.txt")) {
            isInitialized_ = false;
            return false;
        }

        // Read config
        std::ifstream fin(gitliteDir_ + "/config.txt");
        std::string line;
        while (std::getline(fin, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;
            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);
            if (key == "TRACKED_FILE") trackedFile_ = val;
            else if (key == "CURRENT_BRANCH") currentBranch_ = val;
            else if (key == "AUTHOR") author_ = val;
            else if (key == "NEXT_COMMIT_ID") nextCommitId_ = std::stoi(val);
        }
        fin.close();

        // Read branch tree
        std::ifstream bfin(gitliteDir_ + "/branches.txt");
        if (bfin.is_open()) {
            std::string bline;
            bool first = true;
            while (std::getline(bfin, bline)) {
                if (!bline.empty() && bline.back() == '\r') bline.pop_back();
                if (bline.empty()) continue;
                std::stringstream ss(bline);
                std::string bname, parent;
                std::string forkStr;
                std::getline(ss, bname, ',');
                std::getline(ss, parent, ',');
                std::getline(ss, forkStr, ',');
                int forkId = forkStr.empty() ? 0 : std::stoi(forkStr);

                if (first) {
                    branchTree_ = BranchTree(bname);
                    first = false;
                } else {
                    branchTree_.addBranch(parent, bname, forkId);
                }
                if (!branchHistories_.contains(bname)) {
                    branchHistories_.insert(bname, DoublyLinkedList<Commit>());
                }
            }
            bfin.close();
        }

        // Load all commits from commitsDir_
        if (fs::exists(commitsDir_)) {
            SafeArray<Commit> loadedCommits;
            for (const auto& entry : fs::directory_iterator(commitsDir_)) {
                if (entry.is_regular_file()) {
                    Commit c;
                    if (c.loadFromFile(entry.path().string())) {
                        loadedCommits.push_back(c);
                    }
                }
            }

            // Order commits by commit ID before adding to doubly linked lists
            SearchEngine::mergeSortCommits(loadedCommits);

            for (size_t i = 0; i < loadedCommits.size(); ++i) {
                const Commit& c = loadedCommits[i];
                DoublyLinkedList<Commit>* list = branchHistories_.find(c.getBranch());
                if (!list) {
                    branchHistories_.insert(c.getBranch(), DoublyLinkedList<Commit>());
                    list = branchHistories_.find(c.getBranch());
                }
                list->push_back(c);
            }
        }

        loadStacks();
        isInitialized_ = true;
        return true;
    }

    /**
     * @brief Finds a commit by ID across all branches.
     */
    Commit* findCommitById(int commitId) {
        SafeArray<std::string> branches;
        branchTree_.getAllBranchNames(branches);

        for (size_t b = 0; b < branches.size(); ++b) {
            DoublyLinkedList<Commit>* list = branchHistories_.find(branches[b]);
            if (list) {
                auto curr = list->head();
                while (curr != nullptr) {
                    if (curr->data.getId() == commitId) {
                        return &(curr->data);
                    }
                    curr = curr->next;
                }
            }
        }
        return nullptr;
    }

    /**
     * @brief Shows repository status (modified/clean/untracked).
     */
    void status() {
        if (!isInitialized_) {
            std::cout << Utils::Color::RED << "[ERROR] Not a gitlite repository (run 'gitlite init <filename>' first)."
                      << Utils::Color::RESET << "\n";
            return;
        }

        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "GitLite Status | Branch: " << Utils::Color::CYAN << currentBranch_ << Utils::Color::RESET
                  << " | Tracking: " << Utils::Color::YELLOW << trackedFile_ << Utils::Color::RESET << "\n";
        std::cout << "----------------------------------------------------------------------\n";

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        if (!fs::exists(fullPath)) {
            std::cout << Utils::Color::RED << "[ALERT] Tracked file '" << trackedFile_ << "' is missing from the working directory!"
                      << Utils::Color::RESET << "\n";
            std::cout << "----------------------------------------------------------------------\n";
            return;
        }

        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);
        std::string currentHash = Utils::computeContentHash(currentLines);

        DoublyLinkedList<Commit>* list = branchHistories_.find(currentBranch_);
        if (!list || list->empty()) {
            std::cout << Utils::Color::YELLOW << "Status: Initial State (No commits yet on this branch)\n"
                      << "Working tree has untracked lines ready for commit ("
                      << currentLines.size() << " lines).\n\n"
                      << "Next step: Run 'gitlite commit <message>' to save the first snapshot."
                      << Utils::Color::RESET << "\n";
            std::cout << "----------------------------------------------------------------------\n";
            return;
        }

        const Commit& headCommit = list->back();
        if (headCommit.getHash() == currentHash) {
            std::cout << Utils::Color::GREEN << "Status: Working Tree Clean\n"
                      << "No changes detected since commit #" << headCommit.getId()
                      << " (\"" << headCommit.getMessage() << "\")."
                      << Utils::Color::RESET << "\n";
        } else {
            std::cout << Utils::Color::YELLOW << "Status: Changes Detected (Uncommitted)\n"
                      << "  modified: " << trackedFile_ << "\n\n"
                      << "Commands to proceed:\n"
                      << "  * View changes:   gitlite diff\n"
                      << "  * Commit changes: gitlite commit <message>"
                      << Utils::Color::RESET << "\n";
        }
        std::cout << "----------------------------------------------------------------------\n";
    }

    /**
     * @brief Commits current working file snapshot to repository.
     * Prevents duplicate commits by checking content hash.
     */
    bool commit(const std::string& message) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return false;
        }
        if (message.empty()) {
            std::cerr << Utils::Color::RED << "error: commit message cannot be empty." << Utils::Color::RESET << "\n";
            return false;
        }

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        if (!fs::exists(fullPath)) {
            std::cerr << Utils::Color::RED << "fatal: tracked file '" << trackedFile_ << "' does not exist."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        SafeArray<std::string> lines;
        Utils::readFileLines(fullPath, lines);
        std::string contentHash = Utils::computeContentHash(lines);

        DoublyLinkedList<Commit>* list = branchHistories_.find(currentBranch_);
        if (!list) {
            branchHistories_.insert(currentBranch_, DoublyLinkedList<Commit>());
            list = branchHistories_.find(currentBranch_);
        }

        int parentId = 0;
        if (!list->empty()) {
            const Commit& headCommit = list->back();
            // Fast hash comparison to detect duplicate commit
            if (headCommit.getHash() == contentHash) {
                // Secondary check: confirm line equality
                bool identical = (headCommit.getLines().size() == lines.size());
                if (identical) {
                    for (size_t i = 0; i < lines.size(); ++i) {
                        if (lines[i] != headCommit.getLines()[i]) {
                            identical = false;
                            break;
                        }
                    }
                }
                if (identical) {
                    std::cout << Utils::Color::YELLOW
                              << "Nothing to commit: '" << trackedFile_ << "' has not changed since commit #"
                              << headCommit.getId() << ".\n"
                              << Utils::Color::RESET
                              << "  Edit '" << trackedFile_ << "' first, then run 'gitlite commit \"<message>\"'.\n"
                              << "  Tip: The commit message does NOT change the file — you must edit the file itself.\n";
                    return false;
                }
            }
            parentId = headCommit.getId();
        }

        int newId = nextCommitId_++;
        Commit newCommit(newId, currentBranch_, parentId, message, lines, author_);

        // Save commit to .gitlite/commits/commit_<id>.txt
        std::string commitFilePath = commitsDir_ + "/commit_" + std::to_string(newId) + ".txt";
        newCommit.saveToFile(commitFilePath);

        // Append to current branch's DoublyLinkedList
        list->push_back(newCommit);

        // Update config
        saveConfig();

        // Clear redo stack on new commit
        redoStack_.clear();
        saveStacks();

        std::cout << Utils::Color::GREEN << "[" << currentBranch_ << " (commit #" << newId << ")] "
                  << message << Utils::Color::RESET << "\n";
        std::cout << " Author: " << author_ << " | Hash: " << contentHash << " | Lines: " << lines.size() << "\n";
        return true;
    }

    /**
     * @brief Displays chronological log of commits on current branch or across all branches.
     */
    void log(bool showAll = false) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return;
        }

        if (showAll) {
            std::cout << "=== Commit History Across All Branches (Merge-Sorted Chronologically) ===\n\n";
            SafeArray<Commit> allCommits;
            SafeArray<std::string> branches;
            branchTree_.getAllBranchNames(branches);

            for (size_t b = 0; b < branches.size(); ++b) {
                DoublyLinkedList<Commit>* list = branchHistories_.find(branches[b]);
                if (list) {
                    auto curr = list->head();
                    while (curr != nullptr) {
                        allCommits.push_back(curr->data);
                        curr = curr->next;
                    }
                }
            }

            if (allCommits.empty()) {
                std::cout << "No commits recorded in repository.\n";
                return;
            }

            // Use our custom stable Merge Sort to sort all commits by timestamp/ID
            SearchEngine::mergeSortCommits(allCommits);

            for (size_t i = allCommits.size(); i > 0; --i) {
                const Commit& c = allCommits[i - 1];
                std::cout << Utils::Color::YELLOW << "commit " << c.getId() << Utils::Color::RESET;
                if (c.getBranch() == currentBranch_) {
                    std::cout << " (" << Utils::Color::CYAN << "HEAD -> " << c.getBranch() << Utils::Color::RESET << ")";
                } else {
                    std::cout << " (" << Utils::Color::MAGENTA << c.getBranch() << Utils::Color::RESET << ")";
                }
                std::cout << "\nAuthor: " << c.getAuthor() << "\n";
                std::cout << "Date:   " << c.getTimestamp() << "\n";
                std::cout << "Hash:   " << c.getHash() << "\n\n";
                std::cout << "    " << c.getMessage() << "\n\n";
            }
        } else {
            DoublyLinkedList<Commit>* list = branchHistories_.find(currentBranch_);
            if (!list || list->empty()) {
                std::cout << "No commits yet on branch '" << currentBranch_ << "'.\n";
                return;
            }

            std::cout << "=== Commit History on branch '" << currentBranch_ << "' ===\n\n";
            // Print commits in reverse chronological order (HEAD first) using DoublyLinkedList backward traversal
            auto curr = list->tail();
            bool isHead = true;
            while (curr != nullptr) {
                const Commit& c = curr->data;
                std::cout << Utils::Color::YELLOW << "commit " << c.getId() << Utils::Color::RESET;
                if (isHead) {
                    std::cout << " (" << Utils::Color::CYAN << "HEAD -> " << currentBranch_ << Utils::Color::RESET << ")";
                    isHead = false;
                }
                std::cout << "\nAuthor: " << c.getAuthor() << "\n";
                std::cout << "Date:   " << c.getTimestamp() << "\n";
                std::cout << "Hash:   " << c.getHash() << "\n\n";
                std::cout << "    " << c.getMessage() << "\n\n";

                curr = curr->prev;
            }
        }
    }

    /**
     * @brief Computes diff between two commits, or between a commit and the current working file.
     */
    void diff(int commitId1, int commitId2 = -1) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return;
        }

        SafeArray<std::string> lines1;
        SafeArray<std::string> lines2;
        std::string label1, label2;

        if (commitId1 == 0 && commitId2 == -1) {
            // Default: diff between latest HEAD commit and working tree
            DoublyLinkedList<Commit>* list = branchHistories_.find(currentBranch_);
            if (!list || list->empty()) {
                std::cerr << "No commits to diff against.\n";
                return;
            }
            const Commit& head = list->back();
            lines1 = head.getLines();
            label1 = "commit #" + std::to_string(head.getId());

            std::string fullPath = repoPath_ + "/" + trackedFile_;
            Utils::readFileLines(fullPath, lines2);
            label2 = "working file (" + trackedFile_ + ")";
        } else if (commitId2 == -1) {
            // Diff between commitId1 and working tree
            Commit* c1 = findCommitById(commitId1);
            if (!c1) {
                std::cerr << Utils::Color::RED << "Commit #" << commitId1 << " not found." << Utils::Color::RESET << "\n";
                return;
            }
            lines1 = c1->getLines();
            label1 = "commit #" + std::to_string(c1->getId());

            std::string fullPath = repoPath_ + "/" + trackedFile_;
            Utils::readFileLines(fullPath, lines2);
            label2 = "working file (" + trackedFile_ + ")";
        } else {
            // Diff between two specific commits
            Commit* c1 = findCommitById(commitId1);
            Commit* c2 = findCommitById(commitId2);
            if (!c1) {
                std::cerr << Utils::Color::RED << "Commit #" << commitId1 << " not found." << Utils::Color::RESET << "\n";
                return;
            }
            if (!c2) {
                std::cerr << Utils::Color::RED << "Commit #" << commitId2 << " not found." << Utils::Color::RESET << "\n";
                return;
            }
            lines1 = c1->getLines();
            lines2 = c2->getLines();
            label1 = "commit #" + std::to_string(c1->getId());
            label2 = "commit #" + std::to_string(c2->getId());
        }

        std::cout << Utils::Color::BOLD << "diff " << label1 << " <-> " << label2 << Utils::Color::RESET << "\n";
        SafeArray<DiffLine> diffResult;
        DiffEngine::computeDiff(lines1, lines2, diffResult);
        DiffEngine::printDiff(diffResult, std::cout, true);
    }

    /**
     * @brief Rolls back working file to a previous commit snapshot.
     * Records previous state on undo stack.
     */
    bool rollback(int commitId) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return false;
        }

        Commit* target = findCommitById(commitId);
        if (!target) {
            std::cerr << Utils::Color::RED << "error: commit #" << commitId << " not found in repository."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);

        // Save state to Undo stack before rolling back
        undoStack_.push(UndoState("Rollback to commit #" + std::to_string(commitId), currentBranch_, commitId, currentLines));
        redoStack_.clear();
        saveStacks();

        // Overwrite working file with commit snapshot
        if (!Utils::writeFileLines(fullPath, target->getLines())) {
            std::cerr << Utils::Color::RED << "fatal: failed to write to " << trackedFile_ << Utils::Color::RESET << "\n";
            return false;
        }

        std::cout << Utils::Color::GREEN << "Successfully rolled back '" << trackedFile_ << "' to commit #"
                  << commitId << " (\"" << target->getMessage() << "\")." << Utils::Color::RESET << "\n";
        std::cout << "Lines restored: " << target->getLines().size() << " | Hash: " << target->getHash() << "\n";
        std::cout << "Tip: You can reverse this rollback at any time with 'gitlite undo'.\n";
        return true;
    }

    /**
     * @brief Reverses the last rollback or checkout operation.
     */
    bool undo() {
        if (!isInitialized_) return false;
        if (undoStack_.empty()) {
            std::cout << Utils::Color::YELLOW << "Undo stack is empty. Nothing to undo." << Utils::Color::RESET << "\n";
            return false;
        }

        UndoState state = undoStack_.top();
        undoStack_.pop();

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);

        // Push current state to Redo stack
        redoStack_.push(UndoState("Revert of: " + state.actionName, currentBranch_, 0, currentLines));

        // Restore file
        Utils::writeFileLines(fullPath, state.workingFileLines);
        saveStacks();
        std::cout << Utils::Color::GREEN << "Undo executed: Reversed [" << state.actionName << "]."
                  << Utils::Color::RESET << "\n";
        std::cout << "Restored " << state.workingFileLines.size() << " lines to '" << trackedFile_ << "'.\n";
        return true;
    }

    /**
     * @brief Re-applies an undone operation.
     */
    bool redo() {
        if (!isInitialized_) return false;
        if (redoStack_.empty()) {
            std::cout << Utils::Color::YELLOW << "Redo stack is empty. Nothing to redo." << Utils::Color::RESET << "\n";
            return false;
        }

        UndoState state = redoStack_.top();
        redoStack_.pop();

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);

        // Push back to Undo stack
        undoStack_.push(UndoState("Re-applied: " + state.actionName, currentBranch_, 0, currentLines));

        Utils::writeFileLines(fullPath, state.workingFileLines);
        saveStacks();
        std::cout << Utils::Color::GREEN << "Redo executed: Re-applied [" << state.actionName << "]."
                  << Utils::Color::RESET << "\n";
        std::cout << "Restored " << state.workingFileLines.size() << " lines to '" << trackedFile_ << "'.\n";
        return true;
    }

    /**
     * @brief Creates a new branch pointing to current commit fork point.
     */
    bool createBranch(const std::string& branchName) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return false;
        }
        if (branchName.empty()) {
            std::cerr << Utils::Color::RED << "error: branch name cannot be empty." << Utils::Color::RESET << "\n";
            return false;
        }

        if (branchTree_.contains(branchName)) {
            std::cerr << Utils::Color::RED << "error: branch '" << branchName << "' already exists."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        int forkId = 0;
        DoublyLinkedList<Commit>* list = branchHistories_.find(currentBranch_);
        if (list && !list->empty()) {
            forkId = list->back().getId();
        }

        // Add to N-ary branch tree
        branchTree_.addBranch(currentBranch_, branchName, forkId);

        // Add to branch hash table
        branchHistories_.insert(branchName, DoublyLinkedList<Commit>());

        saveBranches();

        std::cout << Utils::Color::GREEN << "Created branch '" << branchName
                  << "' forked from '" << currentBranch_ << "' at commit #" << forkId << "."
                  << Utils::Color::RESET << "\n";
        return true;
    }

    /**
     * @brief Switches active branch and restores its latest commit to the working file.
     */
    bool checkout(const std::string& branchName) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return false;
        }

        if (!branchTree_.contains(branchName)) {
            std::cerr << Utils::Color::RED << "error: branch '" << branchName << "' does not exist."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        if (branchName == currentBranch_) {
            std::cout << "Already on branch '" << branchName << "'.\n";
            return true;
        }

        std::string fullPath = repoPath_ + "/" + trackedFile_;
        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);

        // Record undo state
        undoStack_.push(UndoState("Checkout branch " + branchName, currentBranch_, 0, currentLines));
        redoStack_.clear();

        currentBranch_ = branchName;
        saveConfig();
        saveStacks();

        // If target branch has commits, restore HEAD commit to working file
        DoublyLinkedList<Commit>* targetList = branchHistories_.find(branchName);
        if (targetList && !targetList->empty()) {
            const Commit& head = targetList->back();
            Utils::writeFileLines(fullPath, head.getLines());
            std::cout << Utils::Color::GREEN << "Switched to branch '" << branchName << "'.\n"
                      << "Working tree updated to commit #" << head.getId() << " (\"" << head.getMessage() << "\")."
                      << Utils::Color::RESET << "\n";
        } else {
            // Target branch has no commits yet (brand new branch); check if parent fork commit exists
            BranchNode* node = branchTree_.find(branchName);
            if (node && node->forkCommitId > 0) {
                Commit* forkCommit = findCommitById(node->forkCommitId);
                if (forkCommit) {
                    Utils::writeFileLines(fullPath, forkCommit->getLines());
                }
            }
            std::cout << Utils::Color::GREEN << "Switched to newly created branch '" << branchName << "'."
                      << Utils::Color::RESET << "\n";
        }

        return true;
    }

    /**
     * @brief Merges changes from the specified branch into the currently active branch.
     * Supports fast-forward merge and dynamic programming diff-based integration.
     */
    bool merge(const std::string& branchName) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return false;
        }

        if (branchName == currentBranch_) {
            std::cerr << Utils::Color::RED << "error: cannot merge branch '" << branchName << "' into itself."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        if (!branchTree_.contains(branchName)) {
            std::cerr << Utils::Color::RED << "error: branch '" << branchName << "' does not exist."
                      << Utils::Color::RESET << "\n";
            return false;
        }

        DoublyLinkedList<Commit>* targetList = branchHistories_.find(branchName);
        if (!targetList || targetList->empty()) {
            std::cout << Utils::Color::YELLOW << "Already up to date: branch '" << branchName
                      << "' has no commits to merge." << Utils::Color::RESET << "\n";
            return true;
        }

        DoublyLinkedList<Commit>* currentList = branchHistories_.find(currentBranch_);
        if (!currentList) {
            branchHistories_.insert(currentBranch_, DoublyLinkedList<Commit>());
            currentList = branchHistories_.find(currentBranch_);
        }

        const Commit& incomingCommit = targetList->back();
        std::string fullPath = repoPath_ + "/" + trackedFile_;
        SafeArray<std::string> currentLines;
        Utils::readFileLines(fullPath, currentLines);

        // Record undo state before merging
        undoStack_.push(UndoState("Merge branch '" + branchName + "' into " + currentBranch_,
                                  currentBranch_, 0, currentLines));
        redoStack_.clear();

        BranchNode* node = branchTree_.find(branchName);
        int forkId = node ? node->forkCommitId : 0;
        int currentHeadId = currentList->empty() ? 0 : currentList->back().getId();

        SafeArray<std::string> mergedLines;

        if (currentList->empty() || currentHeadId == forkId) {
            // Fast-forward merge: adopt incoming branch snapshot directly
            mergedLines = incomingCommit.getLines();
            std::cout << Utils::Color::CYAN << "Performing fast-forward merge of branch '" << branchName
                      << "' (commit #" << incomingCommit.getId() << ") into '" << currentBranch_ << "'...\n"
                      << Utils::Color::RESET;
        } else {
            // 3-way merge between fork base, current HEAD, and incoming HEAD
            Commit* forkCommit = (forkId > 0) ? findCommitById(forkId) : nullptr;
            SafeArray<std::string> baseLines = forkCommit ? forkCommit->getLines() : SafeArray<std::string>();
            const SafeArray<std::string>& myLines = currentList->back().getLines();
            const SafeArray<std::string>& theirLines = incomingCommit.getLines();

            // Compute diffs
            SafeArray<DiffLine> theirDiff;
            DiffEngine::computeDiff(baseLines, theirLines, theirDiff);

            mergedLines = myLines;
            for (size_t i = 0; i < theirDiff.size(); ++i) {
                if (theirDiff[i].type == DiffType::ADDED) {
                    bool alreadyPresent = false;
                    for (size_t m = 0; m < mergedLines.size(); ++m) {
                        if (mergedLines[m] == theirDiff[i].text) {
                            alreadyPresent = true;
                            break;
                        }
                    }
                    if (!alreadyPresent) {
                        mergedLines.push_back(theirDiff[i].text);
                    }
                }
            }
        }

        // Write merged lines to working file
        Utils::writeFileLines(fullPath, mergedLines);

        // Record merge commit
        int newId = nextCommitId_++;
        std::string mergeMsg = "Merge branch '" + branchName + "' into " + currentBranch_;
        Commit mergeCommit(newId, currentBranch_, currentHeadId, mergeMsg, mergedLines, author_);

        std::string commitFilePath = commitsDir_ + "/commit_" + std::to_string(newId) + ".txt";
        mergeCommit.saveToFile(commitFilePath);
        currentList->push_back(mergeCommit);

        saveConfig();
        saveStacks();

        std::cout << Utils::Color::GREEN << "[SUCCESS] Merged branch '" << branchName << "' into '"
                  << currentBranch_ << "' (commit #" << newId << ")." << Utils::Color::RESET << "\n";
        std::cout << "Working file '" << trackedFile_ << "' updated to " << mergedLines.size() << " lines.\n";
        std::cout << "Tip: Run 'gitlite log' to view the merge commit, or 'gitlite undo' to revert.\n";
        return true;
    }

    /**
     * @brief Displays branch tree with DFS visual indentation and BFS level-order traversal.
     */
    void printBranches() {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return;
        }

        branchTree_.printDFS(currentBranch_, std::cout);
        std::cout << "\n";
        branchTree_.printBFS(currentBranch_, std::cout);
    }

    /**
     * @brief Searches history for a keyword using KMP string matching algorithm.
     */
    void search(const std::string& keyword) {
        if (!isInitialized_) {
            std::cerr << Utils::Color::RED << "fatal: not a gitlite repository." << Utils::Color::RESET << "\n";
            return;
        }
        if (keyword.empty()) {
            std::cerr << Utils::Color::RED << "error: search keyword cannot be empty." << Utils::Color::RESET << "\n";
            return;
        }

        std::cout << "Searching commit history for keyword: \"" << Utils::Color::CYAN << keyword
                  << Utils::Color::RESET << "\" using Knuth-Morris-Pratt (KMP)...\n\n";

        SafeArray<SearchMatch> matches;
        SafeArray<std::string> branches;
        branchTree_.getAllBranchNames(branches);

        for (size_t b = 0; b < branches.size(); ++b) {
            DoublyLinkedList<Commit>* list = branchHistories_.find(branches[b]);
            if (!list) continue;

            auto curr = list->head();
            while (curr != nullptr) {
                const Commit& c = curr->data;
                const SafeArray<std::string>& lines = c.getLines();

                for (size_t l = 0; l < lines.size(); ++l) {
                    int matchIdx = SearchEngine::kmpSearch(lines[l], keyword);
                    if (matchIdx != -1) {
                        matches.push_back(SearchMatch(c.getId(), c.getBranch(), c.getTimestamp(),
                                                      c.getMessage(), l + 1, lines[l], static_cast<size_t>(matchIdx)));
                    }
                }
                curr = curr->next;
            }
        }

        if (matches.empty()) {
            std::cout << Utils::Color::YELLOW << "No occurrences of \"" << keyword << "\" found in commit history."
                      << Utils::Color::RESET << "\n";
            return;
        }

        std::cout << Utils::Color::GREEN << "Found " << matches.size() << " match(es):"
                  << Utils::Color::RESET << "\n\n";

        for (size_t i = 0; i < matches.size(); ++i) {
            const SearchMatch& m = matches[i];
            std::cout << Utils::Color::YELLOW << "Commit #" << m.commitId << Utils::Color::RESET
                      << " [" << m.branch << "] (" << m.timestamp << ") - \"" << m.commitMessage << "\"\n";
            std::cout << "  Line " << m.lineNum << ", Col " << (m.colIndex + 1) << ": ";

            // Highlight match inside line
            std::string before = m.lineContent.substr(0, m.colIndex);
            std::string matchPart = m.lineContent.substr(m.colIndex, keyword.length());
            std::string after = m.lineContent.substr(m.colIndex + keyword.length());

            std::cout << before << Utils::Color::GREEN << Utils::Color::BOLD << "[" << matchPart << "]"
                      << Utils::Color::RESET << after << "\n\n";
        }
    }
};

#endif // REPOSITORY_HPP
