#ifndef SEARCHENGINE_HPP
#define SEARCHENGINE_HPP

#include <string>
#include <iostream>
#include <cctype>
#include "SafeArray.hpp"
#include "Commit.hpp"
#include "Utils.hpp"

struct SearchMatch {
    int commitId;
    std::string branch;
    std::string timestamp;
    std::string commitMessage;
    size_t lineNum;
    std::string lineContent;
    size_t colIndex;

    SearchMatch() : commitId(0), lineNum(0), colIndex(0) {}
    SearchMatch(int cid, const std::string& b, const std::string& ts,
                const std::string& msg, size_t lnum, const std::string& ltext, size_t cidx)
        : commitId(cid), branch(b), timestamp(ts), commitMessage(msg),
          lineNum(lnum), lineContent(ltext), colIndex(cidx) {}
};

/**
 * @brief Search & Sorting Engine for GitLite.
 * Implements:
 *  1. Knuth-Morris-Pratt (KMP) with failure / LPS table (O(n + m))
 *  2. Brute-force String Search for benchmarking (O(n * m))
 *  3. Custom Stable Merge Sort for log --all commit chronological ordering (O(n log n))
 */
class SearchEngine {
public:
    /**
     * @brief Computes the KMP Longest Proper Prefix which is also Suffix (LPS) / Failure array.
     */
    static void computeLPSArray(const std::string& pattern, SafeArray<int>& lps, bool ignoreCase = true) {
        size_t m = pattern.length();
        lps.clear();
        for (size_t i = 0; i < m; ++i) {
            lps.push_back(0);
        }

        size_t len = 0;
        size_t i = 1;

        while (i < m) {
            char p_i = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(pattern[i]))) : pattern[i];
            char p_len = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(pattern[len]))) : pattern[len];

            if (p_i == p_len) {
                ++len;
                lps[i] = static_cast<int>(len);
                ++i;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    ++i;
                }
            }
        }
    }

    /**
     * @brief Performs Knuth-Morris-Pratt search. Returns first matching 0-based index or -1 if not found.
     * Supports case-insensitive searching by default for optimal user experience.
     */
    static int kmpSearch(const std::string& text, const std::string& pattern, bool ignoreCase = true) {
        if (pattern.empty()) return 0;
        if (text.empty() || text.length() < pattern.length()) return -1;

        SafeArray<int> lps;
        computeLPSArray(pattern, lps, ignoreCase);

        size_t n = text.length();
        size_t m = pattern.length();
        size_t i = 0; // index for text
        size_t j = 0; // index for pattern

        while (i < n) {
            char t_i = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(text[i]))) : text[i];
            char p_j = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(pattern[j]))) : pattern[j];

            if (p_j == t_i) {
                ++i;
                ++j;
            }

            if (j == m) {
                return static_cast<int>(i - j); // Match found!
            } else if (i < n && p_j != t_i) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    ++i;
                }
            }
        }
        return -1;
    }

    /**
     * @brief Performs Brute-Force string search. Returns matching 0-based index or -1 if not found.
     */
    static int bruteForceSearch(const std::string& text, const std::string& pattern, bool ignoreCase = true) {
        if (pattern.empty()) return 0;
        if (text.empty() || text.length() < pattern.length()) return -1;

        size_t n = text.length();
        size_t m = pattern.length();

        for (size_t i = 0; i <= n - m; ++i) {
            size_t j = 0;
            while (j < m) {
                char t_c = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(text[i + j]))) : text[i + j];
                char p_c = ignoreCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(pattern[j]))) : pattern[j];
                if (t_c != p_c) break;
                ++j;
            }
            if (j == m) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    /**
     * @brief Custom Stable Merge Sort for commits by timestamp (or commit ID).
     */
    static void merge(SafeArray<Commit>& arr, size_t left, size_t mid, size_t right) {
        size_t n1 = mid - left + 1;
        size_t n2 = right - mid;

        SafeArray<Commit> L;
        SafeArray<Commit> R;

        for (size_t i = 0; i < n1; ++i) {
            L.push_back(arr[left + i]);
        }
        for (size_t j = 0; j < n2; ++j) {
            R.push_back(arr[mid + 1 + j]);
        }

        size_t i = 0;
        size_t j = 0;
        size_t k = left;

        while (i < n1 && j < n2) {
            // Sort primarily by timestamp; tie-break by commit ID
            bool isLessOrEqual = false;
            if (L[i].getTimestamp() < R[j].getTimestamp()) {
                isLessOrEqual = true;
            } else if (L[i].getTimestamp() == R[j].getTimestamp()) {
                isLessOrEqual = (L[i].getId() <= R[j].getId());
            }

            if (isLessOrEqual) {
                arr[k++] = L[i++];
            } else {
                arr[k++] = R[j++];
            }
        }

        while (i < n1) {
            arr[k++] = L[i++];
        }
        while (j < n2) {
            arr[k++] = R[j++];
        }
    }

    static void mergeSortHelper(SafeArray<Commit>& arr, size_t left, size_t right) {
        if (left < right) {
            size_t mid = left + (right - left) / 2;
            mergeSortHelper(arr, left, mid);
            mergeSortHelper(arr, mid + 1, right);
            merge(arr, left, mid, right);
        }
    }

    static void mergeSortCommits(SafeArray<Commit>& commits) {
        if (commits.size() <= 1) return;
        mergeSortHelper(commits, 0, commits.size() - 1);
    }
};

#endif // SEARCHENGINE_HPP
