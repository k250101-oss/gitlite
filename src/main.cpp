#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Repository.hpp"
#include "DiffEngine.hpp"
#include "SearchEngine.hpp"
#include "Utils.hpp"

void printBanner() {
    std::cout << Utils::Color::CYAN << Utils::Color::BOLD
              << "+--------------------------------------------------------------+\n"
              << "|   ____ _ _   _     _ _                                       |\n"
              << "|  / ___(_) |_| |   (_) |_ ___                                 |\n"
              << "| | |  _| | __| |   | | __/ _ \\                                |\n"
              << "| | |_| | | |_| |___| | ||  __/                                |\n"
              << "|  \\____|_|\\__|_____|_|\\__\\___|                                |\n"
              << "|   GitLite: Mini Version-Tracking & Diff Tool                 |\n"
              << "|   Adeel Sabar, Syed Yahya Raza & Asmaar                      |\n"
              << "|   FAST-NUCES Karachi - Data Structures (Section BAI-3C)      |\n"
              << "+--------------------------------------------------------------+\n"
              << Utils::Color::RESET << "\n";
}

void printCommandsManual() {
    std::cout << Utils::Color::BOLD << Utils::Color::CYAN
              << "========================================================================================\n"
              << "                          GITLITE - COMMAND REFERENCE MANUAL                            \n"
              << "========================================================================================\n"
              << Utils::Color::RESET;
    std::cout << "Usage: " << Utils::Color::BOLD << "gitlite <command> [arguments]" << Utils::Color::RESET << "\n\n";

    // 1. REPOSITORY SETUP
    std::cout << Utils::Color::YELLOW << "[ 1. REPOSITORY SETUP ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "init <filename>" << Utils::Color::RESET
              << "Initialize a new GitLite repository tracking <filename>.\n"
              << "                             Creates hidden .gitlite directory with index & root branch.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite init essay.txt\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "delete-repo" << Utils::Color::RESET
              << "Delete the GitLite repository (.gitlite directory) while keeping the tracked file.\n"
              << "                             Prompts for confirmation unless --force or -f is specified.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite delete-repo\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "author [name]" << Utils::Color::RESET
              << "View or customize the commit author name.\n"
              << "                             Default: \"Adeel, Yahya & Asmaar\".\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite author \"Adeel, Yahya & Asmaar\"\n\n" << Utils::Color::RESET;

    // 2. DAY-TO-DAY WORKFLOW
    std::cout << Utils::Color::YELLOW << "[ 2. DAY-TO-DAY WORKFLOW ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "status" << Utils::Color::RESET
              << "Inspect working file status against the latest HEAD commit.\n"
              << "                             Reports active branch, modifications, and untracked edits.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite status\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "commit <message>" << Utils::Color::RESET
              << "Record a full snapshot of the tracked file with a commit message.\n"
              << "                             Computes 64-bit FNV-1a hash; refuses duplicate commits.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite commit \"Added thesis statement\"\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "log" << Utils::Color::RESET
              << "Display chronological commit history for the current branch.\n"
              << "                             Shows commit IDs, authors, timestamps, hashes, and messages.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite log\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "log --all" << Utils::Color::RESET
              << "Display commits across ALL branches sorted chronologically.\n"
              << "                             Uses custom stable O(N log N) Merge Sort algorithm.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite log --all\n\n" << Utils::Color::RESET;

    // 3. DIFF ENGINE
    std::cout << Utils::Color::YELLOW << "[ 3. VIEWING CHANGES (DIFF ENGINE) ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "diff" << Utils::Color::RESET
              << "Show line-by-line diff between latest commit and working file.\n"
              << "                             Uses Bottom-Up Dynamic Programming (LCS) with line hashing.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite diff\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "diff <id1> <id2>" << Utils::Color::RESET
              << "Show line-by-line diff between two specific commit IDs.\n"
              << "                             Displays additions (+), removals (-), and line numbers.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite diff 1 2\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "diff-modes <id1> <id2>" << Utils::Color::RESET
              << "Compare execution time & results of Recursive vs Memo vs DP diff.\n"
              << "                             Benchmark comparison on two historical commit versions.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite diff-modes 1 2\n\n" << Utils::Color::RESET;

    // 4. BRANCHING & NAVIGATION
    std::cout << Utils::Color::YELLOW << "[ 4. BRANCHING & NAVIGATION ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "branch <branch_name>" << Utils::Color::RESET
              << "Create a new branch forked from the current commit.\n"
              << "                             Links the new branch into the N-ary Branch Tree topology.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite branch feature-exp\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "branches" << Utils::Color::RESET
              << "List all branches using two tree traversals:\n"
              << "                               * DFS Tree View (hierarchical indentation)\n"
              << "                               * BFS View (level-by-level branch generations)\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite branches\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "checkout <branch_name>" << Utils::Color::RESET
              << "Switch active branch and restore its HEAD commit to the working file.\n"
              << "                             Automatically saves pre-checkout state to Undo stack.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite checkout feature-exp\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "merge <branch_name>" << Utils::Color::RESET
              << "Merge changes from <branch_name> into the currently active branch.\n"
              << "                             Creates a merge commit and updates the working file.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite merge feature-exp\n\n" << Utils::Color::RESET;

    // 5. TIME TRAVEL & RECOVERY
    std::cout << Utils::Color::YELLOW << "[ 5. TIME TRAVEL & RECOVERY (UNDO / REDO) ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "rollback <commit_id>" << Utils::Color::RESET
              << "Restore the working file to the snapshot at <commit_id>.\n"
              << "                             Pushes current file onto Undo stack before overwriting.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite rollback 1\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "undo" << Utils::Color::RESET
              << "Reverse the last rollback or checkout operation.\n"
              << "                             Restores previous file state from persistent Undo stack.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite undo\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "redo" << Utils::Color::RESET
              << "Re-apply an operation that was previously undone.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite redo\n\n" << Utils::Color::RESET;

    // 6. SEARCH & BENCHMARKING
    std::cout << Utils::Color::YELLOW << "[ 6. SEARCH & PERFORMANCE BENCHMARKING ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "search <keyword>" << Utils::Color::RESET
              << "Search entire history across all commits using Knuth-Morris-Pratt.\n"
              << "                             Uses O(N+M) LPS failure table; prints line and column.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite search hypothesis\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "benchmark" << Utils::Color::RESET
              << "Execute algorithm benchmark suite:\n"
              << "                               * LCS Diff: 100, 500, 1000 lines (Recursive vs Memo vs DP)\n"
              << "                               * String Search: KMP vs Brute-force speedup\n"
              << "                               * Hash Table: Load factor and collision metrics\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite benchmark\n\n" << Utils::Color::RESET;

    // 7. SHELL & HELP
    std::cout << Utils::Color::YELLOW << "[ 7. SHELL & HELP ]\n" << Utils::Color::RESET;
    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "commands" << Utils::Color::RESET
              << "Display this detailed command reference manual.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite commands\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "help" << Utils::Color::RESET
              << "Display a concise command overview.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite help\n\n" << Utils::Color::RESET;

    std::cout << "  " << Utils::Color::GREEN << std::left << std::setw(26) << "interactive" << Utils::Color::RESET
              << "Start interactive REPL prompt session.\n"
              << "                             Example: " << Utils::Color::CYAN << "gitlite interactive\n" << Utils::Color::RESET;

    std::cout << Utils::Color::BOLD << Utils::Color::CYAN
              << "========================================================================================\n"
              << Utils::Color::RESET << "\n";
}

