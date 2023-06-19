#pragma once

#ifndef RESOURCE_NO_ATOMIC
#include <atomic>
#endif

#ifdef _RESOURCE_HPP_DEV
#include <iostream>
#define _RESOURCE_HPP_DEV_LOG(x, ...) printf(x "\n", ## __VA_ARGS__)
#else
#define _RESOURCE_HPP_DEV_LOG()
#endif

struct ReferenceInfo {
	bool exists = true;
#ifndef RESOURCE_NO_ATOMIC
	std::atomic<size_t> num_references = 0;
#else
	size_t num_references = 0;
#endif
};

template <typename T>
class Resource;

template <typename T>
class ResourceReference {

	T *_type_ptr = nullptr;
	ReferenceInfo *_ref_info = nullptr;

	friend ResourceReference<T> Resource<T>::createReference() noexcept;
	ResourceReference(ReferenceInfo *info, T *type) noexcept: _ref_info(info), _type_ptr(type) {
		_ref_info->num_references++;
	}

	void _ref_inc() noexcept {
		if(_ref_info) _ref_info->num_references++;
	}
	void _ref_dec() noexcept {
		if(_ref_info) {
			_ref_info->num_references--;
			if(!_ref_info->num_references && !_ref_info->exists) delete _ref_info;
		}
	}

public:

	~ResourceReference() noexcept {
		_ref_dec();
	}

	ResourceReference() noexcept = default;
	ResourceReference(nullptr_t) noexcept {}

	ResourceReference(const ResourceReference &copy) noexcept: _ref_info(copy._ref_info), _type_ptr(copy._type_ptr) {
		_ref_inc();
	};

	ResourceReference(ResourceReference &&other) noexcept {
		swap(other);
	};

	ResourceReference &operator =(const ResourceReference &copy) noexcept {
		ResourceReference(copy).swap(*this);
		return *this;
	}

	ResourceReference &operator =(ResourceReference &&other) noexcept {
		ResourceReference(std::move(other)).swap(*this);
		return *this;
	}

	ResourceReference &operator =(nullptr_t) noexcept {
		reset();
		return *this;
	}

	void reset() noexcept {
		ResourceReference().swap(*this);
	}

	void swap(ResourceReference &other) noexcept {
		std::swap(_ref_info, other._ref_info);
		std::swap(_type_ptr, other._type_ptr);
	}

	bool exists() const noexcept { return _ref_info && _ref_info->exists; }
	T *get() const noexcept { return _type_ptr; }

	const T *operator->() const noexcept { return get(); }
	const T &operator *() const noexcept { return *get(); }
	T *operator->() noexcept { return get(); }
	T &operator *() noexcept { return *get(); }
	operator bool() const noexcept { return exists(); }
	bool operator ==(const ResourceReference<T> &other) const noexcept { return _ref_info && _ref_info == other._ref_info; }

	inline long ref_count() const { return _ref_info ? _ref_info->num_references.load() : 0ull; }
};


template <typename T>
class Resource {
	T _type{};
	ReferenceInfo *_ref_info = nullptr;
public:

	Resource() = default;

	template<class... _Types>
	Resource(_Types&&... _Args): _type(std::forward<_Types>(_Args)...) {}

	Resource(const T &tc): _type(tc) {}
	Resource(T &&tm): _type(std::move(tm)) {}
	Resource(const Resource<T> &) = delete;
	Resource(Resource<T> &&) = delete;

	Resource<T> &operator = (const T &tc) { _type = tc; return *this; }
	Resource<T> &operator = (T &&tm) { _type = std::move(tm); return *this; }
	Resource<T> &operator = (const Resource<T> &) = delete;
	Resource<T> &operator = (Resource<T> &&) = delete;

	~Resource() {
		if(_ref_info) {
			if(!_ref_info->num_references) delete _ref_info;
			else _ref_info->exists = false;
		}
	}

	const T &get() const noexcept { return _type; }
	T &get() noexcept { return _type; }

	const T *operator->() const noexcept { return &get(); }
	const T &operator *() const noexcept { return get(); }
	T *operator->() noexcept { return &get(); }
	T &operator *() noexcept { return get(); }

	inline long ref_count() const noexcept { return _ref_info->num_references; }

	ResourceReference<T> createReference() noexcept {
		if(!_ref_info) _ref_info = new ReferenceInfo;
		return ResourceReference<T>(_ref_info, &_type);
	}
	ResourceReference<T> operator &() noexcept { return createReference(); }
};


template <class _Elem, class _Traits, class T>
std::basic_ostream<_Elem, _Traits> &operator<<(std::basic_ostream<_Elem, _Traits> &_Out, const ResourceReference<T> &ref) {
	return _Out << ref.get();
}

template <class _Elem, class _Traits, class T>
std::basic_ostream<_Elem, _Traits> &operator<<(std::basic_ostream<_Elem, _Traits> &_Out, const Resource<T> &res) {
	return _Out << res.get();
}