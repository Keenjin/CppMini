#pragma once
#include <forward_list>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace utils {

	namespace internal {

		// Calls erase on iterators of matching elements.
		template <typename Container, typename Predicate>
		void IterateAndEraseIf(Container& container, Predicate pred) {
			for (auto it = container.begin(); it != container.end();) {
				if (pred(*it))
					it = container.erase(it);
				else
					++it;
			}
		}

		// Utility type traits used for specializing base::Contains() below.
		template <typename Container, typename Element, typename = void>
		struct HasFindWithNpos : std::false_type {};

		template <typename Container, typename Element>
		struct HasFindWithNpos<
			Container,
			Element,
			std::void_t<decltype(std::declval<const Container&>().find(
				std::declval<const Element&>()) != Container::npos)>>
			: std::true_type {};

		template <typename Container, typename Element, typename = void>
		struct HasFindWithEnd : std::false_type {};

		template <typename Container, typename Element>
		struct HasFindWithEnd<Container,
			Element,
			std::void_t<decltype(std::declval<const Container&>().find(
				std::declval<const Element&>()) !=
				std::declval<const Container&>().end())>>
			: std::true_type {};

		template <typename Container, typename Element, typename = void>
		struct HasContains : std::false_type {};

		template <typename Container, typename Element>
		struct HasContains<Container,
			Element,
			std::void_t<decltype(std::declval<const Container&>().contains(
				std::declval<const Element&>()))>> : std::true_type {};
	} // internal

	// General purpose implementation to check if |container| contains |value|.
	template <typename Container,
		typename Value,
		std::enable_if_t<
		!internal::HasFindWithNpos<Container, Value>::value &&
		!internal::HasFindWithEnd<Container, Value>::value &&
		!internal::HasContains<Container, Value>::value>* = nullptr>
		bool Contains(const Container& container, const Value& value) {
		using std::begin;
		using std::end;
		return std::find(begin(container), end(container), value) != end(container);
	}

	// Specialized Contains() implementation for when |container| has a find()
	// member function and a static npos member, but no contains() member function.
	template <typename Container,
		typename Value,
		std::enable_if_t<internal::HasFindWithNpos<Container, Value>::value &&
		!internal::HasContains<Container, Value>::value>* =
		nullptr>
		bool Contains(const Container& container, const Value& value) {
		return container.find(value) != Container::npos;
	}

	// Specialized Contains() implementation for when |container| has a find()
	// and end() member function, but no contains() member function.
	template <typename Container,
		typename Value,
		std::enable_if_t<internal::HasFindWithEnd<Container, Value>::value &&
		!internal::HasContains<Container, Value>::value>* =
		nullptr>
		bool Contains(const Container& container, const Value& value) {
		return container.find(value) != container.end();
	}

	// Specialized Contains() implementation for when |container| has a contains()
	// member function.
	template <
		typename Container,
		typename Value,
		std::enable_if_t<internal::HasContains<Container, Value>::value>* = nullptr>
		bool Contains(const Container& container, const Value& value) {
		return container.contains(value);
	}

	// Erase/EraseIf are based on library fundamentals ts v2 erase/erase_if
	// http://en.cppreference.com/w/cpp/experimental/lib_extensions_2
	// They provide a generic way to erase elements from a container.
	// The functions here implement these for the standard containers until those
	// functions are available in the C++ standard.
	// For Chromium containers overloads should be defined in their own headers
	// (like standard containers).
	// Note: there is no std::erase for standard associative containers so we don't
	// have it either.

	template <typename CharT, typename Traits, typename Allocator, typename Value>
	void Erase(std::basic_string<CharT, Traits, Allocator>& container,
		const Value& value) {
		container.erase(std::remove(container.begin(), container.end(), value),
			container.end());
	}

	template <typename CharT, typename Traits, typename Allocator, class Predicate>
	void EraseIf(std::basic_string<CharT, Traits, Allocator>& container,
		Predicate pred) {
		container.erase(std::remove_if(container.begin(), container.end(), pred),
			container.end());
	}

	template <class T, class Allocator, class Value>
	void Erase(std::deque<T, Allocator>& container, const Value& value) {
		container.erase(std::remove(container.begin(), container.end(), value),
			container.end());
	}

	template <class T, class Allocator, class Predicate>
	void EraseIf(std::deque<T, Allocator>& container, Predicate pred) {
		container.erase(std::remove_if(container.begin(), container.end(), pred),
			container.end());
	}

	template <class T, class Allocator, class Value>
	void Erase(std::vector<T, Allocator>& container, const Value& value) {
		container.erase(std::remove(container.begin(), container.end(), value),
			container.end());
	}

	template <class T, class Allocator, class Predicate>
	void EraseIf(std::vector<T, Allocator>& container, Predicate pred) {
		container.erase(std::remove_if(container.begin(), container.end(), pred),
			container.end());
	}

	template <class T, class Allocator, class Value>
	void Erase(std::forward_list<T, Allocator>& container, const Value& value) {
		// Unlike std::forward_list::remove, this function template accepts
		// heterogeneous types and does not force a conversion to the container's
		// value type before invoking the == operator.
		container.remove_if([&](const T& cur) { return cur == value; });
	}

	template <class T, class Allocator, class Predicate>
	void EraseIf(std::forward_list<T, Allocator>& container, Predicate pred) {
		container.remove_if(pred);
	}

	template <class T, class Allocator, class Value>
	void Erase(std::list<T, Allocator>& container, const Value& value) {
		// Unlike std::list::remove, this function template accepts heterogeneous
		// types and does not force a conversion to the container's value type before
		// invoking the == operator.
		container.remove_if([&](const T& cur) { return cur == value; });
	}

	template <class T, class Allocator, class Predicate>
	void EraseIf(std::list<T, Allocator>& container, Predicate pred) {
		container.remove_if(pred);
	}

	template <class Key, class T, class Compare, class Allocator, class Predicate>
	void EraseIf(std::map<Key, T, Compare, Allocator>& container, Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key, class T, class Compare, class Allocator, class Predicate>
	void EraseIf(std::multimap<Key, T, Compare, Allocator>& container,
		Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key, class Compare, class Allocator, class Predicate>
	void EraseIf(std::set<Key, Compare, Allocator>& container, Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key, class Compare, class Allocator, class Predicate>
	void EraseIf(std::multiset<Key, Compare, Allocator>& container,
		Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key,
		class T,
		class Hash,
		class KeyEqual,
		class Allocator,
		class Predicate>
		void EraseIf(std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& container,
			Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key,
		class T,
		class Hash,
		class KeyEqual,
		class Allocator,
		class Predicate>
		void EraseIf(
			std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& container,
			Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key,
		class Hash,
		class KeyEqual,
		class Allocator,
		class Predicate>
		void EraseIf(std::unordered_set<Key, Hash, KeyEqual, Allocator>& container,
			Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}

	template <class Key,
		class Hash,
		class KeyEqual,
		class Allocator,
		class Predicate>
		void EraseIf(std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& container,
			Predicate pred) {
		internal::IterateAndEraseIf(container, pred);
	}
}
