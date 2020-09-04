#pragma once

namespace task_schedule {

	template <typename Signature>
	class OnceCallback;

	template <typename Signature>
	class RepeatingCallback;

	template <typename Signature>
	using Callback = RepeatingCallback<Signature>;

	// Syntactic sugar to make OnceClosure<void()> and RepeatingClosure<void()>
	// easier to declare since they will be used in a lot of APIs with delayed
	// execution.
	using OnceClosure = OnceCallback<void()>;
	using RepeatingClosure = RepeatingCallback<void()>;
	using Closure = Callback<void()>;
}