#pragma once

#include <stddef.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include "macros.h"

namespace utils {
	template <typename T>
	class CheckedContiguousIterator {
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = std::remove_cv_t<T>;
		using pointer = T * ;
		using reference = T & ;
		using iterator_category = std::random_access_iterator_tag;

		// Required for converting constructor below.
		template <typename U>
		friend class CheckedContiguousIterator;

		constexpr CheckedContiguousIterator() = default;
		constexpr CheckedContiguousIterator(T* start, const T* end)
			: CheckedContiguousIterator(start, start, end) {}
		constexpr CheckedContiguousIterator(const T* start, T* current, const T* end)
			: start_(start), current_(current), end_(end) {
			assert(start <= current);
			assert(current <= end);
		}
		constexpr CheckedContiguousIterator(const CheckedContiguousIterator& other) =
			default;

		// Converting constructor allowing conversions like CRAI<T> to CRAI<const T>,
		// but disallowing CRAI<const T> to CRAI<T> or CRAI<Derived> to CRAI<Base>,
		// which are unsafe. Furthermore, this is the same condition as used by the
		// converting constructors of std::span<T> and std::unique_ptr<T[]>.
		// See https://wg21.link/n4042 for details.
		template <
			typename U,
			std::enable_if_t<std::is_convertible<U(*)[], T(*)[]>::value>* = nullptr>
			constexpr CheckedContiguousIterator(const CheckedContiguousIterator<U>& other)
			: start_(other.start_), current_(other.current_), end_(other.end_) {
			// We explicitly don't delegate to the 3-argument constructor here. Its
			// CHECKs would be redundant, since we expect |other| to maintain its own
			// invariant. However, DCHECKs never hurt anybody. Presumably.
			assert(other.start_ <= other.current_);
			assert(other.current_ <= other.end_);
		}

		~CheckedContiguousIterator() = default;

		constexpr CheckedContiguousIterator& operator=(
			const CheckedContiguousIterator& other) = default;

		constexpr bool operator==(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ == other.current_;
		}

		constexpr bool operator!=(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ != other.current_;
		}

		constexpr bool operator<(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ < other.current_;
		}

		constexpr bool operator<=(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ <= other.current_;
		}

		constexpr bool operator>(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ > other.current_;
		}

		constexpr bool operator>=(const CheckedContiguousIterator& other) const {
			CheckComparable(other);
			return current_ >= other.current_;
		}

		constexpr CheckedContiguousIterator& operator++() {
			assert(current_ != end_);
			++current_;
			return *this;
		}

		constexpr CheckedContiguousIterator operator++(int) {
			CheckedContiguousIterator old = *this;
			++*this;
			return old;
		}

		constexpr CheckedContiguousIterator& operator--() {
			assert(current_ != start_);
			--current_;
			return *this;
		}

		constexpr CheckedContiguousIterator& operator--(int) {
			CheckedContiguousIterator old = *this;
			--*this;
			return old;
		}

		constexpr CheckedContiguousIterator& operator+=(difference_type rhs) {
			if (rhs > 0) {
				assert(rhs <= end_ - current_);
			}
			else {
				assert(-rhs <= current_ - start_);
			}
			current_ += rhs;
			return *this;
		}

		constexpr CheckedContiguousIterator operator+(difference_type rhs) const {
			CheckedContiguousIterator it = *this;
			it += rhs;
			return it;
		}

		constexpr CheckedContiguousIterator& operator-=(difference_type rhs) {
			if (rhs < 0) {
				assert(rhs <= end_ - current_);
			}
			else {
				assert(-rhs <= current_ - start_);
			}
			current_ -= rhs;
			return *this;
		}

		constexpr CheckedContiguousIterator operator-(difference_type rhs) const {
			CheckedContiguousIterator it = *this;
			it -= rhs;
			return it;
		}

		constexpr friend difference_type operator-(
			const CheckedContiguousIterator& lhs,
			const CheckedContiguousIterator& rhs) {
			assert(lhs.start_ == rhs.start_);
			assert(lhs.end_ == rhs.end_);
			return lhs.current_ - rhs.current_;
		}

		constexpr reference operator*() const {
			assert(current_ != end_);
			return *current_;
		}

