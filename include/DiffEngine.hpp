#ifndef DIFFENGINE_HPP
#define DIFFENGINE_HPP

#include <string>
#include <iostream>
#include <algorithm>
#include "SafeArray.hpp"
#include "Utils.hpp"

enum class DiffType {
    UNCHANGED,
    ADDED,
    REMOVED
};

struct DiffLine {
    DiffType type;
    std::string text;
    int oldLineNum;
    int newLineNum;

    DiffLine() : type(DiffType::UNCHANGED), text(""), oldLineNum(0), newLineNum(0) {}
    DiffLine(DiffType t, const std::string& txt, int oldNum, int newNum)
        : type(t), text(txt), oldLineNum(oldNum), newLineNum(newNum) {}
};

/**
 * @brief Diff Engine using Longest Common Subsequence (LCS).
 * Provides:
 *  1. Pure Recursive LCS (O(2^(n+m)))
 *  2. Memoized Top-Down LCS (O(n * m))
 *  3. Bottom-Up Dynamic Programming LCS (O(n * m)) with Backtracking
 * Uses line-hash pre-check for high-speed line comparison.
 */
class DiffEngine {
private:
    // Helper: Fast line equality check (Hash comparison first, string comparison second)
    static inline bool areLinesEqual(const std::string& lineA, uint64_t hashA,
                                     const std::string& lineB, uint64_t hashB) {
        if (hashA != hashB) return false;
        return lineA == lineB;
    }

    // Precompute hashes for SafeArray of lines
    static void computeHashes(const SafeArray<std::string>& lines, SafeArray<uint64_t>& hashes) {
        hashes.clear();
        for (size_t i = 0; i < lines.size(); ++i) {
            hashes.push_back(Utils::hashString(lines[i]));
        }
    }

    // 1. Recursive helper
    static int lcsRecursiveHelper(const SafeArray<std::string>& A, const SafeArray<uint64_t>& hA, size_t i,
                                  const SafeArray<std::string>& B, const SafeArray<uint64_t>& hB, size_t j) {
        if (i == 0 || j == 0) {
            return 0;
        }
        if (areLinesEqual(A[i - 1], hA[i - 1], B[j - 1], hB[j - 1])) {
            return 1 + lcsRecursiveHelper(A, hA, i - 1, B, hB, j - 1);
        } else {
            int option1 = lcsRecursiveHelper(A, hA, i - 1, B, hB, j);
            int option2 = lcsRecursiveHelper(A, hA, i, B, hB, j - 1);
            return (option1 > option2) ? option1 : option2;
        }
    }

    // 2. Memoized helper
    static int lcsMemoHelper(const SafeArray<std::string>& A, const SafeArray<uint64_t>& hA, size_t i,
                             const SafeArray<std::string>& B, const SafeArray<uint64_t>& hB, size_t j,
                             int** memo) {
        if (i == 0 || j == 0) {
            return 0;
        }
        if (memo[i][j] != -1) {
            return memo[i][j];
        }
        if (areLinesEqual(A[i - 1], hA[i - 1], B[j - 1], hB[j - 1])) {
            memo[i][j] = 1 + lcsMemoHelper(A, hA, i - 1, B, hB, j - 1, memo);
        } else {
            int opt1 = lcsMemoHelper(A, hA, i - 1, B, hB, j, memo);
            int opt2 = lcsMemoHelper(A, hA, i, B, hB, j - 1, memo);
            memo[i][j] = (opt1 > opt2) ? opt1 : opt2;
        }
        return memo[i][j];
    }

public:
    /**
     * @brief Computes LCS length using pure recursion (exponential time complexity O(2^(n+m))).
     * Note: Recommended only for small input lengths (e.g. <= 15).
     */
    static int computeLCSRecursive(const SafeArray<std::string>& A, const SafeArray<std::string>& B) {
        SafeArray<uint64_t> hA, hB;
        computeHashes(A, hA);
        computeHashes(B, hB);
        return lcsRecursiveHelper(A, hA, A.size(), B, hB, B.size());
    }

    /**
     * @brief Computes LCS length using Top-Down Dynamic Programming with Memoization.
     */
    static int computeLCSMemoized(const SafeArray<std::string>& A, const SafeArray<std::string>& B) {
        size_t n = A.size();
        size_t m = B.size();

        SafeArray<uint64_t> hA, hB;
        computeHashes(A, hA);
        computeHashes(B, hB);

        int** memo = new int*[n + 1];
        for (size_t i = 0; i <= n; ++i) {
            memo[i] = new int[m + 1];
            for (size_t j = 0; j <= m; ++j) {
                memo[i][j] = -1;
            }
        }

        int result = lcsMemoHelper(A, hA, n, B, hB, m, memo);

        for (size_t i = 0; i <= n; ++i) {
            delete[] memo[i];
        }
        delete[] memo;

        return result;
    }