void printHelp() {
    printCommandsManual();
}

void runBenchmarkSuite(Repository& repo) {
    std::cout << Utils::Color::BOLD << Utils::Color::CYAN
              << "======================================================================\n"
              << "                  GITLITE ALGORITHM BENCHMARK SUITE                   \n"
              << "======================================================================\n"
              << Utils::Color::RESET;

    // 1. Longest Common Subsequence (LCS) Benchmark
    std::cout << "\n" << Utils::Color::YELLOW << "[1] LCS Diff Algorithm Benchmark (Recursive vs Memoized vs Bottom-Up DP):\n"
              << Utils::Color::RESET;
    std::cout << "----------------------------------------------------------------------\n";

    // Small scale test for recursive comparison
    SafeArray<std::string> smallA, smallB;
    for (int i = 1; i <= 14; ++i) {
        smallA.push_back("Line " + std::to_string(i) + ": content for testing LCS");
        if (i % 2 == 0) {
            smallB.push_back("Line " + std::to_string(i) + ": content for testing LCS");
        } else {
            smallB.push_back("Modified line " + std::to_string(i) + " in version B");
        }
    }

    Utils::Timer tRec;
    int lenRec = DiffEngine::computeLCSRecursive(smallA, smallB);
    double tRecMs = tRec.elapsedMilliseconds();

    Utils::Timer tMemo;
    int lenMemo = DiffEngine::computeLCSMemoized(smallA, smallB);
    double tMemoMs = tMemo.elapsedMilliseconds();

    SafeArray<DiffLine> diffDP;
    Utils::Timer tDP;
    DiffEngine::computeDiff(smallA, smallB, diffDP);
    double tDPMs = tDP.elapsedMilliseconds();

    std::cout << "Small Scale (N = 14 lines):\n";
    std::cout << "  * Recursive LCS:    " << std::fixed << std::setprecision(4) << tRecMs << " ms (LCS length: " << lenRec << ")\n";
    std::cout << "  * Memoized Top-Down:" << std::fixed << std::setprecision(4) << tMemoMs << " ms (LCS length: " << lenMemo << ")\n";
    std::cout << "  * Bottom-Up DP:     " << std::fixed << std::setprecision(4) << tDPMs << " ms (Diff reconstructed)\n";

    // Medium & Large Scale tests (100, 500, 1000 lines) for Bottom-up DP
    int sizes[] = {100, 500, 1000};
    std::cout << "\nScalability Test of Bottom-Up DP (O(N * M)):\n";
    std::cout << std::left << std::setw(18) << "Lines (N x M)"
              << std::setw(24) << "Bottom-Up DP Time"
              << std::setw(24) << "Memoized Time" << "\n";
    std::cout << "----------------------------------------------------------------------\n";

    for (int sz : sizes) {
        SafeArray<std::string> lines1, lines2;
        for (int i = 0; i < sz; ++i) {
            lines1.push_back("Sentence #" + std::to_string(i) + " describing scientific concepts.");
            if (i % 3 != 0) {
                lines2.push_back("Sentence #" + std::to_string(i) + " describing scientific concepts.");
            } else {
                lines2.push_back("Edited sentence #" + std::to_string(i) + " with altered phrasing.");
            }
        }

        Utils::Timer tm;
        DiffEngine::computeLCSMemoized(lines1, lines2);
        double memoTime = tm.elapsedMilliseconds();

        Utils::Timer tdp;
        SafeArray<DiffLine> res;
        DiffEngine::computeDiff(lines1, lines2, res);
        double dpTime = tdp.elapsedMilliseconds();

        std::string dimStr = std::to_string(sz) + " x " + std::to_string(sz);
        std::string dpStr = std::to_string(dpTime) + " ms";
        std::string memoStr = std::to_string(memoTime) + " ms";

        std::cout << std::left << std::setw(18) << dimStr
                  << std::setw(24) << dpStr
                  << std::setw(24) << memoStr << "\n";
    }

    // 2. Search Benchmark: KMP vs Brute Force
    std::cout << "\n" << Utils::Color::YELLOW << "[2] String Search Benchmark: Knuth-Morris-Pratt (KMP) vs Brute-Force:\n"
              << Utils::Color::RESET;
    std::cout << "----------------------------------------------------------------------\n";

    std::string testDoc = "";
    for (int i = 0; i < 2000; ++i) {
        testDoc += "Data structures and algorithms form the bedrock of computing. Sentence " + std::to_string(i) + ". ";
    }
    testDoc += "TARGET_KEYWORD_AT_THE_END_OF_DOCUMENT";

    std::string pattern = "TARGET_KEYWORD_AT_THE_END";

    // Benchmark Brute-Force
    Utils::Timer tBF;
    int bfIdx = 0;
    for (int it = 0; it < 50; ++it) {
        bfIdx = SearchEngine::bruteForceSearch(testDoc, pattern);
    }
    double bfMs = tBF.elapsedMilliseconds() / 50.0;

    // Benchmark KMP
    Utils::Timer tKMP;
    int kmpIdx = 0;
    for (int it = 0; it < 50; ++it) {
        kmpIdx = SearchEngine::kmpSearch(testDoc, pattern);
    }
    double kmpMs = tKMP.elapsedMilliseconds() / 50.0;

    std::cout << "Corpus size: " << testDoc.length() << " chars | Pattern length: " << pattern.length() << " chars\n";
    std::cout << "  * Brute Force Avg Time: " << std::fixed << std::setprecision(4) << bfMs << " ms (Match index: " << bfIdx << ")\n";
    std::cout << "  * KMP Avg Time:         " << std::fixed << std::setprecision(4) << kmpMs << " ms (Match index: " << kmpIdx << ")\n";

    // Repetitive worst-case pattern test (A^k B in A^N)
    std::string repText(10000, 'A');
    repText += "B";
    std::string repPat(100, 'A');
    repPat += "B";

    Utils::Timer tBFRep;
    SearchEngine::bruteForceSearch(repText, repPat);
    double repBFMs = tBFRep.elapsedMilliseconds();

    Utils::Timer tKMPRep;
    SearchEngine::kmpSearch(repText, repPat);
    double repKMPMs = tKMPRep.elapsedMilliseconds();

    std::cout << "\nAdversarial Repetitive Pattern Test (A^100 B in A^10000 B):\n";
    std::cout << "  * Brute Force Time: " << std::fixed << std::setprecision(4) << repBFMs << " ms (Worst-case O(N*M))\n";
    std::cout << "  * KMP Time:         " << std::fixed << std::setprecision(4) << repKMPMs << " ms (Linear O(N+M))\n";
    if (repKMPMs > 0.0) {
        std::cout << "  * KMP Speedup:      " << std::fixed << std::setprecision(2) << (repBFMs / repKMPMs) << "x faster!\n";
    }

    // 3. Hash Table Metrics
    std::cout << "\n" << Utils::Color::YELLOW << "[3] Hash Table Performance & Collisions:\n"
              << Utils::Color::RESET;
    std::cout << "----------------------------------------------------------------------\n";
    const auto& table = repo.getBranchHistories();
    std::cout << "  * Registered Branches: " << table.size() << "\n";
    std::cout << "  * Bucket Count:        " << table.bucket_count() << "\n";
    std::cout << "  * Current Load Factor: " << std::fixed << std::setprecision(2) << table.load_factor() << "\n";
    std::cout << "  * Collisions Detected: " << table.count_collisions() << "\n";
    std::cout << "======================================================================\n\n";
}