		constexpr pointer operator->() const {
			assert(current_ != end_);
			return current_;
		}

		constexpr reference operator[](difference_type rhs) const {
			assert(rhs >= 0);
			assert(rhs < end_ - current_);
			return current_[rhs];
		}

		static bool IsRangeMoveSafe(const CheckedContiguousIterator& from_begin,
			const CheckedContiguousIterator& from_end,
			const CheckedContiguousIterator& to)
		{
			if (from_end < from_begin)
				return false;
			const auto from_begin_uintptr = reinterpret_cast<uintptr_t>(from_begin.current_);
			const auto from_end_uintptr = reinterpret_cast<uintptr_t>(from_end.current_);
			const auto to_begin_uintptr = reinterpret_cast<uintptr_t>(to.current_);
			const auto to_end_uintptr =
				reinterpret_cast<uintptr_t>((to + std::distance(from_begin, from_end)).current_);

			return to_begin_uintptr >= from_end_uintptr ||
				to_end_uintptr <= from_begin_uintptr;
		}

	private:
		constexpr void CheckComparable(const CheckedContiguousIterator& other) const {
			assert(start_ == other.start_);
			assert(end_ == other.end_);
		}

		const T* start_ = nullptr;
		T* current_ = nullptr;
		const T* end_ = nullptr;
	};

	template <typename T>
	using CheckedContiguousConstIterator = CheckedContiguousIterator<const T>;

	// [views.constants]
	constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

	template <typename T, size_t Extent = dynamic_extent>
	class span;

	namespace internal {

		template <typename T>
		struct IsSpanImpl : std::false_type {};

		template <typename T, size_t Extent>
		struct IsSpanImpl<span<T, Extent>> : std::true_type {};

		template <typename T>
		using IsSpan = IsSpanImpl<std::decay_t<T>>;

		template <typename T>
		struct IsStdArrayImpl : std::false_type {};

		template <typename T, size_t N>
		struct IsStdArrayImpl<std::array<T, N>> : std::true_type {};

		template <typename T>
		using IsStdArray = IsStdArrayImpl<std::decay_t<T>>;

		template <typename T>
		using IsCArray = std::is_array<std::remove_reference_t<T>>;

		template <typename From, typename To>
		using IsLegalDataConversion = std::is_convertible<From(*)[], To(*)[]>;

		template <typename Container, typename T>
		using ContainerHasConvertibleData = IsLegalDataConversion<
			std::remove_pointer_t<decltype(std::data(std::declval<Container>()))>,
			T>;

		template <typename Container>
		using ContainerHasIntegralSize =
			std::is_integral<decltype(std::size(std::declval<Container>()))>;

		template <typename From, size_t FromExtent, typename To, size_t ToExtent>
		using EnableIfLegalSpanConversion =
			std::enable_if_t<(ToExtent == dynamic_extent || ToExtent == FromExtent) &&
			IsLegalDataConversion<From, To>::value>;

		// SFINAE check if Array can be converted to a span<T>.
		template <typename Array, size_t N, typename T, size_t Extent>
		using EnableIfSpanCompatibleArray =
			std::enable_if_t<(Extent == dynamic_extent || Extent == N) &&
			ContainerHasConvertibleData<Array, T>::value>;

		// SFINAE check if Container can be converted to a span<T>.
		template <typename Container, typename T>
		using IsSpanCompatibleContainer =
			std::conditional_t<!IsSpan<Container>::value &&
			!IsStdArray<Container>::value &&
			!IsCArray<Container>::value &&
			ContainerHasConvertibleData<Container, T>::value &&
			ContainerHasIntegralSize<Container>::value,
			std::true_type,
			std::false_type>;

		template <typename Container, typename T>
		using EnableIfSpanCompatibleContainer =
			std::enable_if_t<IsSpanCompatibleContainer<Container, T>::value>;

		template <typename Container, typename T, size_t Extent>
		using EnableIfSpanCompatibleContainerAndSpanIsDynamic =
			std::enable_if_t<IsSpanCompatibleContainer<Container, T>::value &&
			Extent == dynamic_extent,
			bool>;

