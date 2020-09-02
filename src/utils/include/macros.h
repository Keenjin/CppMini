// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains macros and macro-like constructs (e.g., templates) that
// are commonly used throughout Chromium source. (It may also contain things
// that are closely related to things that are commonly used that belong in this
// file.)

#ifndef BASE_MACROS_H_
#define BASE_MACROS_H_

#include <type_traits>

#undef max
#undef min

// Put this in the declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName) \
  TypeName(const TypeName&) = delete

// Put this in the declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName) TypeName& operator=(const TypeName&) = delete

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  DISALLOW_COPY(TypeName);                 \
  DISALLOW_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
// This is especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                           \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   std::unique_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     ignore_result(my_var.release());
//
template<typename T>
inline void ignore_result(const T&) {
}

namespace utils {

	// A wrapper that makes it easy to create an object of type T with static
	// storage duration that:
	// - is only constructed on first access
	// - never invokes the destructor
	// in order to satisfy the styleguide ban on global constructors and
	// destructors.
	//
	// Runtime constant example:
	// const std::string& GetLineSeparator() {
	//  // Forwards to std::string(size_t, char, const Allocator&) constructor.
	//   static const base::NoDestructor<std::string> s(5, '-');
	//   return *s;
	// }
	//
	// More complex initialization with a lambda:
	// const std::string& GetSessionNonce() {
	//   static const base::NoDestructor<std::string> nonce([] {
	//     std::string s(16);
	//     crypto::RandString(s.data(), s.size());
	//     return s;
	//   }());
	//   return *nonce;
	// }
	//
	// NoDestructor<T> stores the object inline, so it also avoids a pointer
	// indirection and a malloc. Also note that since C++11 static local variable
	// initialization is thread-safe and so is this pattern. Code should prefer to
	// use NoDestructor<T> over:
	// - A function scoped static T* or T& that is dynamically initialized.
	// - A global base::LazyInstance<T>.
	//
	// Note that since the destructor is never run, this *will* leak memory if used
	// as a stack or member variable. Furthermore, a NoDestructor<T> should never
	// have global scope as that may require a static initializer.
	template <typename T>
	class NoDestructor {
	public:
		// Not constexpr; just write static constexpr T x = ...; if the value should
		// be a constexpr.
		template <typename... Args>
		explicit NoDestructor(Args&&... args) {
			new (storage_) T(std::forward<Args>(args)...);
		}

		// Allows copy and move construction of the contained type, to allow
		// construction from an initializer list, e.g. for std::vector.
		explicit NoDestructor(const T& x) { new (storage_) T(x); }
		explicit NoDestructor(T&& x) { new (storage_) T(std::move(x)); }

		NoDestructor(const NoDestructor&) = delete;
		NoDestructor& operator=(const NoDestructor&) = delete;

		~NoDestructor() = default;

		const T& operator*() const { return *get(); }
		T& operator*() { return *get(); }

		const T* operator->() const { return get(); }
		T* operator->() { return get(); }

		const T* get() const { return reinterpret_cast<const T*>(storage_); }
		T* get() { return reinterpret_cast<T*>(storage_); }

	private:
		alignas(T) char storage_[sizeof(T)];
	};

	template <typename T>
	class AutoReset {
	public:
		template <typename U>
		AutoReset(T* scoped_variable, U&& new_value)
			: scoped_variable_(scoped_variable),
			original_value_(
				std::exchange(*scoped_variable_, std::forward<U>(new_value))) {}

		AutoReset(AutoReset&& other)
			: scoped_variable_(std::exchange(other.scoped_variable_, nullptr)),
			original_value_(std::move(other.original_value_)) {}

		AutoReset& operator=(AutoReset&& rhs) {
			scoped_variable_ = std::exchange(rhs.scoped_variable_, nullptr);
			original_value_ = std::move(rhs.original_value_);
			return *this;
		}

		~AutoReset() {
			if (scoped_variable_)
				*scoped_variable_ = std::move(original_value_);
		}

	private:
		T* scoped_variable_;
		T original_value_;
	};

	// bit_cast<Dest,Source> is a template function that implements the equivalent
	// of "*reinterpret_cast<Dest*>(&source)".  We need this in very low-level
	// functions like the protobuf library and fast math support.
	//
	//   float f = 3.14159265358979;
	//   int i = bit_cast<int32_t>(f);
	//   // i = 0x40490fdb
	//
	// The classical address-casting method is:
	//
	//   // WRONG
	//   float f = 3.14159265358979;            // WRONG
	//   int i = * reinterpret_cast<int*>(&f);  // WRONG
	//
	// The address-casting method actually produces undefined behavior according to
	// the ISO C++98 specification, section 3.10 ("basic.lval"), paragraph 15.
	// (This did not substantially change in C++11.)  Roughly, this section says: if
	// an object in memory has one type, and a program accesses it with a different
	// type, then the result is undefined behavior for most values of "different
	// type".
	//
	// This is true for any cast syntax, either *(int*)&f or
	// *reinterpret_cast<int*>(&f).  And it is particularly true for conversions
	// between integral lvalues and floating-point lvalues.
	//
	// The purpose of this paragraph is to allow optimizing compilers to assume that
	// expressions with different types refer to different memory.  Compilers are
	// known to take advantage of this.  So a non-conforming program quietly
	// produces wildly incorrect output.
	//
	// The problem is not the use of reinterpret_cast.  The problem is type punning:
	// holding an object in memory of one type and reading its bits back using a
	// different type.
	//
	// The C++ standard is more subtle and complex than this, but that is the basic
	// idea.
	//
	// Anyways ...
	//
	// bit_cast<> calls memcpy() which is blessed by the standard, especially by the
	// example in section 3.9 .  Also, of course, bit_cast<> wraps up the nasty
	// logic in one place.
	//
	// Fortunately memcpy() is very fast.  In optimized mode, compilers replace
	// calls to memcpy() with inline object code when the size argument is a
	// compile-time constant.  On a 32-bit system, memcpy(d,s,4) compiles to one
	// load and one store, and memcpy(d,s,8) compiles to two loads and two stores.

	template <class Dest, class Source>
	inline Dest bit_cast(const Source& source) {
		static_assert(sizeof(Dest) == sizeof(Source),
			"bit_cast requires source and destination to be the same size");
		static_assert(std::is_trivially_copyable<Dest>::value,
			"bit_cast requires the destination type to be copyable");
		static_assert(std::is_trivially_copyable<Source>::value,
			"bit_cast requires the source type to be copyable");

		Dest dest;
		memcpy(&dest, &source, sizeof(dest));
		return dest;
	}

}

#endif  // BASE_MACROS_H_
