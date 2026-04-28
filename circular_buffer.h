/*
 * circular_buffer.h
 *
 *  Created on: Oct 2, 2019
 *      Author: sergey
 *
 *  Циклический буфер со статической длиной
 *
 */

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <stddef.h>
#include <type_traits>
#include <array>
#include <algorithm>
#include <iterator>
#include <cmath>

// Хелперы для проверки наличия операторов (SFINAE для C++14)
template<typename T, typename = void>
struct has_plus : std::false_type {};

template<typename T>
struct has_plus<T, decltype(std::declval<T&>() + std::declval<T&>(), void())> : std::true_type {};

template<typename T, typename = void>
struct has_minus : std::false_type {};

template<typename T>
struct has_minus<T, decltype(std::declval<T&>() - std::declval<T&>(), void())> : std::true_type {};

template<typename T, typename = void>
struct has_less : std::false_type {};

template<typename T>
struct has_less<T, decltype(std::declval<T&>() < std::declval<T&>(), void())> : std::true_type {};

template<typename T, typename = void>
struct has_abs : std::false_type {};

template<typename T>
struct has_abs<T, decltype(std::abs(std::declval<T&>()), void())> : std::true_type {};

// Комплексная проверка: тип должен быть копируемым и иметь нужные операторы
template<typename T>
struct is_buffer_compatible : std::integral_constant<bool,
    std::is_copy_constructible<T>::value &&
    std::is_copy_assignable<T>::value &&
    has_plus<T>::value &&
    has_minus<T>::value &&
    has_less<T>::value &&
    has_abs<T>::value
> {};

template<typename T, size_t N, typename std::enable_if<is_buffer_compatible<T>::value, bool>::type = true>
class circular_buffer
{
public:
    // Стандартные алиасы типов для интеграции со STL
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

    private:
        const circular_buffer* mBuffer;
        size_t mIndex;   // Текущий индекс в массиве (0..N-1)
        size_t mCount;   // Сколько элементов уже пройдено

    public:
        iterator(const circular_buffer* buffer, size_t index, size_t count)
            : mBuffer(buffer), mIndex(index), mCount(count)
        {}

        iterator& operator++() noexcept
        {
            mIndex = (mIndex + 1) % N;
            ++mCount;
            return *this;
        }

        iterator operator++(int) noexcept
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& rhs) const noexcept
        {
            return mCount == rhs.mCount;
        }

        bool operator!=(const iterator& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        reference operator*() const noexcept
        {
            return const_cast<reference>(mBuffer->mBuff[mIndex]);
        }

        pointer operator->() const noexcept
        {
            return const_cast<pointer>(&mBuffer->mBuff[mIndex]);
        }
    };

    using const_iterator = iterator;

private:
    using storage_t = std::array<T, N>;
    storage_t mBuff{};
    size_t mHead{}; // указатель на следующий свободный слот для записи
    size_t mTail{}; // указатель на самый старый элемент
    bool mFull{};   // флаг заполненности буфера

    T mSum{};

public:
    /**
     * Возвращает итератор на первый элемент буфера.
     *
     * Логика получения begin():
     * - mTail указывает на самый старый элемент в буфере (первый для обхода)
     * - Начальный индекс итератора = mTail
     * - Счётчик пройденных элементов = 0 (начало обзора)
     *
     * Пример: если буфер содержит [5, 1, 3] (где 5 - самый старый),
     * то begin() вернёт итератор, указывающий на элемент 5.
     */
    iterator begin() noexcept       { return iterator(this, mTail, 0); }

    /**
     * Возвращает итератор, указывающий на позицию ПОСЛЕ последнего элемента.
     *
     * Логика получения end():
     * - size() возвращает текущее количество элементов в буфере
     * - Конечный индекс = (mTail + size()) % N
     *   Это вычисляет позицию сразу за последним элементом с учётом цикличности
     * - Счётчик пройденных элементов = size() (все элементы пройдены)
     *
     * Пример: если буфер размером 5 содержит [5, 1, 3] (size=3),
     * и mTail=2, то end() будет указывать на индекс (2+3)%5=0,
     * а счётчик будет равен 3 (количество элементов).
     *
     * Важно: сравнение итераторов происходит по счётчику mCount,
     * поэтому итератор достигнет end(), когда пройдёт ровно size() элементов.
     */
    iterator end() noexcept         { return iterator(this, (mTail + size()) % N, size()); }

    const_iterator begin() const noexcept { return const_iterator(this, mTail, 0); }
    const_iterator end()   const noexcept { return const_iterator(this, (mTail + size()) % N, size()); }

    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend()   const noexcept { return end(); }

public:
        void reset() noexcept
        {
                mHead = 0;
                mTail = 0;
                mFull = false;
                mSum = {};
                mBuff = {};
        }

        bool full() const noexcept
        {
                return mFull;
        }

        bool empty() const noexcept
        {
                return !mFull && (mHead == mTail);
        }

        size_t capacity() const noexcept
        {
                return N;
        };

        size_t size() const noexcept
        {
                return mFull ?          N
                     : mHead >= mTail ? mHead - mTail
                                          : N - mTail + mHead;
        }

        void push(T val) noexcept
        {
                if (capacity() == 0)
                        return;

                mSum += val;
                if (full())
                        mSum -= mBuff[mTail];

                mBuff[mHead] = val;

                if (mFull)
                        mTail = (mTail + 1) % N;
                mHead = (mHead + 1) % N;
                mFull = (mHead == mTail);
        }

        T pop() noexcept
        {
        if (empty())
                    return T{};

        T val = mBuff[mTail];
                mSum -= val;
        mTail = (mTail + 1) % N;
        mFull = false;  // после извлечения буфер точно не полон
        return val;
    }

        int direction() const noexcept
        {
                if (size() <= 1) return 0;

                int trend = 0; // 0=неопределено, 1=возрастает, -1=убывает

                auto current_it = begin();
                auto next_it = std::next(current_it);

                while (next_it != end()) {
                        auto current = *current_it++;
                        auto next = *next_it++;

                        if (current < next) {
                                if (trend == -1) return 0; // направление изменилось
                                trend = 1;
                        } else if (current > next) {
                                if (trend == 1) return 0; // направление изменилось
                                trend = -1;
                        }
                        // Если current == next, оставляем trend без изменений
                }
                return trend;
        }

        // самый старый добавленный элемент, будет удален при следующей вставке
        T front() const noexcept {
                return empty() ? T{} : mBuff[mTail];
        }

        // Последний добавленный элемент
        T back()  const noexcept {
                return empty() ? T{} : mBuff[(mHead > 0 ? mHead : N) - 1];
        }

        T sum() const noexcept { return mSum; }
        T av() const noexcept { return size() > 0 ? mSum / size() : T{}; }
        T min() const noexcept {
                if (size() == 0) return T{};
                return *(std::min_element(begin(), end()));
        }
        T max() const noexcept {
                if (size() == 0) return T{};
                return *(std::max_element(begin(), end()));
        }
        T spread() const noexcept {
                if (size() == 0) return T{};
                return std::abs(max() - min());
        }
};

#endif /* SEQUENCE_H_ */
