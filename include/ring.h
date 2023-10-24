#pragma once
#include <deque>
#include <iterator> /* distance */
#include <initializer_list>
#include <type_traits> /* decay_t  */
#include <mutex> /* lock_guard scoped_lock */

template<class T, class Allocator = std::allocator<T>>
class Ring : public std::deque<T, Allocator> {
public:

    using deque = std::deque<T, Allocator>;
    using lock_guard = std::lock_guard<std::mutex>;

    Ring(size_t capacity = 100000) : _capacity(capacity) {}

    virtual ~Ring() {}

    Ring(std::initializer_list<T> list) : deque(list) {
        _capacity = std::distance(list.begin(), list.end());
    }

    template <class I>
    Ring(I first, I last) : deque(first, last) {
        _capacity = std::distance(first, last);
    }

    size_t max_size() const noexcept {
        return _capacity;
    }

    void resize(size_t size) {
        lock_guard lock(_mutex);
        _capacity = size;
        deque::resize(size);
    }

    void push_front(T &&value) {
        lock_guard lock(_mutex);
        deque::push_front(value);
        if (deque::size() > _capacity)
            deque::pop_back();
    }

    void push_front(const T &value) {
        lock_guard lock(_mutex);
        deque::push_front(value);
        if (deque::size() > _capacity)
            deque::pop_back();
    }

    template <class... Args>
    void emplace_front(Args&&... args) {
        lock_guard lock(_mutex);
        deque::emplace_front(std::forward<Args>(args)...);
        if (deque::size() > _capacity)
            deque::pop_back();
    }

    void push_back(T &&value) {
        lock_guard lock(_mutex);
        deque::push_back(value);
        if (deque::size() > _capacity)
            deque::pop_front();
    }

    void push_back(const T &value) {
        lock_guard lock(_mutex);
        deque::push_back(value);
        if (deque::size() > _capacity)
            deque::pop_front();
    }

    template <class... Args>
    void emplace_back(Args&&... args) {
        lock_guard lock(_mutex);
        deque::emplace_back(std::forward<Args>(args)...);
        if (deque::size() > _capacity)
            deque::pop_front();
    }

    void shrink_to_fit() {
        lock_guard lock(_mutex);
        deque::shrink_to_fit();
        _capacity = deque::size();
    }

    std::optional<T> pop_front() {
        lock_guard lock(_mutex);
        if (deque::empty())
            return std::nullopt;
        T value = deque::front();
        deque::pop_front();
        return value;
    }

    std::optional<T> pop_back() {
        lock_guard lock(_mutex);
        if (deque::empty())
            return std::nullopt;
        T value = deque::back();
        deque::pop_back();
        return value;
    }

    void swap(Ring& that) {
        if (this == &that) return;
        std::scoped_lock lock(_mutex, that._mutex);
        std::deque<T>::swap(that);
    }

    template <class I>
    void assign(I first, I last) {
        lock_guard lock(_mutex);
        deque::assign(first, last);
        if (auto size = deque::size();
                size > _capacity)
            _capacity = size;
    }

    Ring& operator=(const Ring& that) {
        if (this == &that) return *this;
        std::scoped_lock lock(_mutex, that._mutex);
        std::deque<T>::operator=(that);
        return *this;
    }

// -----------------------------------------------------------------------------

    using std::deque<T, Allocator>::operator=;
    using iterator = typename std::deque<T>::iterator;
    using const_iterator = typename deque::const_iterator;
    using reverse_iterator = typename deque::reverse_iterator;
    using const_reverse_iterator = typename deque::const_reverse_iterator;

    T& operator[](size_t) = delete;
    const T& operator[](size_t) const = delete;
    T& at(size_t) = delete;
    const T& at(size_t) const = delete;
    T& front() = delete;
    const T& front() const = delete;
    T& back() = delete;
    const T& back() const = delete;
    iterator begin() noexcept = delete;
    const_iterator begin() const noexcept = delete;
    iterator end() noexcept = delete;
    const_iterator end() const noexcept = delete;
    reverse_iterator rbegin() noexcept = delete;
    const_reverse_iterator rbegin() const noexcept = delete;
    reverse_iterator rend() noexcept = delete;
    const_reverse_iterator rend() const noexcept = delete;
    iterator erase(iterator) = delete;
    iterator erase(iterator, iterator) = delete;
    template <class... Args> void emplace(const_iterator, Args&&...) = delete;
    template <class... Args> void insert(const_iterator, Args&&...) = delete;

private:
    size_t _capacity;
    mutable std::mutex _mutex;
};

// deduction guide
template <class I> Ring(I b, I e) -> Ring<std::decay_t<decltype(*b)>>;
