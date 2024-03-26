#pragma once


#include <concepts>
#include <functional>
#include <memory>
#include <bit>


#include "snippet/aliases.hpp"

#include "numeric/arithmetic.hpp"


namespace lib {


// Thanks to: https://tk0-math.hatenablog.com/entry/2020/03/27/194150
template<class ValueType, u32 BUFFER_DEPTH = 20, class Allocator = std::allocator<ValueType>>
struct persistent_queue {
    using value_type = ValueType;
    using size_type = internal::size_t;

    using allocator_type = Allocator;


    struct node_type;
    using node_pointer = std::shared_ptr<node_type>;

    struct node_type {
        value_type value;
        node_pointer prev[BUFFER_DEPTH];
    };

  private:
    size_type _size = 0;
    node_pointer _head = {}, _tail = {};

    [[no_unique_address]] allocator_type _allocator = {};

  public:
    explicit persistent_queue(const allocator_type& allocator = {}) noexcept(NO_EXCEPT)
      : _allocator(allocator)
    {}

    persistent_queue(const persistent_queue& source) noexcept(NO_EXCEPT) = default;
    persistent_queue(persistent_queue&& source) noexcept(NO_EXCEPT) = default;


    persistent_queue(const persistent_queue& source, const allocator_type& allocator) noexcept(NO_EXCEPT)
      : _size(source._size), _head(source._head), _tail(source._tail), _allocator(allocator)
    {}

    persistent_queue(persistent_queue&& source, const allocator_type& allocator) noexcept(NO_EXCEPT)
      : _size(source._size), _head(source._head), _tail(source._tail), _allocator(allocator)
    {}


    inline persistent_queue& operator=(const persistent_queue& source) noexcept(NO_EXCEPT) {
        this->_size = source._size;
        this->_head = source._head, this->_tail = source._tail;
    };

    inline persistent_queue& operator=(persistent_queue&& source) noexcept(NO_EXCEPT) {
        this->_size = source._size;
        this->_head = source._head, this->_tail = source._tail;
    };


    inline auto clone() const noexcept(NO_EXCEPT) { return *this; }


    inline bool empty() const noexcept(NO_EXCEPT) { return this->_size == 0; }
    inline size_type size() const noexcept(NO_EXCEPT) { return this->_size; }


    inline value_type front() const noexcept(NO_EXCEPT) {
        assert(!this->empty());
        return this->_head->value;
    }

    template<std::convertible_to<value_type> T>
        requires std::is_move_constructible_v<T>
    inline value_type front_or(T&& v) const noexcept(NO_EXCEPT) {
        if(this->empty()) return static_cast<value_type>(std::forward<T>(v));
        else return this->front();
    }


    inline value_type back() const noexcept(NO_EXCEPT) {
        assert(!this->empty());
        return this->_tail->value;
    }

    template<std::convertible_to<value_type> T>
        requires std::is_move_constructible_v<T>
    inline value_type back_or(T&& v) const noexcept(NO_EXCEPT) {
        if(this->empty()) return static_cast<value_type>(std::forward<T>(v));
        else return this->back();
    }


    inline auto& clear() noexcept(NO_EXCEPT) {
        this->_head.reset(), this->_tail.reset();
        this->_size = 0;
        return *this;
    }


    template<std::convertible_to<value_type> T>
    auto& push(T&& x) noexcept(NO_EXCEPT) {
        node_pointer node = std::allocate_shared<node_type>(this->_allocator, x);

        node->prev[0] = this->_tail;

        REP(i, 1, BUFFER_DEPTH) {
            node_pointer prev = node->prev[i - 1];
            if(prev) node->prev[i] = prev->prev[i - 1];
            else break;
        }

        if(!this->_head) this->_head = node;
        this->_tail = node;

        ++this->_size;

        return *this;
    }


    auto& pop() noexcept(NO_EXCEPT) {
        assert(!this->empty());

        if(!this->_head || !this->_tail || this->_size == 1) {
            this->clear();
            return *this;
        }

        auto index = to_unsigned(this->_size - 2);
        node_pointer node = this->_tail;

        while(index > 0) {
            const size_type msb = highest_bit_pos(index);
            index -= 1 << msb;
            node = node->prev[msb];
        }

        this->_head = node;
        --this->_size;

        return *this;
    }
};


namespace pmr {


template<class T, u32 BUFFER_DEPTH = 20>
using persistent_queue = lib::persistent_queue<T, BUFFER_DEPTH, std::pmr::polymorphic_allocator<T>>;


} // namesapce pmr


} // namespace lib
