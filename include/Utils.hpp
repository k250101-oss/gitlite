#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <cstdlib>
#include "SafeArray.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Utils {

    inline bool g_colorEnabled = true;

    // ANSI terminal color codes
    namespace Color {
        inline const char* RESET   = "\033[0m";
        inline const char* BOLD    = "\033[1m";
        inline const char* RED     = "\033[31m";
        inline const char* GREEN   = "\033[32m";
        inline const char* YELLOW  = "\033[33m";
        inline const char* BLUE    = "\033[34m";
        inline const char* MAGENTA = "\033[35m";
        inline const char* CYAN    = "\033[36m";
        inline const char* WHITE   = "\033[37m";
        inline const char* GRAY    = "\033[90m";

        inline void disable() {
            RESET   = "";
            BOLD    = "";
            RED     = "";
            GREEN   = "";
            YELLOW  = "";
            BLUE    = "";
            MAGENTA = "";
            CYAN    = "";
            WHITE   = "";
            GRAY    = "";
            g_colorEnabled = false;
        }

        inline void enable() {
            RESET   = "\033[0m";
            BOLD    = "\033[1m";
            RED     = "\033[31m";
            GREEN   = "\033[32m";
            YELLOW  = "\033[33m";
            BLUE    = "\033[34m";
            MAGENTA = "\033[35m";
            CYAN    = "\033[36m";
            WHITE   = "\033[37m";
            GRAY    = "\033[90m";
            g_colorEnabled = true;
        }
    }

    /**
     * @brief Initializes the console environment.
     * On Windows, enables Virtual Terminal Processing so ANSI color escape sequences
     * work properly in cmd.exe. If VT processing is not supported or output is redirected,
     * cleanly disables ANSI escape sequences so no raw `[36m` symbols are printed.
     */
    inline void initConsole() {
        // Check NO_COLOR environment variable (standard cross-platform convention)
        const char* noColorEnv = std::getenv("NO_COLOR");
        if (noColorEnv != nullptr && noColorEnv[0] != '\0') {
            Color::disable();
            return;
        }

#ifdef _WIN32
        // Always attempt to enable Virtual Terminal Processing.
        // Even if SetConsoleMode fails (e.g. older build of cmd.exe), modern
        // Windows Terminal and most recent cmd.exe versions honour the flag.
        // We therefore keep colors ENABLED regardless of SetConsoleMode's return
        // value and only fall back to disabling them when stdout is not a tty at
        // all (i.e. redirected to a file / pipe).
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        bool isTTY = false;
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                // stdout IS a real console (not a pipe/file)
                isTTY = true;
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);   // best-effort; ignore return value
            }
        }

        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        if (hErr != INVALID_HANDLE_VALUE && hErr != NULL) {
            DWORD dwErrMode = 0;
            if (GetConsoleMode(hErr, &dwErrMode)) {
                dwErrMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hErr, dwErrMode);
            }
        }

        // Set console codepage to UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        if (!isTTY) {
            // Output is redirected to a file or pipe — strip ANSI codes.
            Color::disable();
        } else {
            // Real console: keep colors on (VT flag already set above).
            Color::enable();
        }
#else
        Color::enable();
#endif
    }

    /**
     * @brief Computes 64-bit FNV-1a hash of a single string.
     */
    inline uint64_t hashString(const std::string& str) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : str) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    /**
     * @brief Converts uint64_t hash to a 16-character hexadecimal string.
     */
    inline std::string hashToHex(uint64_t hash) {
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }

    /**
     * @brief Computes a combined hash across all lines of a file snapshot.
     */
    inline std::string computeContentHash(const SafeArray<std::string>& lines) {
        uint64_t combined = 14695981039346656037ULL;
        for (size_t i = 0; i < lines.size(); ++i) {
            uint64_t lineHash = hashString(lines[i]);
            combined ^= lineHash;
            combined *= 1099511628211ULL;
            // Also factor in line index to ensure order sensitivity
            combined ^= static_cast<uint64_t>(i);
            combined *= 1099511628211ULL;
        }
        return hashToHex(combined);
    }

    /**
     * @brief Returns current timestamp formatted as readable string.
     */
    inline std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
        return std::string(buf);
    }

    /**
     * @brief Reads a text file line-by-line into a SafeArray<std::string>.
     */
    inline bool readFileLines(const std::string& filepath, SafeArray<std::string>& lines) {
        lines.clear();
        std::ifstream fin(filepath);
        if (!fin.is_open()) {
            return false;
        }
        std::string line;
        while (std::getline(fin, line)) {
            // Strip carriage return if present (Windows CRLF normalization)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        return true;
    }

    /**
     * @brief Writes a SafeArray<std::string> to a text file.
     */
    inline bool writeFileLines(const std::string& filepath, const SafeArray<std::string>& lines) {
        std::ofstream fout(filepath);
        if (!fout.is_open()) {
            return false;
        }
        for (size_t i = 0; i < lines.size(); ++i) {
            fout << lines[i] << "\n";
        }
        return true;
    }

    /**
     * @brief Simple timer class using std::chrono for benchmarking algorithms.
     */
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_;
    public:
        Timer() { reset(); }
        void reset() { start_ = std::chrono::high_resolution_clock::now(); }
        double elapsedMicroseconds() const {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::micro>(end - start_).count();
        }
        double elapsedMilliseconds() const {
            return elapsedMicroseconds() / 1000.0;
        }
    };

} // namespace Utils

#endif // UTILS_HPP