		template <typename Container, typename T, size_t Extent>
		using EnableIfSpanCompatibleContainerAndSpanIsStatic =
			std::enable_if_t<IsSpanCompatibleContainer<Container, T>::value &&
			Extent != dynamic_extent,
			bool>;

		// A helper template for storing the size of a span. Spans with static extents
		// don't require additional storage, since the extent itself is specified in the
		// template parameter.
		template <size_t Extent>
		class ExtentStorage {
		public:
			constexpr explicit ExtentStorage(size_t size) noexcept {}
			constexpr size_t size() const noexcept { return Extent; }
			constexpr void swap(ExtentStorage& other) noexcept {}
		};

		// Specialization of ExtentStorage for dynamic extents, which do require
		// explicit storage for the size.
		template <>
		struct ExtentStorage<dynamic_extent> {
			constexpr explicit ExtentStorage(size_t size) noexcept : size_(size) {}
			constexpr size_t size() const noexcept { return size_; }
			constexpr void swap(ExtentStorage& other) noexcept {
				// Note: Can't use std::swap here, as it's not constexpr prior to C++20.
				size_t size = size_;
				size_ = other.size_;
				other.size_ = size;
			}

		private:
			size_t size_;
		};

	}  // namespace internal

	// A span is a value type that represents an array of elements of type T. Since
	// it only consists of a pointer to memory with an associated size, it is very
	// light-weight. It is cheap to construct, copy, move and use spans, so that
	// users are encouraged to use it as a pass-by-value parameter. A span does not
	// own the underlying memory, so care must be taken to ensure that a span does
	// not outlive the backing store.
	//
	// span is somewhat analogous to StringPiece, but with arbitrary element types,
	// allowing mutation if T is non-const.
	//
	// span is implicitly convertible from C++ arrays, as well as most [1]
	// container-like types that provide a data() and size() method (such as
	// std::vector<T>). A mutable span<T> can also be implicitly converted to an
	// immutable span<const T>.
	//
	// Consider using a span for functions that take a data pointer and size
	// parameter: it allows the function to still act on an array-like type, while
	// allowing the caller code to be a bit more concise.
	//
	// For read-only data access pass a span<const T>: the caller can supply either
	// a span<const T> or a span<T>, while the callee will have a read-only view.
	// For read-write access a mutable span<T> is required.
	//
	// Without span:
	//   Read-Only:
	//     // std::string HexEncode(const uint8_t* data, size_t size);
	//     std::vector<uint8_t> data_buffer = GenerateData();
	//     std::string r = HexEncode(data_buffer.data(), data_buffer.size());
	//
	//  Mutable:
	//     // ssize_t SafeSNPrintf(char* buf, size_t N, const char* fmt, Args...);
	//     char str_buffer[100];
	//     SafeSNPrintf(str_buffer, sizeof(str_buffer), "Pi ~= %lf", 3.14);
	//
	// With span:
	//   Read-Only:
	//     // std::string HexEncode(base::span<const uint8_t> data);
	//     std::vector<uint8_t> data_buffer = GenerateData();
	//     std::string r = HexEncode(data_buffer);
	//
	//  Mutable:
	//     // ssize_t SafeSNPrintf(base::span<char>, const char* fmt, Args...);
	//     char str_buffer[100];
	//     SafeSNPrintf(str_buffer, "Pi ~= %lf", 3.14);
	//
	// Spans with "const" and pointers
	// -------------------------------
	//
	// Const and pointers can get confusing. Here are vectors of pointers and their
	// corresponding spans:
	//
	//   const std::vector<int*>        =>  base::span<int* const>
	//   std::vector<const int*>        =>  base::span<const int*>
	//   const std::vector<const int*>  =>  base::span<const int* const>
	//
	// Differences from the C++20 draft
	// --------------------------------
	//
	// http://eel.is/c++draft/views contains the latest C++20 draft of std::span.
	// Chromium tries to follow the draft as close as possible. Differences between
	// the draft and the implementation are documented in subsections below.
	//
	// Differences from [span.objectrep]:
	// - as_bytes() and as_writable_bytes() return spans of uint8_t instead of
	//   std::byte (std::byte is a C++17 feature)
	//
	// Differences from [span.cons]:
	// - Constructing a static span (i.e. Extent != dynamic_extent) from a dynamic
	//   sized container (e.g. std::vector) requires an explicit conversion (in the
	//   C++20 draft this is simply UB)
	//
	// Differences from [span.obs]:
	// - empty() is marked with WARN_UNUSED_RESULT instead of [[nodiscard]]
	//   ([[nodiscard]] is a C++17 feature)
	//
	// Furthermore, all constructors and methods are marked noexcept due to the lack
	// of exceptions in Chromium.
	//
	// Due to the lack of class template argument deduction guides in C++14
	// appropriate make_span() utility functions are provided.

