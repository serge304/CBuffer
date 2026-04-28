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

template<class T, size_t N>
    class circular_iterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

public:
    circular_iterator(pointer _data, size_t _ndx)
        : mData(_data), mNdx(_ndx)
    {}

private:
    pointer mData;
	size_t mNdx;

public:
    circular_iterator& operator++()
    {
        ++mNdx;
        if (mNdx == N)
            mNdx = 0;
        return *this;
    }

    circular_iterator operator++(int)
    {
        circular_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const circular_iterator& rhs) const
    {
        return mData == rhs.mData && mNdx == rhs.mNdx;
    }

    bool operator!=(const circular_iterator& rhs) const
    {
        return !(*this == rhs);
    }

    reference operator*() const
    {
        return mData[mNdx];
    }
};

template<typename T, size_t N, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
class circular_buffer
{
public:
	using storage_t = std::array<T, N>;
	using iterator_t = circular_iterator<T, N>;
	using const_iterator_t = circular_iterator<const T, N>;

private:
	storage_t mBuff{};
	size_t mHead{}; // указатель на следующий свободный слот для записи     
	size_t mTail{}; // указатель на самый старый элемент
	bool mFull{};   // флаг заполненности буфера

	T mSum{};

public:
	auto begin()       { return       iterator_t(mBuff.data(), mTail); }
	auto begin() const { return const_iterator_t(mBuff.data(), mTail); }

	auto end()       { return       iterator_t(mBuff.data(), mHead); }
	auto end() const { return const_iterator_t(mBuff.data(), mHead); }

public:
	void reset()
	{
		mHead = 0;
		mTail = 0;
		mFull = false;
		mSum = {};
		mBuff = {};
	}

	bool full() const
	{
		return mFull;
	}

	bool empty() const
	{
		return !mFull && (mHead == mTail);
	}

	size_t capacity() const
	{
		return N;
	};

	size_t size() const
	{
		return mFull ?          N 
		     : mHead >= mTail ? mHead - mTail
			                  : N - mTail + mHead;
	}

	void push(T val)
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

	T pop()
	{
        if (empty())
		    return T{}; 

        T val = mBuff[mTail];
		mSum -= val;
        mTail = (mTail + 1) % N;
        mFull = false;  // после извлечения буфер точно не полон
        return val;
    }

	int direction() const 
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
	T front() const { 
		return empty() ? T{} : mBuff[mTail]; 
	}

	// Последний добавленный элемент
	T back()  const { 
		return empty() ? T{} : mBuff[(mHead > 0 ? mHead : N) - 1]; 
	}

	T sum() const { return mSum; }
	T av() const { return size() > 0 ? mSum / size() : T{}; }
	T min() const { return *(std::min_element(begin(), end())); }
	T max() const { return *(std::max_element(begin(), end())); }
	T spread() const { return std::abs(max() - min()); }
};

#endif /* SEQUENCE_H_ */
