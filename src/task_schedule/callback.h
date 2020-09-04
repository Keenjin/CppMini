#pragma once
#include "callback_forward.h"

namespace task_schedule {
	template <typename R, typename... Args>
	class OnceCallback<R(Args...)> : public internal::CallbackBase {
	public:
		using RunType = R(Args...);
		using PolymorphicInvoke = R(*)(internal::BindStateBase*,
			internal::PassingType<Args>...);

		constexpr OnceCallback() = default;
		OnceCallback(std::nullptr_t) = delete;

		explicit OnceCallback(internal::BindStateBase* bind_state)
			: internal::CallbackBase(bind_state) {}

		OnceCallback(const OnceCallback&) = delete;
		OnceCallback& operator=(const OnceCallback&) = delete;

		OnceCallback(OnceCallback&&) noexcept = default;
		OnceCallback& operator=(OnceCallback&&) noexcept = default;

		OnceCallback(RepeatingCallback<RunType> other)
			: internal::CallbackBase(std::move(other)) {}

		OnceCallback& operator=(RepeatingCallback<RunType> other) {
			static_cast<internal::CallbackBase&>(*this) = std::move(other);
			return *this;
		}

		R Run(Args... args) const & {
			static_assert(!sizeof(*this),
				"OnceCallback::Run() may only be invoked on a non-const "
				"rvalue, i.e. std::move(callback).Run().");
			assert(false);
		}

		R Run(Args... args) && {
			// Move the callback instance into a local variable before the invocation,
			// that ensures the internal state is cleared after the invocation.
			// It's not safe to touch |this| after the invocation, since running the
			// bound function may destroy |this|.
			OnceCallback cb = std::move(*this);
			PolymorphicInvoke f =
				reinterpret_cast<PolymorphicInvoke>(cb.polymorphic_invoke());
			return f(cb.bind_state_.get(), std::forward<Args>(args)...);
		}
	};

	template <typename R, typename... Args>
	class RepeatingCallback<R(Args...)> : public internal::CallbackBaseCopyable {
	public:
		using RunType = R(Args...);
		using PolymorphicInvoke = R(*)(internal::BindStateBase*,
			internal::PassingType<Args>...);

		constexpr RepeatingCallback() = default;
		RepeatingCallback(std::nullptr_t) = delete;

		explicit RepeatingCallback(internal::BindStateBase* bind_state)
			: internal::CallbackBaseCopyable(bind_state) {}

		// Copyable and movable.
		RepeatingCallback(const RepeatingCallback&) = default;
		RepeatingCallback& operator=(const RepeatingCallback&) = default;
		RepeatingCallback(RepeatingCallback&&) noexcept = default;
		RepeatingCallback& operator=(RepeatingCallback&&) noexcept = default;

		bool operator==(const RepeatingCallback& other) const {
			return EqualsInternal(other);
		}

		bool operator!=(const RepeatingCallback& other) const {
			return !operator==(other);
		}

		R Run(Args... args) const & {
			PolymorphicInvoke f =
				reinterpret_cast<PolymorphicInvoke>(this->polymorphic_invoke());
			return f(this->bind_state_.get(), std::forward<Args>(args)...);
		}

		R Run(Args... args) && {
			// Move the callback instance into a local variable before the invocation,
			// that ensures the internal state is cleared after the invocation.
			// It's not safe to touch |this| after the invocation, since running the
			// bound function may destroy |this|.
			RepeatingCallback cb = std::move(*this);
			PolymorphicInvoke f =
				reinterpret_cast<PolymorphicInvoke>(cb.polymorphic_invoke());
			return f(std::move(cb).bind_state_.get(), std::forward<Args>(args)...);
		}
	};
}