	// [span], class template span
	template <typename T, size_t Extent>
	class span : public internal::ExtentStorage<Extent> {
	private:
		using ExtentStorage = internal::ExtentStorage<Extent>;

	public:
		using element_type = T;
		using value_type = std::remove_cv_t<T>;
		using index_type = size_t;
		using difference_type = ptrdiff_t;
		using pointer = T * ;
		using reference = T & ;
		using iterator = CheckedContiguousIterator<T>;
		using const_iterator = CheckedContiguousConstIterator<T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		static constexpr index_type extent = Extent;

		// [span.cons], span constructors, copy, assignment, and destructor
		constexpr span() noexcept : ExtentStorage(0), data_(nullptr) {
			static_assert(Extent == dynamic_extent || Extent == 0, "Invalid Extent");
		}

		constexpr span(T* data, size_t size) noexcept
			: ExtentStorage(size), data_(data) {
			assert(Extent == dynamic_extent || Extent == size);
		}

		// Artificially templatized to break ambiguity for span(ptr, 0).
		template <typename = void>
		constexpr span(T* begin, T* end) noexcept : span(begin, end - begin) {
			// Note: CHECK_LE is not constexpr, hence regular CHECK must be used.
			assert(begin <= end);
		}

		template <
			size_t N,
			typename = internal::EnableIfSpanCompatibleArray<T(&)[N], N, T, Extent>>
			constexpr span(T(&array)[N]) noexcept : span(base::data(array), N) {}

		template <
			size_t N,
			typename = internal::
			EnableIfSpanCompatibleArray<std::array<value_type, N>&, N, T, Extent>>
			constexpr span(std::array<value_type, N>& array) noexcept
			: span(base::data(array), N) {}

		template <size_t N,
			typename = internal::EnableIfSpanCompatibleArray<
			const std::array<value_type, N>&,
			N,
			T,
			Extent>>
			constexpr span(const std::array<value_type, N>& array) noexcept
			: span(base::data(array), N) {}

		// Conversion from a container that has compatible base::data() and integral
		// base::size().
		template <
			typename Container,
			internal::EnableIfSpanCompatibleContainerAndSpanIsDynamic<Container&,
			T,
			Extent> = false>
			constexpr span(Container& container) noexcept
			: span(base::data(container), base::size(container)) {}

		template <
			typename Container,
			internal::EnableIfSpanCompatibleContainerAndSpanIsStatic<Container&,
			T,
			Extent> = false>
			constexpr explicit span(Container& container) noexcept
			: span(base::data(container), base::size(container)) {}

		template <typename Container,
			internal::EnableIfSpanCompatibleContainerAndSpanIsDynamic<
			const Container&,
			T,
			Extent> = false>
			constexpr span(const Container& container) noexcept
			: span(base::data(container), base::size(container)) {}

		template <
			typename Container,
			internal::EnableIfSpanCompatibleContainerAndSpanIsStatic<const Container&,
			T,
			Extent> = false>
			constexpr explicit span(const Container& container) noexcept
			: span(base::data(container), base::size(container)) {}

		constexpr span(const span& other) noexcept = default;

		// Conversions from spans of compatible types and extents: this allows a
		// span<T> to be seamlessly used as a span<const T>, but not the other way
		// around. If extent is not dynamic, OtherExtent has to be equal to Extent.
		template <
			typename U,
			size_t OtherExtent,
			typename =
			internal::EnableIfLegalSpanConversion<U, OtherExtent, T, Extent>>
			constexpr span(const span<U, OtherExtent>& other)
			: span(other.data(), other.size()) {}

		constexpr span& operator=(const span& other) noexcept = default;
		~span() noexcept = default;

