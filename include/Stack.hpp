#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * @brief Templated Stack container adhering strictly to the Rule of Five.
 * Backed by a dynamic array with automatic growth.
 * Used for Undo and Redo operations in GitLite.
 */
template <typename T>
class Stack {
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
    explicit Stack(size_t initial_capacity = 8)
        : data_(nullptr), capacity_(initial_capacity > 0 ? initial_capacity : 4), size_(0) {
        data_ = new T[capacity_];
    }

    ~Stack() {
        delete[] data_;
        data_ = nullptr;
        capacity_ = 0;
        size_ = 0;
    }

    Stack(const Stack& other)
        : data_(nullptr), capacity_(other.capacity_), size_(other.size_) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < other.size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    Stack& operator=(const Stack& other) {
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

    Stack(Stack&& other) noexcept
        : data_(other.data_), capacity_(other.capacity_), size_(other.size_) {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    Stack& operator=(Stack&& other) noexcept {
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
    bool empty() const noexcept { return size_ == 0; }

    void clear() {
        size_ = 0;
    }

    void push(const T& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void push(T&& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = std::move(value);
    }

    void pop() {
        if (empty()) {
            throw std::out_of_range("Stack::pop() called on empty stack");
        }
        --size_;
    }

    T& top() {
        if (empty()) {
            throw std::out_of_range("Stack::top() called on empty stack");
        }
        return data_[size_ - 1];
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Stack::top() called on empty stack");
        }
        return data_[size_ - 1];
    }

    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Stack::at() index out of bounds");
        }
        return data_[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Stack::at() index out of bounds");
        }
        return data_[index];
    }

    T& operator[](size_t index) {
        return at(index);
    }

    const T& operator[](size_t index) const {
        return at(index);
    }
};

#endif // STACK_HPP
