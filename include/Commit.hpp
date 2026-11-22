#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "SafeArray.hpp"
#include "Utils.hpp"

/**
 * @brief Represents a snapshot commit in GitLite.
 * Holds file lines, metadata, author, hash, and parent pointers.
 * Implements the Rule of Five.
 */
class Commit {
private:
    int id_;
    std::string branch_;
    int parentId_;
    std::string timestamp_;
    std::string message_;
    std::string author_;
    std::string hash_;
    SafeArray<std::string> lines_;

public:
    Commit()
        : id_(0), branch_("main"), parentId_(0),
          timestamp_(""), message_(""), author_("Adeel, Yahya & Asmaar"),
          hash_(""), lines_(8) {}

    Commit(int id, const std::string& branch, int parentId,
           const std::string& msg, const SafeArray<std::string>& fileLines,
           const std::string& author = "Adeel, Yahya & Asmaar")
        : id_(id), branch_(branch), parentId_(parentId),
          timestamp_(Utils::getCurrentTimestamp()), message_(msg),
          author_(author.empty() ? "Adeel, Yahya & Asmaar" : author),
          lines_(fileLines) {
        hash_ = Utils::computeContentHash(lines_);
    }

    // Copy Constructor
    Commit(const Commit& other)
        : id_(other.id_), branch_(other.branch_), parentId_(other.parentId_),
          timestamp_(other.timestamp_), message_(other.message_),
          author_(other.author_), hash_(other.hash_), lines_(other.lines_) {}

    // Copy Assignment Operator
    Commit& operator=(const Commit& other) {
        if (this != &other) {
            id_ = other.id_;
            branch_ = other.branch_;
            parentId_ = other.parentId_;
            timestamp_ = other.timestamp_;
            message_ = other.message_;
            author_ = other.author_;
            hash_ = other.hash_;
            lines_ = other.lines_;
        }
        return *this;
    }

    // Move Constructor
    Commit(Commit&& other) noexcept
        : id_(other.id_), branch_(std::move(other.branch_)), parentId_(other.parentId_),
          timestamp_(std::move(other.timestamp_)), message_(std::move(other.message_)),
          author_(std::move(other.author_)), hash_(std::move(other.hash_)),
          lines_(std::move(other.lines_)) {
        other.id_ = 0;
        other.parentId_ = 0;
    }

    // Move Assignment
    Commit& operator=(Commit&& other) noexcept {
        if (this != &other) {
            id_ = other.id_;
            branch_ = std::move(other.branch_);
            parentId_ = other.parentId_;
            timestamp_ = std::move(other.timestamp_);
            message_ = std::move(other.message_);
            author_ = std::move(other.author_);
            hash_ = std::move(other.hash_);
            lines_ = std::move(other.lines_);
            other.id_ = 0;
            other.parentId_ = 0;
        }
        return *this;
    }

    ~Commit() = default;

    // Getters
    int getId() const noexcept { return id_; }
    const std::string& getBranch() const noexcept { return branch_; }
    int getParentId() const noexcept { return parentId_; }
    const std::string& getTimestamp() const noexcept { return timestamp_; }
    const std::string& getMessage() const noexcept { return message_; }
    const std::string& getAuthor() const noexcept { return author_; }
    const std::string& getHash() const noexcept { return hash_; }
    const SafeArray<std::string>& getLines() const noexcept { return lines_; }
    SafeArray<std::string>& getLines() noexcept { return lines_; }

    // Setters for deserialization
    void setId(int id) { id_ = id; }
    void setBranch(const std::string& b) { branch_ = b; }
    void setParentId(int p) { parentId_ = p; }
    void setTimestamp(const std::string& t) { timestamp_ = t; }
    void setMessage(const std::string& m) { message_ = m; }
    void setAuthor(const std::string& a) { author_ = a; }
    void setHash(const std::string& h) { hash_ = h; }

    /**
     * @brief Serializes commit to a text file in .gitlite/commits/
     */
    bool saveToFile(const std::string& filepath) const {
        std::ofstream fout(filepath);
        if (!fout.is_open()) return false;

        fout << "COMMIT_ID=" << id_ << "\n";
        fout << "BRANCH=" << branch_ << "\n";
        fout << "PARENT_ID=" << parentId_ << "\n";
        fout << "TIMESTAMP=" << timestamp_ << "\n";
        fout << "AUTHOR=" << author_ << "\n";
        fout << "HASH=" << hash_ << "\n";
        fout << "MESSAGE=" << message_ << "\n";
        fout << "LINES_COUNT=" << lines_.size() << "\n";
        fout << "---CONTENT---\n";
        for (size_t i = 0; i < lines_.size(); ++i) {
            fout << lines_[i] << "\n";
        }
        return true;
    }

    /**
     * @brief Deserializes commit from file.
     */
    bool loadFromFile(const std::string& filepath) {
        std::ifstream fin(filepath);
        if (!fin.is_open()) return false;

        std::string line;
        lines_.clear();
        author_ = "Adeel, Yahya & Asmaar"; // Default if not found in legacy commits
        bool inContent = false;

        while (std::getline(fin, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (inContent) {
                lines_.push_back(line);
                continue;
            }

            if (line == "---CONTENT---") {
                inContent = true;
                continue;
            }

            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);

            if (key == "COMMIT_ID") {
                id_ = std::stoi(val);
            } else if (key == "BRANCH") {
                branch_ = val;
            } else if (key == "PARENT_ID") {
                parentId_ = std::stoi(val);
            } else if (key == "TIMESTAMP") {
                timestamp_ = val;
            } else if (key == "AUTHOR") {
                author_ = val;
            } else if (key == "HASH") {
                hash_ = val;
            } else if (key == "MESSAGE") {
                message_ = val;
            }
        }
        return true;
    }
};

#endif // COMMIT_HPP