		// [span.sub], span subviews
		template <size_t Count>
		constexpr span<T, Count> first() const noexcept {
			static_assert(Extent == dynamic_extent || Count <= Extent,
				"Count must not exceed Extent");
			assert(Extent != dynamic_extent || Count <= size());
			return { data(), Count };
		}

		template <size_t Count>
		constexpr span<T, Count> last() const noexcept {
			static_assert(Extent == dynamic_extent || Count <= Extent,
				"Count must not exceed Extent");
			assert(Extent != dynamic_extent || Count <= size());
			return { data() + (size() - Count), Count };
		}

		template <size_t Offset, size_t Count = dynamic_extent>
		constexpr span<T,
			(Count != dynamic_extent
				? Count
				: (Extent != dynamic_extent ? Extent - Offset
					: dynamic_extent))>
			subspan() const noexcept {
			static_assert(Extent == dynamic_extent || Offset <= Extent,
				"Offset must not exceed Extent");
			static_assert(Extent == dynamic_extent || Count == dynamic_extent ||
				Count <= Extent - Offset,
				"Count must not exceed Extent - Offset");
			assert(Extent != dynamic_extent || Offset <= size());
			assert(Extent != dynamic_extent || Count == dynamic_extent ||
				Count <= size() - Offset);
			return { data() + Offset, Count != dynamic_extent ? Count : size() - Offset };
		}

		constexpr span<T, dynamic_extent> first(size_t count) const noexcept {
			// Note: CHECK_LE is not constexpr, hence regular CHECK must be used.
			assert(count <= size());
			return { data(), count };
		}

		constexpr span<T, dynamic_extent> last(size_t count) const noexcept {
			// Note: CHECK_LE is not constexpr, hence regular CHECK must be used.
			assert(count <= size());
			return { data() + (size() - count), count };
		}

		constexpr span<T, dynamic_extent> subspan(size_t offset,
			size_t count = dynamic_extent) const
			noexcept {
			// Note: CHECK_LE is not constexpr, hence regular CHECK must be used.
			assert(offset <= size());
			assert(count == dynamic_extent || count <= size() - offset);
			return { data() + offset, count != dynamic_extent ? count : size() - offset };
		}

		// [span.obs], span observers
		constexpr size_t size() const noexcept { return ExtentStorage::size(); }
		constexpr size_t size_bytes() const noexcept { return size() * sizeof(T); }
		constexpr bool empty() const noexcept WARN_UNUSED_RESULT {
			return size() == 0;
		}

		// [span.elem], span element access
		constexpr T& operator[](size_t idx) const noexcept {
			// Note: CHECK_LT is not constexpr, hence regular CHECK must be used.
			assert(idx < size());
			return *(data() + idx);
		}

		constexpr T& front() const noexcept {
			static_assert(Extent == dynamic_extent || Extent > 0,
				"Extent must not be 0");
			assert(Extent != dynamic_extent || !empty());
			return *data();
		}

		constexpr T& back() const noexcept {
			static_assert(Extent == dynamic_extent || Extent > 0,
				"Extent must not be 0");
			assert(Extent != dynamic_extent || !empty());
			return *(data() + size() - 1);
		}

		constexpr T* data() const noexcept { return data_; }

		// [span.iter], span iterator support
		constexpr iterator begin() const noexcept {
			return iterator(data_, data_ + size());
		}
		constexpr iterator end() const noexcept {
			return iterator(data_, data_ + size(), data_ + size());
		}

		constexpr const_iterator cbegin() const noexcept { return begin(); }
		constexpr const_iterator cend() const noexcept { return end(); }

		constexpr reverse_iterator rbegin() const noexcept {
			return reverse_iterator(end());
		}
		constexpr reverse_iterator rend() const noexcept {
			return reverse_iterator(begin());
		}

		constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(cend());
		}
		constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(cbegin());
		}

		constexpr void swap(span& other) noexcept {
			// Note: Can't use std::swap here, as it's not constexpr prior to C++20.
			T* data = data_;
			data_ = other.data_;
			other.data_ = data;

			ExtentStorage::swap(other);
		}

	private:
		T* data_;
	};

	// span<T, Extent>::extent can not be declared inline prior to C++17, hence this
	// definition is required.
	template <class T, size_t Extent>
	constexpr size_t span<T, Extent>::extent;

	// [span.objectrep], views of object representation
	template <typename T, size_t X>
	span<const uint8_t, (X == dynamic_extent ? dynamic_extent : sizeof(T) * X)>
		as_bytes(span<T, X> s) noexcept {
		return { reinterpret_cast<const uint8_t*>(s.data()), s.size_bytes() };
	}

	template <typename T,
		size_t X,
		typename = std::enable_if_t<!std::is_const<T>::value>>
		span<uint8_t, (X == dynamic_extent ? dynamic_extent : sizeof(T) * X)>
		as_writable_bytes(span<T, X> s) noexcept {
		return { reinterpret_cast<uint8_t*>(s.data()), s.size_bytes() };
	}

	template <typename T, size_t X>
	constexpr void swap(span<T, X>& lhs, span<T, X>& rhs) noexcept {
		lhs.swap(rhs);
	}

	// Type-deducing helpers for constructing a span.
	template <typename T>
	constexpr span<T> make_span(T* data, size_t size) noexcept {
		return { data, size };
	}

	template <typename T>
	constexpr span<T> make_span(T* begin, T* end) noexcept {
		return { begin, end };
	}

	template <typename T, size_t N>
	constexpr span<T, N> make_span(T(&array)[N]) noexcept {
		return array;
	}

	template <typename T, size_t N>
	constexpr span<T, N> make_span(std::array<T, N>& array) noexcept {
		return array;
	}

	template <typename T, size_t N>
	constexpr span<const T, N> make_span(const std::array<T, N>& array) noexcept {
		return array;
	}

	template <typename Container,
		typename T = std::remove_pointer_t<
		decltype(std::data(std::declval<Container&>()))>,
		typename = internal::EnableIfSpanCompatibleContainer<Container&, T>>
		constexpr span<T> make_span(Container& container) noexcept {
		return container;
	}

	template <
		typename Container,
		typename T = std::remove_pointer_t<
		decltype(std::data(std::declval<const Container&>()))>,
		typename = internal::EnableIfSpanCompatibleContainer<const Container&, T>>
		constexpr span<T> make_span(const Container& container) noexcept {
		return container;
	}

	template <size_t N,
		typename Container,
		typename T = std::remove_pointer_t<
		decltype(std::data(std::declval<Container&>()))>,
		typename = internal::EnableIfSpanCompatibleContainer<Container&, T>>
		constexpr span<T, N> make_span(Container& container) noexcept {
		return span<T, N>(container);
	}

	template <
		size_t N,
		typename Container,
		typename T = std::remove_pointer_t<
		decltype(std::data(std::declval<const Container&>()))>,
		typename = internal::EnableIfSpanCompatibleContainer<const Container&, T>>
		constexpr span<T, N> make_span(const Container& container) noexcept {
		return span<T, N>(container);
	}

	template <typename T, size_t X>
	constexpr span<T, X> make_span(const span<T, X>& span) noexcept {
		return span;
	}

}  // namespace base

// Note: std::tuple_size, std::tuple_element and std::get are specialized for
// static spans, so that they can be used in C++17's structured bindings. While
// we don't support C++17 yet, there is no harm in providing these
// specializations already.
namespace std {

	template <typename T, size_t X>
	struct tuple_size<utils::span<T, X>> : public integral_constant<size_t, X> {};

	template <typename T>
	struct tuple_size<utils::span<T, utils::dynamic_extent>>;  // not defined

	template <size_t I, typename T, size_t X>
	struct tuple_element<I, utils::span<T, X>> {
		static_assert(
			base::dynamic_extent != X,
			"std::tuple_element<> not supported for base::span<T, dynamic_extent>");
		static_assert(I < X,
			"Index out of bounds in std::tuple_element<> (base::span)");
		using type = T;
	};

	template <size_t I, typename T, size_t X>
	constexpr T& get(utils::span<T, X> s) noexcept {
		static_assert(utils::dynamic_extent != X,
			"std::get<> not supported for base::span<T, dynamic_extent>");
		static_assert(I < X, "Index out of bounds in std::get<> (base::span)");
		return s[I];
	}

}  // namespace std
