#ifndef ENGINE_UTILS_NOALLOC_VECTOR_HPP
#define ENGINE_UTILS_NOALLOC_VECTOR_HPP

#include <array>
#include <stdexcept>
#include "detail/smallest_uint.hpp"

namespace engine {
    template<typename T, size_t MAX_SIZE>
    class noalloc_vector {
        alignas(T) std::array<std::byte, MAX_SIZE * sizeof(T)> m_arr;
        detail::smallest_uint_t<MAX_SIZE> m_size;

        T* nth(size_t i) {
            return reinterpret_cast<T*>(m_arr.data() + i * sizeof(T));
        }
        const T* nth(size_t i) const {
            return reinterpret_cast<T*>(m_arr.data() + i * sizeof(T));
        }
    public:
        struct empty_error : std::runtime_error {};

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = T*;
        using const_pointer = const T*;

        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        noalloc_vector() : m_size(0) {}
        noalloc_vector(const std::initializer_list<T>& il) : m_size(il.size()) {
            std::copy(il.begin(), il.end(), m_arr.begin());
        }
        ~noalloc_vector() { clear(); }

        static size_t max_size() { return MAX_SIZE; }
        static size_t capacity() { return MAX_SIZE; }
        size_t size() const { return m_size; }

        void clear() {
            for(size_t i = 0; i < m_size; i++) {
                nth(i)->~T();
            }
            m_size = 0;
        }

        bool empty() const { return m_size == 0; }

        template<typename... Ts>
        void emplace_back(Ts&&...args) {
            if(m_size >= MAX_SIZE) {
                throw std::length_error("stack_array::emplace_back called with size() >= max_size()");
            }
            T* stack_top = nth(m_size);
            new(stack_top) T (std::forward<Ts&&...>(args)...);
            m_size++;
        }
        void push_back(T el) {
            emplace_back(std::move(el));
        }

        T pop_back() {
            if(m_size == 0) {
                throw empty_error("noalloc_vector::pop_back called with size() == 0");
            }
            T ret = std::move(nth(m_size-1));
            nth(m_size-1)->~T();
            m_size--;
            return ret;
        }

        T& front() {
            if(m_size == 0) {
                throw empty_error("noalloc_vector::front called with size() == 0");
            }
            return *nth(0);
        }
        const T& front() const {
            if(m_size == 0) {
                throw empty_error("noalloc_vector::front called with size() == 0");
            }
            return *nth(0);
        }
        T& back() {
            if(m_size == 0) {
                throw empty_error("noalloc_vector::back called with size() == 0");
            }
            return *nth(m_size - 1);
        }
        const T& back() const {
            if(m_size == 0) {
                throw empty_error("noalloc_vector::back called with size() == 0");
            }
            return *nth(m_size - 1);
        }

        T& operator[](size_t i) {
            EXPECTS(i < m_size && i < MAX_SIZE);
            return *nth(i);
        }
        const T& operator[](size_t i) const {
            EXPECTS(i < m_size && i < MAX_SIZE);
            return *nth(i);
        }
        T& at(size_t i) {
            if(i >= m_size)
                throw std::out_of_range("out of range access through noalloc_vector::at");
            return this->operator[](i);
        }
        const T& at(size_t i) const {
            if(i >= m_size)
                throw std::out_of_range("out of range access through noalloc_vector::at");
            return this->operator[](i);
        }

        T* data() { return nth(0); }
        const T* data() const { return nth(0); }

        iterator emplace(const_iterator pos, auto&&... args) {
            if(m_size >= MAX_SIZE) {
                throw std::length_error("stack_array::emplace called with size() >= max_size()");
            }
            iterator it = end();

            m_size++;

            while(it > pos) {
                new(it) T(std::move(it-1));
                it--;
                it->~T();
            }

            new(it) T(std::forward(args)...);

            return it;
        }

        iterator insert(const_iterator pos, T&& v) { return emplace(pos, std::forward(v)); }
        iterator insert(const_iterator pos, const T& v) { return emplace(pos, std::forward(v)); }
        //more complex forms of insert (multiple/range inserts) are not yet implemented

        iterator erase(iterator pos) {
            if(pos < begin() || pos >= end())
                throw std::out_of_range("noalloc_vector::erase called with an iterator not in [begin, end)");

            iterator it = pos;

            while (it != end() - 1) {
                it->~T();
                *it = std::move(++it);
            }
            it->~T();
            m_size--;

            return pos;
        }
        iterator erase(const_iterator pos) {
            return erase(const_cast<iterator>(pos)); //this const cast is fine: we have mutable access to the object anyway
        }
        //more complex forms of erase (with ranges) are not yet implemented

        void resize(size_t count, const T& value = T()) {
            if(count > MAX_SIZE) {
                throw std::length_error("stack_array::resize called with count >= max_size()");
            }

            while(count < m_size) {
                nth(--m_size)->~T();
            }
            while(count > m_size) {
                new(nth(m_size++)) T(value);
            }
        }
        void resize(size_t count) {
            while(count < m_size) {
                nth(--m_size)->~T();
            }
            while(count > m_size) {
                new(nth(m_size++)) T();
            }
        }

        void swap(noalloc_vector& o) {
            // this allows us to assume that this->size() <= o.size()
            if(o.size() < size()) {
                o.swap(*this);
                return;
            }

            for(size_t i = 0; i < this->size(); i++) {
                std::swap((*this)[i], o[i]);
            }
            for(size_t i = this->size(); i < o.size(); i++) {
                new(nth(i)) T(std::move(o[i]));
                o[i].~T();
            }

            std::swap(this->m_size, o.m_size);
        }

        bool contains(const T& v) {
            for(const T& e : *this)
                if(e == v)
                    return true;
            return false;
        }

        iterator begin() { return nth(0); }
        const_iterator begin() const { return nth(0); }
        const_iterator cbegin() const { return nth(0); }

        iterator end() { return nth(m_size); }
        const_iterator end() const { return nth(m_size); }
        const_iterator cend() const { return nth(m_size); }

        reverse_iterator rbegin() { return end(); }
        const_reverse_iterator rbegin() const { return cend(); }
        const_reverse_iterator crbegin() const { return cend(); }

        reverse_iterator rend() { return begin(); }
        const_reverse_iterator rend() const { return cbegin(); }
        const_reverse_iterator crend() const { return cbegin(); }
    };
}

#endif // ENGINE_UTILS_NOALLOC_VECTOR_HPP
