#ifndef SAFEARRAY_HPP
#define SAFEARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * @brief Templated dynamic array with explicit bounds-checking and automatic doubling.
 * Strictly adheres to the Rule of Five (Destructor, Copy/Move Constructors, Copy/Move Assignment).
 */
template <typename T>
class SafeArray {
private:
    T* data_;
    size_t capacity_;
    size_t size_;

    void resize(size_t new_capacity) {
        if (new_capacity < 4) new_capacity = 4;
        T* new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    explicit SafeArray(size_t initial_capacity = 8)
        : data_(nullptr), capacity_(initial_capacity > 0 ? initial_capacity : 4), size_(0) {
        data_ = new T[capacity_];
    }

    // Destructor
    ~SafeArray() {
        delete[] data_;
        data_ = nullptr;
        capacity_ = 0;
        size_ = 0;
    }

    // Copy Constructor
    SafeArray(const SafeArray& other)
        : data_(nullptr), capacity_(other.capacity_), size_(other.size_) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy Assignment Operator
    SafeArray& operator=(const SafeArray& other) {
        if (this != &other) {
            T* new_data = new T[other.capacity_];
            for (size_t i = 0; i < other.size_; ++i) {
                new_data[i] = other.data_[i];
            }
            delete[] data_;
            data_ = new_data;
            capacity_ = other.capacity_;
            size_ = other.size_;
        }
        return *this;
    }

    // Move Constructor
    SafeArray(SafeArray&& other) noexcept
        : data_(other.data_), capacity_(other.capacity_), size_(other.size_) {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    // Move Assignment Operator
    SafeArray& operator=(SafeArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void clear() {
        size_ = 0;
    }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void push_back(T&& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = std::move(value);
    }

    void pop_back() {
        if (size_ == 0) {
            throw std::out_of_range("SafeArray::pop_back() called on empty array");
        }
        --size_;
    }

    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("SafeArray::at() index out of bounds");
        }
        return data_[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("SafeArray::at() index out of bounds");
        }
        return data_[index];
    }

    T& operator[](size_t index) {
        return at(index); // Explicit bounds checking as required for SafeArray
    }

    const T& operator[](size_t index) const {
        return at(index);
    }

    T& front() {
        if (empty()) throw std::out_of_range("SafeArray::front() called on empty array");
        return data_[0];
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("SafeArray::front() called on empty array");
        return data_[0];
    }

    T& back() {
        if (empty()) throw std::out_of_range("SafeArray::back() called on empty array");
        return data_[size_ - 1];
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("SafeArray::back() called on empty array");
        return data_[size_ - 1];
    }

    T* begin() noexcept { return data_; }
    const T* begin() const noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
    const T* end() const noexcept { return data_ + size_; }
};

#endif // SAFEARRAY_HPP
