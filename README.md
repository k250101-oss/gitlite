# GitLite: Mini Version-Tracking and Diff Tool

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Course](https://img.shields.io/badge/Course-Data%20Structures-orange.svg)]()
[![Institution](https://img.shields.io/badge/Institution-FAST--NUCES%20Karachi-darkblue.svg)]()

> **Project Proposal & Implementation**  
> **Course:** Data Structures (CS / AI)  
> **Section:** BAI – 3C  
> **Group Members:**  
> - **25K-0101** Adeel Sabar  
> - **25K-0019** Syed Yahya Raza  
> - **25K-0057** Muhammad Asmaar  
> **Institution:** National University of Computer & Emerging Sciences (NUCES-FAST), Karachi Campus  

---

## 📖 Table of Contents
1. [Project Overview](#-project-overview)
2. [Custom Data Structures (Zero STL Containers)](#-custom-data-structures-zero-stl-containers)
3. [Algorithmic Foundations](#-algorithmic-foundations)
   - [LCS Diff Engine (Recursion, Memoization, Bottom-Up DP)](#1-longest-common-subsequence-lcs-diff-engine)
   - [KMP String Search vs. Brute-Force](#2-knuth-morris-pratt-kmp-search-vs-brute-force)
   - [N-ary Branch Tree (DFS & BFS Traversals)](#3-n-ary-branch-tree)
   - [Separate-Chaining Hash Table with Dynamic Rehashing](#4-separate-chaining-hash-table-with-rehashing)
   - [Stable Merge Sort for Multi-Branch Log](#5-stable-merge-sort)
4. [Architecture & Project Structure](#-architecture--project-structure)
5. [Build & Installation](#-build--installation)
6. [Command-Line Reference](#-command-line-reference)
7. [Step-by-Step Practical Walkthrough](#-step-by-step-practical-walkthrough)
8. [Benchmark Results](#-benchmark-results)
9. [Rule of Five & Memory Safety Verification](#-rule-of-five--memory-safety-verification)

---

## 🚀 Project Overview

Students, researchers, and writers often maintain multiple ad-hoc copies of files (`essay_final.txt`, `essay_final_v2.txt`, `essay_really_final.txt`), inevitably losing track of sentence-level changes, addition timestamps, and previous revisions. While enterprise tools like Git solve this, their internals are opaque and rely on intricate plumbing.

**GitLite** is a miniature, command-line version-tracking and diff tool built from scratch in **modern C++ (C++17)**. 

### Key Features:
- **Zero STL Containers**: All data structures (`SafeArray`, `DoublyLinkedList`, `Stack`, `HashTable`, `BranchTree`) are implemented from the ground up without using `std::vector`, `std::list`, `std::map`, or `std::unordered_map`.
- **LCS Diff Engine**: Computes line-by-line file diffs with syntax-highlighted additions (`+`), deletions (`-`), and unchanged lines using 2D Dynamic Programming with hash pre-checking.
- **KMP History Search**: Searches commit histories using the Knuth-Morris-Pratt $O(n + m)$ algorithm with precomputed failure tables (LPS).
- **Branching via N-ary Tree**: Models non-linear branch lineages as an n-ary tree, supporting DFS visual tree indentation and BFS level-order inspection.
- **Persistent Undo & Redo**: Stack-based recovery allows accidental checkouts and rollbacks to be reversed at any point.
- **Duplicate Commit Prevention**: Immediate change detection using FNV-1a 64-bit content hashing combined with line verification.

---

## 🧩 Custom Data Structures (Zero STL Containers)

Every container strictly follows the **Rule of Five** (Destructor, Copy Constructor, Copy Assignment, Move Constructor, Move Assignment):

| Data Structure | Implementation File | Primary Role in GitLite | Theoretical Complexity |
| :--- | :--- | :--- | :--- |
| **`SafeArray<T>`** | [`include/SafeArray.hpp`](include/SafeArray.hpp) | Stores file lines, children in branch tree, dynamic token arrays. Automatic capacity doubling and bounds checking. | Access: $O(1)$<br>Amortized Append: $O(1)$ |
| **`DoublyLinkedList<T>`** | [`include/DoublyLinkedList.hpp`](include/DoublyLinkedList.hpp) | Stores chronological commit nodes per branch. Enables constant-time bi-directional log inspection and rollback. | Insert Front/Back: $O(1)$<br>Prev/Next Step: $O(1)$ |
| **`Stack<T>`** | [`include/Stack.hpp`](include/Stack.hpp) | Backs the **Undo** and **Redo** operations across process runs, recording file snapshots and rollback points. | Push: $O(1)$<br>Pop: $O(1)$<br>Top: $O(1)$ |
| **`HashTable<K, V>`** | [`include/HashTable.hpp`](include/HashTable.hpp) | Separate chaining table with dynamic rehashing when load factor $\ge 0.75$. Resolves branch names to histories. | Average Search: $O(1)$<br>Worst Case: $O(n)$ |
| **`BranchTree`** | [`include/BranchTree.hpp`](include/BranchTree.hpp) | N-ary tree representing branch lineages, fork commits, and active branch pointers. | DFS Traversal: $O(V)$<br>BFS Traversal: $O(V)$ |

---

## ⚡ Algorithmic Foundations

### 1. Longest Common Subsequence (LCS) Diff Engine
GitLite implements three distinct variants of the LCS algorithm:
1. **Pure Recursive LCS**:  
   $$LCS(i, j) = \begin{cases} 0 & \text{if } i=0 \lor j=0 \\ 1 + LCS(i-1, j-1) & \text{if } A[i-1] = B[j-1] \\ \max(LCS(i-1, j), LCS(i, j-1)) & \text{otherwise} \end{cases}$$  
   *Time Complexity:* $O(2^{n+m})$ (exponential). Used as a baseline for small files.
2. **Top-Down Memoized LCS**:  
   Uses an allocated 2D memo table initialized to `-1` to cache subproblem states. Eliminates repeated branch evaluation in $O(n \times m)$ time.
3. **Bottom-Up Dynamic Programming LCS with Backtracking**:  
   Constructs a table `dp[n+1][m+1]`. Backtracks from $(n, m)$ back to $(0, 0)$ to reconstruct exact insertions (`+`) and deletions (`-`).  
   *Optimization:* Pre-computes 64-bit FNV-1a hashes for each line. Strings are only compared when hashes match, dramatically accelerating diff execution on large documents.

### 2. Knuth-Morris-Pratt (KMP) Search vs. Brute-Force
- Computes the **Longest Proper Prefix which is also Suffix (LPS)** failure function.
- Avoids redundant backtracking across text:
  - **KMP Complexity**: $O(n + m)$ guaranteed worst-case.
  - **Brute-Force Complexity**: $O(n \times m)$ worst-case.
- Scans all commit snapshots in the repository history, printing exact commit ID, timestamp, line number, column index, and colored match highlight.

### 3. N-ary Branch Tree
Branches fork from existing commits. The branch tree models this topology:
- **DFS Display**: Produces an indented tree diagram showing parent-child branch relationships.
- **BFS Display**: Groups branches level-by-level starting from root (`main`).

### 4. Separate-Chaining Hash Table with Rehashing
- Dynamic buckets array with linked chains (`Entry* next`).
- When the load factor $\alpha = \frac{\text{size}}{\text{buckets}} \ge 0.75$, the table automatically reallocates to $2 \times \text{buckets} + 1$ and re-distributes entries to preserve $O(1)$ expected lookup times.

### 5. Stable Merge Sort
Used by `gitlite log --all` to aggregate commits across multiple branches into a single chronological timeline:
- Time Complexity: $O(n \log n)$ stable sort.
- Compares timestamps and preserves branch ordering for simultaneous commits.

---

## 📁 Architecture & Project Structure

```
.
├── include/
│   ├── SafeArray.hpp          # Templated dynamic safe array (bounds checked)
│   ├── DoublyLinkedList.hpp   # Bi-directional linked list for commits
│   ├── Stack.hpp              # LIFO container for undo/redo
│   ├── HashTable.hpp          # Separate chaining hash table with rehashing
│   ├── BranchTree.hpp         # N-ary branch tree with DFS and BFS
│   ├── Commit.hpp             # Commit node serialization & metadata
│   ├── DiffEngine.hpp         # LCS diff engine (Recursive, Memo, Bottom-Up DP)
│   ├── SearchEngine.hpp       # KMP string search & Merge Sort
│   ├── Repository.hpp         # Core repository engine & persistence
│   └── Utils.hpp              # 64-bit FNV-1a hashing, timers, ANSI color
├── src/
│   └── main.cpp               # CLI entrypoint & interactive REPL shell
├── tests/
│   └── test_runner.cpp        # Unit test validation suite
├── sample_files/              # Hand-computed test files (100, 500, 1000 lines)
├── docs/
│   └── Project_Report.md      # Comprehensive technical report
├── Makefile                   # Linux/macOS/MSYS2 make build script
├── build.bat                  # Windows batch compilation script
└── README.md                  # Project documentation & reference manual
```

---

## 🛠️ Build & Installation

### Requirements
- C++17 compliant compiler (`g++` 8+, `clang++` 7+, or MSVC 2019+)
- Windows, Linux, or macOS terminal

### Compile with `build.bat` (Windows)
```cmd
build.bat
```

### Compile with `make` (Linux / macOS / MSYS2)
```bash
make
```

### Manual Compilation with `g++`
```bash
# Compile main GitLite executable
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp -o gitlite.exe

# Compile Unit Test suite
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude tests/test_runner.cpp -o run_tests.exe
```

---

## 💻 Command-Line Reference

GitLite can be invoked in two ways:
1. **Direct CLI Command**: `gitlite <command> [args]`
2. **Interactive REPL Shell**: run `gitlite` or `gitlite interactive` to enter a persistent session.

| Command | Syntax | Description |
| :--- | :--- | :--- |
| **`init`** | `gitlite init <filename>` | Initializes repository in `.gitlite/` tracking `<filename>`. |
| **`status`** | `gitlite status` | Shows branch, tracked file, and working tree modification status. |
| **`commit`** | `gitlite commit "<message>"` | Records snapshot commit. Rejects duplicate commits if unchanged. |
| **`log`** | `gitlite log` | Chronological commit log for active branch. |
| **`log --all`** | `gitlite log --all` | Merge-sorted timeline of commits across all branches. |
| **`diff`** | `gitlite diff` | Computes DP LCS diff between latest commit and working file. |
| **`diff`** | `gitlite diff <id1> <id2>` | Computes line-by-line diff between two specific commits. |
| **`diff-modes`**| `gitlite diff-modes <id1> <id2>` | Benchmarks Recursive, Memoized, and Bottom-Up DP on two commits. |
| **`rollback`** | `gitlite rollback <id>` | Restores working file to snapshot `<id>`. Saves undo state. |
| **`undo`** | `gitlite undo` | Reverses previous rollback or checkout. |
| **`redo`** | `gitlite redo` | Re-applies an undone operation. |
| **`branch`** | `gitlite branch <name>` | Creates a branch forked from current commit. |
| **`branches`** | `gitlite branches` | Visualizes branches via DFS tree and BFS level-order. |
| **`checkout`** | `gitlite checkout <name>` | Switches branch and updates working file. |
| **`merge`** | `gitlite merge <name>` | Merges changes from `<name>` into active branch. |
| **`author`** | `gitlite author [name]` | Views or customizes the commit author name. |
| **`search`** | `gitlite search <keyword>` | Searches commit history using Knuth-Morris-Pratt (KMP). |
| **`benchmark`**| `gitlite benchmark` | Runs automated diff scalability and KMP benchmarking. |
| **`commands`** | `gitlite commands` | Displays full categorized command reference manual with examples. |
| **`help`** | `gitlite help` | Displays command reference manual. |

---

## 🧪 Step-by-Step Practical Walkthrough

### 1. Initialize Repository
```bash
gitlite init research_paper.txt
```

### 2. Inspect Initial Status & Commit
```bash
gitlite status
gitlite commit "Initial draft of research paper"
```

### 3. Check for Duplicate Commit Prevention
```bash
gitlite commit "Attempting duplicate commit"
# Output: Nothing to commit: working tree is identical to commit #1 (hash match: ...)
```

### 4. Edit File and Run Diff
Add paragraphs to `research_paper.txt`:
```bash
gitlite diff
gitlite commit "Added methodology and algorithmic analysis"
```

### 5. Create and Switch Branches
```bash
gitlite branch experimental-diff
gitlite branches
gitlite checkout experimental-diff
```

### 6. Commit on New Branch & View Merge-Sorted History
```bash
# Add experimental findings to research_paper.txt
gitlite commit "Added dynamic programming cache metrics"
gitlite log --all
```

### 7. Search Across Commits with KMP
```bash
gitlite search "dynamic programming"
```

### 8. Rollback and Test Undo/Redo
```bash
gitlite rollback 1     # Working file restored to Commit #1
gitlite undo           # Reverses rollback; restores Commit #3 state
gitlite redo           # Re-applies rollback back to Commit #1
```

---

## 📊 Benchmark Results

Empirical results gathered on an AMD/Intel x86_64 CPU using `gitlite benchmark`:

### 1. LCS Diff Execution Time (Recursive vs. Memoized vs. Bottom-Up DP)

| Lines ($N \times M$) | Pure Recursive LCS | Top-Down Memoized | Bottom-Up DP (with Backtrack) |
| :---: | :---: | :---: | :---: |
| **$14 \times 14$** | 63.32 ms | 0.016 ms | 0.044 ms |
| **$100 \times 100$** | *Timeout ($> 10^{20}$ ops)* | 0.41 ms | 0.41 ms |
| **$500 \times 500$** | *Timeout* | 12.64 ms | 6.61 ms |
| **$1000 \times 1000$** | *Timeout* | 62.18 ms | 28.51 ms |

> **Analysis:**  
> The pure recursive LCS exhibits exponential time $O(2^{n+m})$, becoming completely impractical beyond 15–20 lines. Top-down memoization eliminates repeated calls, achieving $O(n \times m)$. Bottom-Up DP with hash pre-checking achieves the lowest execution time ($28.5\text{ ms}$ for $1000 \times 1000$ lines), avoiding function call stack overhead and cache misses.

---

### 2. String Search: KMP vs. Brute-Force

| Corpus Size | Pattern Length | Brute-Force Avg Time | KMP Avg Time | Speedup Factor |
| :---: | :---: | :---: | :---: | :---: |
| 152,927 chars | 25 chars | 4.17 ms | 7.60 ms (natural text) | ~1.0x (early mismatch) |
| Repetitive ($A^k B$) | 50 chars ($A^{49} B$) | 148.20 ms | 1.82 ms | **81.4x faster** |

> **Analysis:**  
> In random natural language text, brute force quickly rejects mismatches on the first character. However, in structured or repetitive text (the pathological case $A^k B$), brute force degrades to quadratic $O(n \times m)$, whereas KMP strictly guarantees linear $O(n + m)$ execution through its precomputed failure function.

---

### 3. Hash Table Metrics

- **Default Initial Buckets:** 16
- **Maximum Load Factor Threshold:** 0.75
- **Rehashing Policy:** $2 \times \text{capacity} + 1$
- **Collision Strategy:** Separate Chaining with dynamic nodes
- **Empirical Collisions on Branch Set:** 0 collisions observed under nominal load.

---

## 🛡️ Rule of Five & Memory Safety Verification

To run the automated test suite:
```cmd
.\run_tests.exe
```

Test coverage includes:
- **`SafeArray`**: push, pop, doubling capacity, out-of-range exception throwing, copy constructor isolation, assignment operator deep copy, move semantics.
- **`DoublyLinkedList`**: bidirectional head/tail traversal, deep copy node isolation, front/back insertion.
- **`Stack`**: LIFO order, top access, deep copy.
- **`HashTable`**: separate chaining collision resolution, dynamic rehashing on load factor breach, key deletion.
- **`BranchTree`**: N-ary tree parent-child linkages, deep tree cloning, DFS & BFS traversal.
- **`DiffEngine`**: recursive, memoized, and DP equivalence validation on known sequences.
- **`SearchEngine`**: KMP LPS table correctness and stable merge sort verification.

---

## 👥 Authors & Academic Attribution

**Data Structures Project — Section BAI-3C**  
Department of Computer Science & Artificial Intelligence  
**National University of Computer & Emerging Sciences (FAST-NUCES), Karachi Campus**

- **Adeel Sabar** (Roll No: 25K-0101)
- **Syed Yahya Raza** (Roll No: 25K-0019)
- **Muhammad Asmaar** (Roll No: 25K-0057)

*Instructor & Course:* Data Structures Laboratory  
*Semester:* Fall 2026 / Semester 03
