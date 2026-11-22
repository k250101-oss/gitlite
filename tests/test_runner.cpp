#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "SafeArray.hpp"
#include "DoublyLinkedList.hpp"
#include "Stack.hpp"
#include "HashTable.hpp"
#include "BranchTree.hpp"
#include "DiffEngine.hpp"
#include "SearchEngine.hpp"
#include "Utils.hpp"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << Utils::Color::RED << "[FAILED] " << msg << " at line " << __LINE__ << Utils::Color::RESET << "\n"; \
            std::exit(1); \
        } else { \
            std::cout << Utils::Color::GREEN << "[PASSED] " << msg << Utils::Color::RESET << "\n"; \
        } \
    } while (0)

void testSafeArray() {
    std::cout << "\n--- Testing SafeArray (Rule of Five & Bounds Checking) ---\n";

    SafeArray<int> arr;
    TEST_ASSERT(arr.empty() && arr.size() == 0, "SafeArray initially empty");

    for (int i = 0; i < 20; ++i) {
        arr.push_back(i * 10);
    }
    TEST_ASSERT(arr.size() == 20, "SafeArray push_back and dynamic doubling");
    TEST_ASSERT(arr.at(5) == 50 && arr[19] == 190, "SafeArray index access");

    // Bounds checking
    bool threw = false;
    try {
        arr.at(25);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    TEST_ASSERT(threw, "SafeArray bounds check throws std::out_of_range on invalid index");

    // Copy Constructor (Deep Copy)
    SafeArray<int> arrCopy = arr;
    TEST_ASSERT(arrCopy.size() == arr.size(), "SafeArray copy constructor size matches");
    arrCopy[0] = 999;
    TEST_ASSERT(arr[0] == 0 && arrCopy[0] == 999, "SafeArray copy constructor performs deep copy");

    // Copy Assignment Operator
    SafeArray<int> arrAssign;
    arrAssign = arr;
    TEST_ASSERT(arrAssign.size() == arr.size(), "SafeArray assignment operator size matches");
    arrAssign[1] = 888;
    TEST_ASSERT(arr[1] == 10 && arrAssign[1] == 888, "SafeArray assignment operator performs deep copy");

    // Move Constructor
    SafeArray<int> arrMoved = std::move(arrCopy);
    TEST_ASSERT(arrMoved.size() == 20 && arrMoved[0] == 999, "SafeArray move constructor");
    TEST_ASSERT(arrCopy.size() == 0 && arrCopy.empty(), "SafeArray moved-from object is empty");
}

void testDoublyLinkedList() {
    std::cout << "\n--- Testing DoublyLinkedList (Rule of Five & Bidirectional Links) ---\n";

    DoublyLinkedList<std::string> list;
    TEST_ASSERT(list.empty(), "DoublyLinkedList initially empty");

    list.push_back("commit 1");
    list.push_back("commit 2");
    list.push_back("commit 3");
    list.push_front("commit 0");

    TEST_ASSERT(list.size() == 4, "DoublyLinkedList size matches after insertions");
    TEST_ASSERT(list.front() == "commit 0" && list.back() == "commit 3", "DoublyLinkedList front and back elements");

    // Traversal test forward and backward
    std::string forwardConcat = "";
    auto curr = list.head();
    while (curr) {
        forwardConcat += curr->data + ";";
        curr = curr->next;
    }
    TEST_ASSERT(forwardConcat == "commit 0;commit 1;commit 2;commit 3;", "DoublyLinkedList forward traversal");

    std::string backwardConcat = "";
    curr = list.tail();
    while (curr) {
        backwardConcat += curr->data + ";";
        curr = curr->prev;
    }
    TEST_ASSERT(backwardConcat == "commit 3;commit 2;commit 1;commit 0;", "DoublyLinkedList backward traversal");

    // Copy Constructor (Deep Copy)
    DoublyLinkedList<std::string> listCopy = list;
    TEST_ASSERT(listCopy.size() == list.size(), "DoublyLinkedList copy constructor size matches");
    listCopy.pop_back();
    TEST_ASSERT(list.size() == 4 && listCopy.size() == 3, "DoublyLinkedList deep copy isolation");

    // Copy Assignment Operator
    DoublyLinkedList<std::string> listAssign;
    listAssign = list;
    TEST_ASSERT(listAssign.size() == 4, "DoublyLinkedList copy assignment size matches");
    listAssign.pop_front();
    TEST_ASSERT(list.size() == 4 && listAssign.size() == 3, "DoublyLinkedList copy assignment deep copy isolation");
}

void testStack() {
    std::cout << "\n--- Testing Stack (LIFO & Rule of Five) ---\n";

    Stack<int> s;
    TEST_ASSERT(s.empty(), "Stack initially empty");

    s.push(10);
    s.push(20);
    s.push(30);
    TEST_ASSERT(s.size() == 3 && s.top() == 30, "Stack push and top");

    s.pop();
    TEST_ASSERT(s.top() == 20, "Stack pop updates top");

    // Copy Constructor
    Stack<int> sCopy = s;
    TEST_ASSERT(sCopy.size() == s.size() && sCopy.top() == 20, "Stack copy constructor");
    sCopy.push(100);
    TEST_ASSERT(s.size() == 2 && sCopy.size() == 3, "Stack copy constructor deep copy isolation");
}

void testHashTable() {
    std::cout << "\n--- Testing HashTable (Separate Chaining & Dynamic Rehashing) ---\n";

    HashTable<std::string, int> table(4); // Start with 4 buckets
    table.insert("main", 1);
    table.insert("feature", 2);
    table.insert("bugfix", 3);

    TEST_ASSERT(table.size() == 3, "HashTable size correct after insertions");
    TEST_ASSERT(table.contains("main") && *table.find("main") == 1, "HashTable contains and find key");
    TEST_ASSERT(table.contains("feature") && *table.find("feature") == 2, "HashTable find second key");

    // Trigger rehashing
    size_t oldBucketCount = table.bucket_count();
    table.insert("hotfix", 4);
    table.insert("experiment", 5);
    table.insert("dev", 6);
    table.insert("testing", 7);
    TEST_ASSERT(table.bucket_count() > oldBucketCount, "HashTable automatically rehashed when load factor exceeded");
    TEST_ASSERT(table.size() == 7, "HashTable all elements preserved after rehash");

    // Update existing key
    table.insert("main", 99);
    TEST_ASSERT(*table.find("main") == 99 && table.size() == 7, "HashTable update existing key without expanding size");

    // Remove key
    bool removed = table.remove("bugfix");
    TEST_ASSERT(removed && !table.contains("bugfix") && table.size() == 6, "HashTable remove key");

    // Copy Constructor (Deep Copy)
    HashTable<std::string, int> tableCopy = table;
    TEST_ASSERT(tableCopy.size() == table.size(), "HashTable copy constructor size matches");
    tableCopy.remove("main");
    TEST_ASSERT(table.contains("main") && !tableCopy.contains("main"), "HashTable copy constructor deep copy isolation");
}

void testBranchTree() {
    std::cout << "\n--- Testing BranchTree (N-ary Tree, DFS & BFS) ---\n";

    BranchTree tree("main");
    TEST_ASSERT(tree.contains("main"), "BranchTree has root branch 'main'");

    tree.addBranch("main", "feature-a", 1);
    tree.addBranch("main", "feature-b", 2);
    tree.addBranch("feature-a", "sub-feature", 3);

    TEST_ASSERT(tree.contains("feature-a"), "BranchTree contains feature-a");
    TEST_ASSERT(tree.contains("sub-feature"), "BranchTree contains nested child sub-feature");
    TEST_ASSERT(!tree.contains("non-existent"), "BranchTree correctly handles non-existent branch");

    BranchNode* node = tree.find("sub-feature");
    TEST_ASSERT(node != nullptr && node->parentBranch == "feature-a" && node->forkCommitId == 3,
                "BranchNode preserves correct parent and fork commit ID");

    // Deep copy verification
    BranchTree treeCopy = tree;
    TEST_ASSERT(treeCopy.contains("sub-feature"), "BranchTree copy constructor retains topology");
    treeCopy.addBranch("main", "copy-branch", 4);
    TEST_ASSERT(!tree.contains("copy-branch") && treeCopy.contains("copy-branch"),
                "BranchTree copy constructor deep copy isolation");
}

void testDiffEngine() {
    std::cout << "\n--- Testing DiffEngine (Recursive, Memoized & Bottom-Up DP) ---\n";

    SafeArray<std::string> fileA;
    fileA.push_back("Introduction");
    fileA.push_back("Data structures are fundamental.");
    fileA.push_back("We study trees and graphs.");
    fileA.push_back("Conclusion");

    SafeArray<std::string> fileB;
    fileB.push_back("Introduction");
    fileB.push_back("Data structures and algorithms are fundamental.");
    fileB.push_back("We study trees and graphs.");
    fileB.push_back("Future Work");
    fileB.push_back("Conclusion");

    int lcsRec = DiffEngine::computeLCSRecursive(fileA, fileB);
    int lcsMemo = DiffEngine::computeLCSMemoized(fileA, fileB);

    TEST_ASSERT(lcsRec == 3, "Recursive LCS computes correct subsequence length (3)");
    TEST_ASSERT(lcsMemo == lcsRec, "Memoized LCS exactly matches recursive result");

    SafeArray<DiffLine> diffDP;
    DiffEngine::computeDiff(fileA, fileB, diffDP);
    TEST_ASSERT(!diffDP.empty(), "Bottom-Up DP computes non-empty diff");

    // Verify reconstruction
    size_t addedCount = 0;
    size_t removedCount = 0;
    size_t unchangedCount = 0;
    for (size_t i = 0; i < diffDP.size(); ++i) {
        if (diffDP[i].type == DiffType::ADDED) ++addedCount;
        else if (diffDP[i].type == DiffType::REMOVED) ++removedCount;
        else if (diffDP[i].type == DiffType::UNCHANGED) ++unchangedCount;
    }

    TEST_ASSERT(unchangedCount == 3, "Diff correctly identifies 3 unchanged lines");
    TEST_ASSERT(removedCount == 1, "Diff correctly identifies 1 removed line");
    TEST_ASSERT(addedCount == 2, "Diff correctly identifies 2 added lines");
}

void testSearchAndSort() {
    std::cout << "\n--- Testing SearchEngine (KMP vs Brute-Force & Merge Sort) ---\n";

    // 1. KMP vs Brute Force
    std::string text = "ABC ABCDAB ABCDABCDABDE";
    std::string pattern = "ABCDABD";

    int kmpResult = SearchEngine::kmpSearch(text, pattern);
    int bfResult = SearchEngine::bruteForceSearch(text, pattern);

    TEST_ASSERT(kmpResult == 15, "KMP search finds correct match index");
    TEST_ASSERT(bfResult == kmpResult, "KMP and Brute-Force yield identical match index");

    // KMP with missing pattern
    int missing = SearchEngine::kmpSearch(text, "XYZ");
    TEST_ASSERT(missing == -1, "KMP returns -1 for missing pattern");

    // 2. Stable Merge Sort on Commits
    SafeArray<Commit> commits;
    SafeArray<std::string> dummyLines;
    dummyLines.push_back("dummy");

    Commit c3(3, "main", 2, "Third commit", dummyLines);
    c3.setTimestamp("2026-11-22 10:30:00");

    Commit c1(1, "main", 0, "First commit", dummyLines);
    c1.setTimestamp("2026-11-22 10:10:00");

    Commit c2(2, "main", 1, "Second commit", dummyLines);
    c2.setTimestamp("2026-11-22 10:20:00");

    commits.push_back(c3);
    commits.push_back(c1);
    commits.push_back(c2);

    SearchEngine::mergeSortCommits(commits);

    TEST_ASSERT(commits[0].getId() == 1, "Merge sort correctly placed commit #1 first");
    TEST_ASSERT(commits[1].getId() == 2, "Merge sort correctly placed commit #2 second");
    TEST_ASSERT(commits[2].getId() == 3, "Merge sort correctly placed commit #3 third");
}

int main() {
    std::cout << Utils::Color::BOLD << Utils::Color::CYAN
              << "========================================================\n"
              << "         GITLITE UNIT TEST & VALIDATION SUITE           \n"
              << "========================================================\n"
              << Utils::Color::RESET;

    testSafeArray();
    testDoublyLinkedList();
    testStack();
    testHashTable();
    testBranchTree();
    testDiffEngine();
    testSearchAndSort();

    std::cout << "\n" << Utils::Color::GREEN << Utils::Color::BOLD
              << "========================================================\n"
              << " ALL UNIT TESTS & RULE OF FIVE VALIDATIONS PASSED!     \n"
              << "========================================================\n"
              << Utils::Color::RESET << "\n";
    return 0;
}
