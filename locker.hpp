#pragma once
#include "lockable.hpp"

namespace kt {
///
/// \brief Storage for a T and an associated mutex
///
template <typename T, typename M = std::mutex>
struct locker_t;

///
/// \brief Wrapper modelling a held lock and a (reference to a) T
///
template <typename T, template <typename...> typename L, typename M>
struct locked_t;

///
/// \brief Storage for a T and an associated mutex
///
template <typename T, typename M>
struct locker_t {
	mutable kt::lockable_t<M> mutex;
	T t;

	locker_t() = default;
	locker_t(T&& t) noexcept;
	locker_t(T const& t);

	///
	/// \brief Lock mutex and obtain const reference to T
	///
	template <template <typename...> typename L = std::scoped_lock>
	locked_t<T const, L, M> lock() const;
	///
	/// \brief Lock mutex and obtain reference to T
	///
	template <template <typename...> typename L = std::scoped_lock>
	locked_t<T, L, M> lock();
};

// impl

template <typename T, template <typename...> typename L, typename M>
struct locked_t {
	using lock_t = L<M>;

	lock_t lock;
	T& t;

	locked_t(kt::lockable_t<M>& mutex, T& t) : lock(mutex.template lock<L>()), t(t) {
	}

	T& get() const noexcept {
		return t;
	}

	operator T&() const noexcept {
		return t;
	}
};

template <typename T, template <typename...> typename L, typename M>
struct locked_t<T const, L, M> {
	using lock_t = L<M>;

	lock_t lock;
	T const& t;

	locked_t(kt::lockable_t<M>& mutex, T const& t) : lock(mutex.template lock<L>()), t(t) {
	}

	T const& get() const noexcept {
		return t;
	}

	operator T const &() const noexcept {
		return t;
	}
};

template <typename T, typename M>
locker_t<T, M>::locker_t(T&& t) noexcept : t(std::move(t)) {
}
template <typename T, typename M>
locker_t<T, M>::locker_t(T const& t) : t(t) {
}
template <typename T, typename M>
template <template <typename...> typename L>
locked_t<T const, L, M> locker_t<T, M>::lock() const {
	return locked_t<T const, L, M>(mutex, t);
}
template <typename T, typename M>
template <template <typename...> typename L>
locked_t<T, L, M> locker_t<T, M>::lock() {
	return locked_t<T, L, M>(mutex, t);
}
} // namespace kt