bool executeCommand(Repository& repo, const std::vector<std::string>& args) {
    if (args.empty()) return true;

    const std::string& cmd = args[0];

    if (cmd == "init") {
        std::string filename = (args.size() > 1) ? args[1] : "document.txt";
        repo.init(filename);
    } else if (cmd == "delete-repo" || cmd == "delete") {
        bool force = (args.size() > 1 && (args[1] == "--force" || args[1] == "-f"));
        if (!force) {
            std::cout << Utils::Color::YELLOW << "Are you sure you want to delete the GitLite repository in this directory? (y/N): " << Utils::Color::RESET;
            std::string ans;
            if (std::getline(std::cin, ans)) {
                if (ans != "y" && ans != "Y" && ans != "yes" && ans != "YES") {
                    std::cout << "Repository deletion canceled.\n";
                    return true;
                }
            }
        }
        repo.deleteRepo(true);
    } else if (cmd == "author" || cmd == "set-author") {
        if (args.size() < 2) {
            std::cout << "Current commit author: " << Utils::Color::CYAN << repo.getAuthor() << Utils::Color::RESET << "\n";
            std::cout << "To customize author name: gitlite author \"<Your Name>\"\n";
            return true;
        }
        std::string newAuthor = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            newAuthor += " " + args[i];
        }
        repo.setAuthor(newAuthor);
    } else if (cmd == "status") {
        repo.status();
    } else if (cmd == "commit") {
        if (args.size() < 2) {
            std::cerr << Utils::Color::RED << "[ERROR] 'commit' requires a message: gitlite commit <message>"
                      << Utils::Color::RESET << "\n";
            return false;
        }
        std::string msg = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            msg += " " + args[i];
        }
        repo.commit(msg);
    } else if (cmd == "log") {
        bool showAll = (args.size() > 1 && args[1] == "--all");
        repo.log(showAll);
    } else if (cmd == "diff") {
        if (args.size() == 1) {
            repo.diff(0, -1);
        } else if (args.size() == 2) {
            repo.diff(std::stoi(args[1]), -1);
        } else if (args.size() >= 3) {
            repo.diff(std::stoi(args[1]), std::stoi(args[2]));
        }
    } else if (cmd == "diff-modes") {
        if (args.size() < 3) {
            std::cerr << "Usage: gitlite diff-modes <commit1> <commit2>\n";
            return false;
        }
        int id1 = std::stoi(args[1]);
        int id2 = std::stoi(args[2]);
        Commit* c1 = repo.findCommitById(id1);
        Commit* c2 = repo.findCommitById(id2);
        if (!c1 || !c2) {
            std::cerr << Utils::Color::RED << "[ERROR] One or both commits not found in history.\n" << Utils::Color::RESET;
            return false;
        }

        std::cout << "Comparing Diff Modes for Commit #" << id1 << " (" << c1->getLines().size() << " lines) vs Commit #"
                  << id2 << " (" << c2->getLines().size() << " lines):\n\n";

        if (c1->getLines().size() <= 16 && c2->getLines().size() <= 16) {
            Utils::Timer t1;
            int r = DiffEngine::computeLCSRecursive(c1->getLines(), c2->getLines());
            std::cout << "1. Recursive LCS:   " << t1.elapsedMilliseconds() << " ms (LCS: " << r << ")\n";
        } else {
            std::cout << "1. Recursive LCS:   [SKIPPED: Line count > 16 to avoid O(2^(N+M)) exponential delay]\n";
        }

        Utils::Timer t2;
        int m = DiffEngine::computeLCSMemoized(c1->getLines(), c2->getLines());
        std::cout << "2. Memoized LCS:    " << t2.elapsedMilliseconds() << " ms (LCS: " << m << ")\n";

        Utils::Timer t3;
        SafeArray<DiffLine> dres;
        DiffEngine::computeDiff(c1->getLines(), c2->getLines(), dres);
        std::cout << "3. Bottom-Up DP:    " << t3.elapsedMilliseconds() << " ms (Reconstructed diff lines)\n";
    } else if (cmd == "rollback") {
        if (args.size() < 2) {
            std::cerr << "Usage: gitlite rollback <commit_id>\n";
            return false;
        }
        repo.rollback(std::stoi(args[1]));
    } else if (cmd == "undo") {
        repo.undo();
    } else if (cmd == "redo") {
        repo.redo();
    } else if (cmd == "branch") {
        if (args.size() < 2) {
            std::cerr << "Usage: gitlite branch <branch_name>\n";
            return false;
        }
        std::string bname = args[1];
        for (size_t i = 2; i < args.size(); ++i) bname += " " + args[i];
        repo.createBranch(bname);
    } else if (cmd == "branches") {
        repo.printBranches();
    } else if (cmd == "checkout") {
        if (args.size() < 2) {
            std::cerr << "Usage: gitlite checkout <branch_name>\n";
            return false;
        }
        std::string bname = args[1];
        for (size_t i = 2; i < args.size(); ++i) bname += " " + args[i];
        repo.checkout(bname);
    } else if (cmd == "merge") {
        if (args.size() < 2) {
            std::cerr << "Usage: gitlite merge <branch_name>\n";
            return false;
        }
        std::string bname = args[1];
        for (size_t i = 2; i < args.size(); ++i) bname += " " + args[i];
        repo.merge(bname);
    } else if (cmd == "search") {
        if (args.size() < 2) {
            std::cerr << "Usage: gitlite search <keyword>\n";
            return false;
        }
        std::string kw = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            kw += " " + args[i];
        }
        repo.search(kw);
    } else if (cmd == "benchmark") {
        runBenchmarkSuite(repo);
    } else if (cmd == "commands") {
        printCommandsManual();
    } else if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        printCommandsManual();
    } else if (cmd == "exit" || cmd == "quit") {
        return false;
    } else {
        std::cerr << Utils::Color::RED << "Unknown command: '" << cmd << "'. Type 'gitlite commands' to view all available commands."
                  << Utils::Color::RESET << "\n";
    }
    return true;
}

