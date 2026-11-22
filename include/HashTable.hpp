#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include <cstddef>
#include <string>
#include <stdexcept>
#include <utility>

/**
 * @brief Default hash functor using FNV-1a algorithm for std::string and basic types.
 */
template <typename Key>
struct DefaultHash {
    size_t operator()(const Key& key) const {
        // Fallback for general types
        return std::hash<Key>{}(key);
    }
};

template <>
struct DefaultHash<std::string> {
    size_t operator()(const std::string& str) const {
        // FNV-1a 64-bit hash
        size_t hash = 14695981039346656037ULL;
        for (char c : str) {
            hash ^= static_cast<unsigned char>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

/**
 * @brief Templated separate-chaining Hash Table with automatic dynamic rehashing.
 * Strictly adheres to the Rule of Five.
 * Tracks load factor and collision statistics.
 */
template <typename Key, typename Value, typename Hash = DefaultHash<Key>>
class HashTable {
public:
    struct Entry {
        Key key;
        Value value;
        Entry* next;

        Entry(const Key& k, const Value& v, Entry* n = nullptr)
            : key(k), value(v), next(n) {}
        Entry(Key&& k, Value&& v, Entry* n = nullptr)
            : key(std::move(k)), value(std::move(v)), next(n) {}
    };

private:
    Entry** buckets_;
    size_t bucket_count_;
    size_t size_;
    float max_load_factor_;
    Hash hasher_;

    void clearInternal() {
        if (buckets_ != nullptr) {
            for (size_t i = 0; i < bucket_count_; ++i) {
                Entry* curr = buckets_[i];
                while (curr != nullptr) {
                    Entry* next = curr->next;
                    delete curr;
                    curr = next;
                }
            }
            delete[] buckets_;
            buckets_ = nullptr;
        }
        size_ = 0;
        bucket_count_ = 0;
    }

    size_t getBucketIndex(const Key& key, size_t num_buckets) const {
        return hasher_(key) % num_buckets;
    }

public:
    explicit HashTable(size_t initial_buckets = 16, float max_load_factor = 0.75f)
        : buckets_(nullptr), bucket_count_(initial_buckets > 0 ? initial_buckets : 16),
          size_(0), max_load_factor_(max_load_factor) {
        buckets_ = new Entry*[bucket_count_]();
    }

    ~HashTable() {
        clearInternal();
    }

    // Copy Constructor
    HashTable(const HashTable& other)
        : buckets_(nullptr), bucket_count_(other.bucket_count_),
          size_(0), max_load_factor_(other.max_load_factor_), hasher_(other.hasher_) {
        buckets_ = new Entry*[bucket_count_]();
        for (size_t i = 0; i < other.bucket_count_; ++i) {
            Entry* curr = other.buckets_[i];
            while (curr != nullptr) {
                insert(curr->key, curr->value);
                curr = curr->next;
            }
        }
    }

    // Copy Assignment Operator
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            clearInternal();
            bucket_count_ = other.bucket_count_;
            max_load_factor_ = other.max_load_factor_;
            hasher_ = other.hasher_;
            buckets_ = new Entry*[bucket_count_]();
            for (size_t i = 0; i < other.bucket_count_; ++i) {
                Entry* curr = other.buckets_[i];
                while (curr != nullptr) {
                    insert(curr->key, curr->value);
                    curr = curr->next;
                }
            }
        }
        return *this;
    }

    // Move Constructor
    HashTable(HashTable&& other) noexcept
        : buckets_(other.buckets_), bucket_count_(other.bucket_count_),
          size_(other.size_), max_load_factor_(other.max_load_factor_), hasher_(std::move(other.hasher_)) {
        other.buckets_ = nullptr;
        other.bucket_count_ = 0;
        other.size_ = 0;
    }

    // Move Assignment Operator
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            clearInternal();
            buckets_ = other.buckets_;
            bucket_count_ = other.bucket_count_;
            size_ = other.size_;
            max_load_factor_ = other.max_load_factor_;
            hasher_ = std::move(other.hasher_);
            other.buckets_ = nullptr;
            other.bucket_count_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return size_; }
    size_t bucket_count() const noexcept { return bucket_count_; }
    bool empty() const noexcept { return size_ == 0; }
    float load_factor() const noexcept {
        return bucket_count_ > 0 ? static_cast<float>(size_) / bucket_count_ : 0.0f;
    }

