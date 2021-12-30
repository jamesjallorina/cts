// Copyright (c) 2021-present cppnetwork
// All Rights Reserved
//
// Distributed under the "MIT License". See the accompanying LICENSE.rst file.

#ifndef CTS_H
#define CTS_H

#define CTS_VERSION_MAJOR 1
#define CTS_VERSION_MINOR 0
#define CTS_VERSION_PATCH 0

#include <mutex>
#include <chrono>
#include <type_traits>
#include <utility>

#if defined(HAVE_THREAD_SAFETY_ATTRIBUTES)
// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#	define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#	define THREAD_ANNOTATION_ATTRIBUTE__(x) // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define RELEASE_GENERIC(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)


#ifdef USE_LOCK_STYLE_THREAD_SAFETY_ATTRIBUTES
// The original version of thread safety analysis the following attribute
// definitions.  These use a lock-based terminology.  They are still in use
// by existing thread safety code, and will continue to be supported.

// Deprecated.
#define PT_GUARDED_VAR \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_var)

// Deprecated.
#define GUARDED_VAR \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_var)

// Replaced by REQUIRES
#define EXCLUSIVE_LOCKS_REQUIRED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

// Replaced by REQUIRES_SHARED
#define SHARED_LOCKS_REQUIRED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_locks_required(__VA_ARGS__))

// Replaced by CAPABILITY
#define LOCKABLE \
  THREAD_ANNOTATION_ATTRIBUTE__(lockable)

// Replaced by SCOPED_CAPABILITY
#define SCOPED_LOCKABLE \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

// Replaced by ACQUIRE
#define EXCLUSIVE_LOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

// Replaced by ACQUIRE_SHARED
#define SHARED_LOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))

// Replaced by RELEASE and RELEASE_SHARED
#define UNLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

// Replaced by TRY_ACQUIRE
#define EXCLUSIVE_TRYLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

// Replaced by TRY_ACQUIRE_SHARED
#define SHARED_TRYLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))

// Replaced by ASSERT_CAPABILITY
#define ASSERT_EXCLUSIVE_LOCK(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_exclusive_lock(__VA_ARGS__))

// Replaced by ASSERT_SHARED_CAPABILITY
#define ASSERT_SHARED_LOCK(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_lock(__VA_ARGS__))

// Replaced by EXCLUDE_CAPABILITY.
#define LOCKS_EXCLUDED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

// Replaced by RETURN_CAPABILITY
#define LOCK_RETURNED(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#endif  // USE_LOCK_STYLE_THREAD_SAFETY_ATTRIBUTES

namespace cts {

template <class ...T>
using void_t = void;

template <class T, class = void>
struct is_mutex : std::false_type {};

template <class T>
struct is_mutex<T, void_t< 
                          decltype(std::declval<T>().lock()),
                          decltype(std::declval<T>().unlock()),
						  decltype(std::declval<T>().native_handle())>
                        > : std::true_type {};

class CAPABILITY("mutex") mutex
{
public:
	using native_handle_type = std::mutex::native_handle_type;

    constexpr mutex() noexcept = default;
    ~mutex() = default;
    mutex& operator=(const mutex&) = delete;

    void lock() ACQUIRE() { return m_mutex.lock(); }
    bool try_lock() TRY_ACQUIRE(true) { return m_mutex.try_lock(); }
    void unlock() RELEASE() { return m_mutex.unlock(); }

	native_handle_type native_handle() { return m_mutex.native_handle(); }

private:
    std::mutex m_mutex;
};

template<class Mutex = mutex, template<class> class UniqueLock = std::unique_lock>
class SCOPED_CAPABILITY unique_lock
{
public:
	using mutex_type = Mutex;
	template<class T>
	using lock_type = UniqueLock<T>;

	static_assert(is_mutex<Mutex>::value, "Mutex parameter does not meet the requirements");
	
	unique_lock() noexcept = default;

	unique_lock(unique_lock &&other) noexcept : 
		m_unique_lock(std::move(other.m_unique_lock)) 
	{ 
		m_mu = other.m_mu;
		other.m_mu = nullptr;
	}
	
	explicit unique_lock(mutex_type &m) ACQUIRE(m): 
		m_unique_lock(m) 
	{
		m_mu = &m;
	}
	
	unique_lock(mutex_type &m, std::defer_lock_t t) noexcept EXCLUDES(m): 
		m_unique_lock(m, t)
	{
		m_mu = &m;
	}
	
	unique_lock(mutex_type &m, std::try_to_lock_t t) ACQUIRE_SHARED(m): 
		m_unique_lock(m, t)
	{
		m_mu = &m;
	}
	
