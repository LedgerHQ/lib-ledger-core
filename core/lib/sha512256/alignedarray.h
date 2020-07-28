/*
This code is written by kerukuro for cppcrypto library (http://cppcrypto.sourceforge.net/)
and released into public domain.
*/

#ifndef CPPCRYPTO_ALIGNED_ARRAY_H
#define CPPCRYPTO_ALIGNED_ARRAY_H

#include <stdlib.h>
#include <memory.h>
#include <algorithm>
#include "portability.h"

namespace cppcrypto
{
	template<typename T, size_t N, size_t A>
	class aligned_pod_array
	{
	public:
		aligned_pod_array();
		~aligned_pod_array();
		operator T*() { return t; }
		T* get() { return t; }
		const T* get() const { return t; }
		void reset();
		size_t size() const { return N; }
		size_t bytes() const { return sizeof(T) * N; }

		aligned_pod_array(aligned_pod_array&& other);
		aligned_pod_array& operator=(aligned_pod_array&& other);
		aligned_pod_array(const aligned_pod_array& other);
		aligned_pod_array& operator=(const aligned_pod_array& other);
	private:
		T* t;
	};

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>::aligned_pod_array() : t(0)
	{
		t = static_cast<T*>(aligned_allocate(sizeof(T) * N, A));
	}

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>::~aligned_pod_array()
	{
		reset();
	}

	template<typename T, size_t N, size_t A>
	void aligned_pod_array<T, N, A>::reset()
	{
		if (t)
		{
			aligned_deallocate(t);
			t = nullptr;
		}
}

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>::aligned_pod_array(aligned_pod_array<T, N, A>&& other)
	{
		t = other.t;
		other.t = nullptr;
	}

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>& aligned_pod_array<T, N, A>::operator=(aligned_pod_array<T, N, A>&& other)
	{
		std::swap(t, other.t);
		return *this;
	}

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>::aligned_pod_array(const aligned_pod_array<T, N, A>& other)
		: t(nullptr)
	{
		t = static_cast<T*>(aligned_allocate(sizeof(T) * N, A));
		*this = other;
	}

	template<typename T, size_t N, size_t A>
	aligned_pod_array<T, N, A>& aligned_pod_array<T, N, A>::operator=(const aligned_pod_array<T, N, A>& other)
	{
		memcpy(t, other.t, sizeof(T) * N);
		return *this;
	}

	template<typename T, size_t A>
	class aligned_impl_ptr
	{
	public:
		aligned_impl_ptr();
		~aligned_impl_ptr();

		template<typename RT>
		void create();
		void destroy();

		operator T*() { return t; }
		T* get() { return t; }
		const T* get() const { return t; }

		T* operator->() const { return t; }

		aligned_impl_ptr(aligned_impl_ptr&& other);
		aligned_impl_ptr& operator=(aligned_impl_ptr&& other);
	private:
		aligned_impl_ptr(const aligned_impl_ptr& other) = delete;
		aligned_impl_ptr& operator=(const aligned_impl_ptr& other) = delete;
		T* t;
	};

	template<typename T, size_t A>
	aligned_impl_ptr<T, A>::aligned_impl_ptr() : t(nullptr)
	{
	}

	template<typename T, size_t A>
	template<typename RT>
	void aligned_impl_ptr<T, A>::create()
	{
		void* p = aligned_allocate(sizeof(RT), A);
		t = new (p)RT;
	}

	template<typename T, size_t A>
	void aligned_impl_ptr<T, A>::destroy()
	{
		if (t)
		{
			t->~T();
			aligned_deallocate(t);
			t = nullptr;
		}
	}

	template<typename T, size_t A>
	aligned_impl_ptr<T, A>::~aligned_impl_ptr()
	{
		destroy();
	}

	template<typename T, size_t A>
	aligned_impl_ptr<T, A>::aligned_impl_ptr(aligned_impl_ptr<T, A>&& other)
	{
		t = other.t;
		other.t = nullptr;
	}

	template<typename T, size_t A>
	aligned_impl_ptr<T, A>& aligned_impl_ptr<T, A>::operator=(aligned_impl_ptr<T, A>&& other)
	{
		std::swap(t, other.t);
		return *this;
	}

#if 0
	template<typename T, size_t A>
	aligned_impl_ptr<T, A>::aligned_impl_ptr(const aligned_impl_ptr<T, A>& other)
	{
		*this = other;
	}

	template<typename T, size_t A>
	aligned_impl_ptr<T, A>& aligned_impl_ptr<T, A>::operator=(const aligned_impl_ptr<T, A>& other)
	{
		t = other.clone();
		return *this;
	}
#endif

}

#endif