    /**
     * @brief Computes full diff using Bottom-Up Dynamic Programming table and backtracking.
     */
    static void computeDiff(const SafeArray<std::string>& oldLines,
                            const SafeArray<std::string>& newLines,
                            SafeArray<DiffLine>& diffResult) {
        diffResult.clear();

        size_t n = oldLines.size();
        size_t m = newLines.size();

        SafeArray<uint64_t> oldHashes, newHashes;
        computeHashes(oldLines, oldHashes);
        computeHashes(newLines, newHashes);

        // Allocate DP table (n + 1) x (m + 1)
        int** dp = new int*[n + 1];
        for (size_t i = 0; i <= n; ++i) {
            dp[i] = new int[m + 1]();
        }

        // Fill DP table bottom-up
        for (size_t i = 1; i <= n; ++i) {
            for (size_t j = 1; j <= m; ++j) {
                if (areLinesEqual(oldLines[i - 1], oldHashes[i - 1], newLines[j - 1], newHashes[j - 1])) {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                } else {
                    dp[i][j] = (dp[i - 1][j] >= dp[i][j - 1]) ? dp[i - 1][j] : dp[i][j - 1];
                }
            }
        }

        // Backtrack to reconstruct diff path
        SafeArray<DiffLine> reverseDiff;
        size_t i = n;
        size_t j = m;

        while (i > 0 || j > 0) {
            if (i > 0 && j > 0 && areLinesEqual(oldLines[i - 1], oldHashes[i - 1], newLines[j - 1], newHashes[j - 1])) {
                reverseDiff.push_back(DiffLine(DiffType::UNCHANGED, oldLines[i - 1], static_cast<int>(i), static_cast<int>(j)));
                --i;
                --j;
            } else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j])) {
                // Line added in new version
                reverseDiff.push_back(DiffLine(DiffType::ADDED, newLines[j - 1], -1, static_cast<int>(j)));
                --j;
            } else if (i > 0 && (j == 0 || dp[i][j - 1] < dp[i - 1][j])) {
                // Line removed from old version
                reverseDiff.push_back(DiffLine(DiffType::REMOVED, oldLines[i - 1], static_cast<int>(i), -1));
                --i;
            }
        }

        // Clean up DP table
        for (size_t r = 0; r <= n; ++r) {
            delete[] dp[r];
        }
        delete[] dp;

        // Reverse into chronological order
        for (size_t k = reverseDiff.size(); k > 0; --k) {
            diffResult.push_back(reverseDiff[k - 1]);
        }
    }

    /**
     * @brief Prints the diff to an output stream with clean visual structure and ANSI color styling.
     */
    static void printDiff(const SafeArray<DiffLine>& diffResult, std::ostream& os = std::cout, bool useColor = true) {
        size_t additions = 0;
        size_t deletions = 0;
        size_t unchanged = 0;

        os << "--------------------------------------------------------------------------------\n";
        os << " Tag | Line | Content\n";
        os << "--------------------------------------------------------------------------------\n";

        for (size_t i = 0; i < diffResult.size(); ++i) {
            const DiffLine& dl = diffResult[i];
            if (dl.type == DiffType::ADDED) {
                ++additions;
                if (useColor) os << Utils::Color::GREEN;
                os << "  +  | L" << std::setw(4) << std::left << dl.newLineNum << " | " << dl.text;
                if (useColor) os << Utils::Color::RESET;
                os << "\n";
            } else if (dl.type == DiffType::REMOVED) {
                ++deletions;
                if (useColor) os << Utils::Color::RED;
                os << "  -  | L" << std::setw(4) << std::left << dl.oldLineNum << " | " << dl.text;
                if (useColor) os << Utils::Color::RESET;
                os << "\n";
            } else {
                ++unchanged;
                os << "     | L" << std::setw(4) << std::left << dl.newLineNum << " | " << dl.text << "\n";
            }
        }

        os << "--------------------------------------------------------------------------------\n";
        os << "Diff Summary: ";
        if (useColor) os << Utils::Color::GREEN;
        os << "+" << additions << " added";
        if (useColor) os << Utils::Color::RESET << ", ";
        if (useColor) os << Utils::Color::RED;
        os << "-" << deletions << " removed";
        if (useColor) os << Utils::Color::RESET << ", ";
        os << unchanged << " unchanged lines\n";
        os << "--------------------------------------------------------------------------------\n";
    }
};

#endif // DIFFENGINE_HPP