    void clear() {
        if (buckets_ != nullptr) {
            for (size_t i = 0; i < bucket_count_; ++i) {
                Entry* curr = buckets_[i];
                while (curr != nullptr) {
                    Entry* next = curr->next;
                    delete curr;
                    curr = next;
                }
                buckets_[i] = nullptr;
            }
        }
        size_ = 0;
    }

    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= size_) {
            new_bucket_count = size_ * 2 + 1;
        }
        if (new_bucket_count < 8) new_bucket_count = 8;

        Entry** new_buckets = new Entry*[new_bucket_count]();
        for (size_t i = 0; i < bucket_count_; ++i) {
            Entry* curr = buckets_[i];
            while (curr != nullptr) {
                Entry* next = curr->next;
                size_t new_idx = hasher_(curr->key) % new_bucket_count;
                curr->next = new_buckets[new_idx];
                new_buckets[new_idx] = curr;
                curr = next;
            }
        }
        delete[] buckets_;
        buckets_ = new_buckets;
        bucket_count_ = new_bucket_count;
    }

    void insert(const Key& key, const Value& value) {
        if (load_factor() >= max_load_factor_) {
            rehash(bucket_count_ * 2);
        }

        size_t idx = getBucketIndex(key, bucket_count_);
        Entry* curr = buckets_[idx];
        while (curr != nullptr) {
            if (curr->key == key) {
                curr->value = value; // Update existing
                return;
            }
            curr = curr->next;
        }

        // Insert at beginning of bucket chain
        Entry* newEntry = new Entry(key, value, buckets_[idx]);
        buckets_[idx] = newEntry;
        ++size_;
    }

    Value* find(const Key& key) {
        if (bucket_count_ == 0) return nullptr;
        size_t idx = getBucketIndex(key, bucket_count_);
        Entry* curr = buckets_[idx];
        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    const Value* find(const Key& key) const {
        if (bucket_count_ == 0) return nullptr;
        size_t idx = getBucketIndex(key, bucket_count_);
        Entry* curr = buckets_[idx];
        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    bool contains(const Key& key) const {
        return find(key) != nullptr;
    }

    bool remove(const Key& key) {
        if (bucket_count_ == 0) return false;
        size_t idx = getBucketIndex(key, bucket_count_);
        Entry* curr = buckets_[idx];
        Entry* prev = nullptr;

        while (curr != nullptr) {
            if (curr->key == key) {
                if (prev != nullptr) {
                    prev->next = curr->next;
                } else {
                    buckets_[idx] = curr->next;
                }
                delete curr;
                --size_;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    Value& operator[](const Key& key) {
        Value* existing = find(key);
        if (existing != nullptr) {
            return *existing;
        }
        insert(key, Value());
        return *find(key);
    }

    size_t count_collisions() const {
        size_t collisions = 0;
        for (size_t i = 0; i < bucket_count_; ++i) {
            size_t chain_len = 0;
            Entry* curr = buckets_[i];
            while (curr != nullptr) {
                ++chain_len;
                curr = curr->next;
            }
            if (chain_len > 1) {
                collisions += (chain_len - 1);
            }
        }
        return collisions;
    }

    // Callback visitor for iteration
    template <typename Func>
    void forEach(Func func) const {
        for (size_t i = 0; i < bucket_count_; ++i) {
            Entry* curr = buckets_[i];
            while (curr != nullptr) {
                func(curr->key, curr->value);
                curr = curr->next;
            }
        }
    }
};

#endif // HASHTABLE_HPP
