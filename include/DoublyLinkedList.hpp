#ifndef DOUBLYLINKEDLIST_HPP
#define DOUBLYLINKEDLIST_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

/**
 * @brief Templated Doubly Linked List adhering strictly to the Rule of Five.
 * Ideal for linear bidirectional commit histories, rollbacks, and fast insertions.
 */
template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        explicit Node(const T& val, Node* p = nullptr, Node* n = nullptr)
            : data(val), prev(p), next(n) {}
        explicit Node(T&& val, Node* p = nullptr, Node* n = nullptr)
            : data(std::move(val)), prev(p), next(n) {}
    };

private:
    Node* head_;
    Node* tail_;
    size_t size_;

    void clearInternal() {
        Node* current = head_;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

public:
    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    // Destructor
    ~DoublyLinkedList() {
        clearInternal();
    }

    // Copy Constructor
    DoublyLinkedList(const DoublyLinkedList& other) : head_(nullptr), tail_(nullptr), size_(0) {
        Node* curr = other.head_;
        while (curr != nullptr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }

    // Copy Assignment Operator
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            clearInternal();
            Node* curr = other.head_;
            while (curr != nullptr) {
                push_back(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }

    // Move Constructor
    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move Assignment Operator
    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            clearInternal();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    void clear() { clearInternal(); }

    Node* head() noexcept { return head_; }
    const Node* head() const noexcept { return head_; }
    Node* tail() noexcept { return tail_; }
    const Node* tail() const noexcept { return tail_; }

    void push_back(const T& value) {
        Node* newNode = new Node(value, tail_, nullptr);
        if (tail_ != nullptr) {
            tail_->next = newNode;
        } else {
            head_ = newNode;
        }
        tail_ = newNode;
        ++size_;
    }

    void push_back(T&& value) {
        Node* newNode = new Node(std::move(value), tail_, nullptr);
        if (tail_ != nullptr) {
            tail_->next = newNode;
        } else {
            head_ = newNode;
        }
        tail_ = newNode;
        ++size_;
    }

    void push_front(const T& value) {
        Node* newNode = new Node(value, nullptr, head_);
        if (head_ != nullptr) {
            head_->prev = newNode;
        } else {
            tail_ = newNode;
        }
        head_ = newNode;
        ++size_;
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("DoublyLinkedList::pop_back() on empty list");
        }
        Node* toDelete = tail_;
        tail_ = tail_->prev;
        if (tail_ != nullptr) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }
        delete toDelete;
        --size_;
    }

    void pop_front() {
        if (empty()) {
            throw std::out_of_range("DoublyLinkedList::pop_front() on empty list");
        }
        Node* toDelete = head_;
        head_ = head_->next;
        if (head_ != nullptr) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }
        delete toDelete;
        --size_;
    }

    T& front() {
        if (empty()) throw std::out_of_range("DoublyLinkedList::front() on empty list");
        return head_->data;
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("DoublyLinkedList::front() on empty list");
        return head_->data;
    }

    T& back() {
        if (empty()) throw std::out_of_range("DoublyLinkedList::back() on empty list");
        return tail_->data;
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("DoublyLinkedList::back() on empty list");
        return tail_->data;
    }
};

#endif // DOUBLYLINKEDLIST_HPP
