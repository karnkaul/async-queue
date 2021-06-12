// KT header-only library
// Requirements: C++17

#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace kt {
///
/// \brief FIFO std::deque wrapper with thread safe API
///
/// Features:
/// 	- Thread-safe push-and-notify
/// 	- Thread-safe wait-and-pop
/// 	- Clear queue and return residue
/// 	- Deactivate queue (as secondary wait condition)
///
template <typename T, typename Mutex = std::mutex>
class async_queue {
  public:
	using type = T;
	using mutex_t = Mutex;

	async_queue() = default;
	virtual ~async_queue() { clear(); }

	///
	/// \brief Move a T to the back of the queue and notify one
	///
	void push(T&& t);
	///
	/// \brief Copy a T to the back of the queue and notify one
	///
	void push(T const& t);
	///
	/// \brief Emplace a T to the back of the queue and notify one
	///
	template <typename... U>
	void emplace(U&&... u);
	///
	/// \brief Forward Ts from a container to the back of the queue and notify one
	///
	template <template <typename...> typename C, typename... Args>
	void push(C<T, Args...>&& ts);
	///
	/// \brief Pop a T from the front of the queue, wait until populated / not active
	///
	std::optional<T> pop();
	///
	/// \brief Flush the queue, notify all, and obtain any residual items
	/// \param active Set m_active after moving items
	/// \returns Residual items that were still in the queue
	///
	std::deque<T> clear(bool active = false);
	///
	/// \brief Check whether queue is empty
	///
	bool empty() const;
	///
	/// \brief Check whether queue is active
	///
	bool active() const;
	///
	/// \brief Set active/inactive
	///
	void active(bool set);

  protected:
	///
	/// \brief Used to wait on pop() and notify on push()/clear()
	///
	std::condition_variable m_cv;
	std::deque<T> m_queue;
	mutable Mutex m_mutex;
	///
	/// \brief Used to halt / deactivate queue
	///
	bool m_active = true;
};

template <typename T, typename Mutex>
void async_queue<T, Mutex>::push(T&& t) {
	emplace<T>(std::move(t));
}

template <typename T, typename Mutex>
void async_queue<T, Mutex>::push(T const& t) {
	emplace<T>(t);
}

template <typename T, typename Mutex>
template <typename... U>
void async_queue<T, Mutex>::emplace(U&&... u) {
	{
		std::scoped_lock lock(m_mutex);
		if (m_active) { m_queue.emplace_back(std::forward<U>(u)...); }
	}
	m_cv.notify_one();
}

template <typename T, typename Mutex>
template <template <typename...> typename C, typename... Args>
void async_queue<T, Mutex>::push(C<T, Args...>&& ts) {
	{
		std::scoped_lock lock(m_mutex);
		if (m_active) { std::move(ts.begin(), ts.end(), std::back_inserter(m_queue)); }
	}
	m_cv.notify_all();
}

template <typename T, typename Mutex>
std::optional<T> async_queue<T, Mutex>::pop() {
	std::unique_lock lock(m_mutex);
	m_cv.wait(lock, [this]() -> bool { return !m_queue.empty() || !m_active; });
	if (m_active && !m_queue.empty()) {
		auto ret = std::move(m_queue.front());
		m_queue.pop_front();
		return ret;
	}
	return {};
}

template <typename T, typename Mutex>
std::deque<T> async_queue<T, Mutex>::clear(bool active) {
	decltype(m_queue) ret;
	{
		std::scoped_lock lock(m_mutex);
		ret = std::move(m_queue);
		m_queue.clear();
		m_active = active;
	}
	m_cv.notify_all();
	return ret;
}

template <typename T, typename Mutex>
bool async_queue<T, Mutex>::empty() const {
	std::scoped_lock lock(m_mutex);
	return m_queue.empty();
}

template <typename T, typename Mutex>
bool async_queue<T, Mutex>::active() const {
	std::scoped_lock lock(m_mutex);
	return m_active;
}

template <typename T, typename Mutex>
void async_queue<T, Mutex>::active(bool set) {
	{
		std::scoped_lock lock(m_mutex);
		m_active = set;
	}
	m_cv.notify_all();
}
} // namespace kt
