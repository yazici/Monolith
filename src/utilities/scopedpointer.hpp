#pragma once

/// \brief Delete a resource end of scope.
/// \details In contrast to a unique_ptr you can duplicate the pointer...
///		but be sure you do not reference the resource after scope is left.
template<typename T>
class ScopedPtr
{
public:
	/// \brief Only construction from typed pointer (implicit too).
	ScopedPtr(T* _ptr) : m_ptr(_ptr) {}

	/// \brief Delete the resource!
	~ScopedPtr() { delete m_ptr; }

	/// \brief This swap function can only be found by ADL.
	friend void swap(ScopedPtr& _lhs, ScopedPtr& _rhs)
	{
		T* tmp = _lhs.m_ptr;
		_lhs.m_ptr = _rhs.m_ptr;
		_rhs.m_ptr = tmp;
	}

	/// \brief Moving allowed
	ScopedPtr(ScopedPtr&& _rhs) : m_ptr(nullptr) {
		m_ptr = _rhs.m_ptr;
		_rhs.m_ptr = nullptr;
	}
	ScopedPtr& operator = (ScopedPtr&& _rhs) {
		swap(*this, _rhs);
		return *this;
	}

	/// \brief Auto cast
	operator T* () { return m_ptr; }
	operator const T* () const { return m_ptr; }

	T* operator -> () { return m_ptr; }
	T& operator * () { return *m_ptr; }

	operator bool () { return m_ptr != nullptr; }
private:
	T* m_ptr;

	// More than one deleting reference not allowed
	ScopedPtr(const ScopedPtr&);
	void operator = (const ScopedPtr&);
};