void runInteractiveREPL(Repository& repo) {
    printBanner();
    std::cout << "Entering interactive REPL shell. Type 'commands' for manual, 'exit' to quit.\n\n";

    std::string line;
    while (true) {
        std::string promptBranch = repo.isInitialized() ? repo.getCurrentBranch() : "no-repo";
        std::cout << Utils::Color::CYAN << "gitlite(" << promptBranch << ")> " << Utils::Color::RESET;

        if (!std::getline(std::cin, line)) break;

        // Split into tokens handling basic quotes
        std::vector<std::string> args;
        std::string token;
        bool inQuotes = false;
        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (c == '"' || c == '\'') {
                inQuotes = !inQuotes;
            } else if (std::isspace(static_cast<unsigned char>(c)) && !inQuotes) {
                if (!token.empty()) {
                    args.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            args.push_back(token);
        }

        if (args.empty()) continue;
        if (!executeCommand(repo, args)) {
            break;
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    // 1. Initialize Console: enables VT Processing on Windows or disables ANSI if not supported
    Utils::initConsole();

    // 2. Check for manual color override flags
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-color" || a == "--nocolor") {
            Utils::Color::disable();
        } else {
            args.push_back(a);
        }
    }

    Repository repo(".");

    if (args.empty()) {
        runInteractiveREPL(repo);
        return 0;
    }

    if (args[0] == "interactive") {
        runInteractiveREPL(repo);
        return 0;
    }

    executeCommand(repo, args);
    return 0;
}