	unique_lock(mutex_type &m, std::adopt_lock_t t) REQUIRES_SHARED(m): 
		m_unique_lock(m, t)
	{
		m_mu = &m;
	}
	
	template< class Rep, class Period >
	unique_lock( mutex_type& m,
             	const std::chrono::duration<Rep, Period>& timeout_duration ) TRY_ACQUIRE(true, m) : 
		m_unique_lock(m, timeout_duration)
	{
		m_mu = &m;
	}

	template< class Clock, class Duration >
	unique_lock( mutex_type& m,
				const std::chrono::time_point<Clock,Duration>& timeout_time ) TRY_ACQUIRE(true, m):
		m_unique_lock(m, timeout_time)
	{
		m_mu = &m;
	}

	unique_lock& operator=(unique_lock &&other)
	{
		m_unique_lock(std::move(other.m_unique_lock));
	}

	~unique_lock() RELEASE() = default;

	void lock() ACQUIRE() 
	{
		return m_unique_lock.lock(); 
	}
	
	bool try_lock() TRY_ACQUIRE(true) 
	{
		return m_unique_lock.try_lock(); 
	}

	template< class Rep, class Period >
	bool try_lock_for(const std::chrono::duration<Rep,Period>& timeout_duration) 
			TRY_ACQUIRE(true)
	{
		return m_unique_lock.try_lock_for(timeout_duration);
	}

	template< class Clock, class Duration >
	bool try_lock_until(const std::chrono::time_point<Clock,Duration>& timeout_time) 
			TRY_ACQUIRE(true)
	{
		return m_unique_lock(timeout_time);
	}

	void swap(unique_lock& other) noexcept
	{
		return m_unique_lock.swap(other.m_unique_lock);
	}

	mutex_type *release() noexcept RETURN_CAPABILITY(m_mu)
	{
		return m_unique_lock.release();
	}

	mutex_type *mutex() const noexcept RETURN_CAPABILITY(m_mu)
	{
		return m_unique_lock.mutex();
	}

	bool owns_lock() const noexcept
	{
		return m_unique_lock.owns_lock();
	}

	explicit operator bool() const noexcept
	{
		return m_unique_lock.owns_lock();
	}

private:
	mutex_type *m_mu = nullptr;
	lock_type<mutex_type> m_unique_lock;
};

template<class Mutex = mutex, template<class> class LockGuard = std::lock_guard>
class SCOPED_CAPABILITY lock_guard
{
public:
	using mutex_type = Mutex;
	template<class T>
	using lock_type = LockGuard<T>;

	static_assert(is_mutex<Mutex>::value, "Mutex parameter does not meet the requirements");

	explicit lock_guard(mutex_type& m) ACQUIRE(m) : 
		m_lock_guard(m) 
	{
		m_mu = &m;
	}

	lock_guard(mutex_type& m, std::adopt_lock_t t) REQUIRES_SHARED(m) :
		m_lock_guard(m, t)
	{
		m_mu = &m;
	}

	lock_guard( const lock_guard& ) = delete;

	~lock_guard() RELEASE() = default;

private:
	mutex_type *m_mu;
	lock_type<mutex_type> m_lock_guard;
};

}   // namespace cts

#else
/// @brief Nullify THREAD SAFETY ANNOTATIONS

#define CAPABILITY(x)

#define SCOPED_CAPABILITY

#define GUARDED_BY(x)

#define PT_GUARDED_BY(x)

#define ACQUIRED_BEFORE(...)

#define ACQUIRED_AFTER(...)

#define REQUIRES(...)

#define REQUIRES_SHARED(...)

#define ACQUIRE(...)

#define ACQUIRE_SHARED(...)

#define RELEASE(...)

#define RELEASE_SHARED(...)

#define RELEASE_GENERIC(...)

#define TRY_ACQUIRE(...)

#define TRY_ACQUIRE_SHARED(...)

#define EXCLUDES(...)

#define ASSERT_CAPABILITY(x)

#define ASSERT_SHARED_CAPABILITY(x)

#define RETURN_CAPABILITY(x)

#define NO_THREAD_SAFETY_ANALYSIS

namespace cts{

using mutex = std::mutex;

template <class Mutex>
using unique_lock = std::unique_lock<Mutex>;

template <class Mutex>
using lock_guard = std::lock_guard<Mutex>;

}	// namespace cts


#endif // HAVE_THREAD_SAFETY_ATTRIBUTES

#endif // CTS_H
