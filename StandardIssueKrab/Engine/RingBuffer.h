#pragma once

#include <stdexcept>
#include <array>

template<class T, std::size_t N>
class RingBuffer
{
private:
	std::array<T, N> container_;
	std::size_t count_;

public:
	RingBuffer() = default;

	// Returns: index of element
	std::size_t PushBack(const T& val) noexcept;
	std::size_t PushBack(T&& val = T{}) noexcept;

	// Unchecked access
	T& operator[](std::size_t idx) noexcept;
	T const& operator[](std::size_t idx) const noexcept;	
	
	// Throwing checked access
	T& At(std::size_t idx);
	T const& At(std::size_t idx) const;

	// Non-throwing checked access
	T* TryAt(std::size_t idx) noexcept;
	T const* TryAt(std::size_t idx) const noexcept;
};

template<class T, std::size_t N>
std::size_t RingBuffer<T, N>::PushBack(const T& val) noexcept {
	container_[count_ % N] = val;
	std::size_t idx = count_;
	++count_;
	return idx;
}

template<class T, std::size_t N>
std::size_t RingBuffer<T, N>::PushBack(T&& val) noexcept {
	container_[count_ % N] = std::move(val);
	std::size_t idx = count_;
	++count_;
	return idx;
}

template<class T, std::size_t N>
T& RingBuffer<T, N>::operator[](std::size_t idx) noexcept {
	return container_[idx % N];
}

template<class T, std::size_t N>
T const& RingBuffer<T, N>::operator[](std::size_t idx) const noexcept {
	return container_[idx % N];
}

template<class T, std::size_t N>
T& RingBuffer<T, N>::At(std::size_t idx) {
	if ( count_ - idx > N || idx >= count_ ) {
		throw std::out_of_range{ "Index has been invalidated." };
	}

	return container_[idx % N];
}

template<class T, std::size_t N>
T const& RingBuffer<T, N>::At(std::size_t idx) const {
	if ( count_ - idx > N || idx >= count_ ) {
		throw std::out_of_range{ "Index has been invalidated." };
	}

	return container_[idx % N];
}

template<class T, std::size_t N>
T* RingBuffer<T, N>::TryAt(std::size_t idx) noexcept {
	if ( count_ - idx > N || idx >= count_ ) {
		return nullptr;
	}

	return &container_[idx % N];
}

template<class T, std::size_t N>
T const* RingBuffer<T, N>::TryAt(std::size_t idx) const noexcept {
	if ( count_ - idx > N || idx >= count_ ) {
		return nullptr;
	}

	return &container_[idx % N];
}