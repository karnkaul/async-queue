// KT header-only library
// Requirements: C++17

#pragma once
#include <mutex>

namespace kt {
///
/// \brief Wrapper for a mutex and associated locks
///
template <typename Mutex = std::mutex>
struct lockable final {
	Mutex mutex;

	///
	/// \brief Obtain a Lock on mutex
	///
	template <template <typename...> typename Lock = std::scoped_lock>
	Lock<Mutex> lock() {
		return Lock<Mutex>(mutex);
	}
};
} // namespace kt
