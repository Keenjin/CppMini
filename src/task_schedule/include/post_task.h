#pragma once
#include "task_def.h"
#include "bind.h"

namespace task_schedule {
	namespace internal {
		template <typename ReturnType>
		void ReturnAsParamAdapter(OnceCallback<ReturnType()> func,
			std::unique_ptr<ReturnType>* result) {
			result->reset(new ReturnType(std::move(func).Run()));
		}

		template <typename TaskReturnType>
		void ReplyAdapter(OnceCallback<void(TaskReturnType)> callback,
			std::unique_ptr<TaskReturnType>* result) {
			assert(result->get());
			std::move(callback).Run(std::move(**result));
		}
	}

	// 相关任务类型必须经过注册，才能使用。默认不注册，减少不必要的资源消耗
	bool Register(TaskThreadType type);
	void UnRegister(TaskThreadType type);
	void UnRegisterAll();

	// 结束任务，尽可能快
	void CleanupTasksImmediately(TaskThreadType type, bool disableForever = true);

	// 结束任务，并等待所有任务结束
	void StopAndWaitTasksFinish(TaskThreadType type);

	// 判断当前线程是哪个线程
	TaskThreadType GetCurrentThreadType();

	// 适合一些异步任务，不需要回复
	bool PostTask(
		TaskThreadType type,
		OnceClosure task, 
		TaskPriority priority = TaskPriority::NORMAL);

	// 同步任务，仅适合调用者和任务线程不在一个线程，才能同步执行，同一个线程不会同步。此接口慎用，目前仅仅简单实现解决同步调用问题
	// 暂时不支持线程池任务 -- todo
	bool PostTaskAndWaitFinish(
		TaskThreadType type,
		OnceClosure task,
		uint32_t timeout = 3000,		// 默认等3s
		TaskPriority priority = TaskPriority::NORMAL);

	// 适合一些异步任务，回复任务执行完毕
	bool PostTaskAndReply(
		TaskThreadType type, 
		OnceClosure task, 
		OnceClosure reply, 
		TaskPriority priority = TaskPriority::NORMAL);
	bool PostTaskAndReply(
		TaskThreadType task_thread_type, OnceClosure task, 
		TaskThreadType reply_thread_type, OnceClosure reply, 
		TaskPriority task_priority = TaskPriority::NORMAL, 
		TaskPriority reply_priority = TaskPriority::NORMAL);

	// 发一个任务，执行结果通过返回值回传给reply，即：reply的输入参数，就是前面执行任务的返回结果
	template<typename TaskReturnType>
	bool PostTaskAndReplyWithResult(
		TaskThreadType type,
		OnceCallback<TaskReturnType()> task,
		OnceCallback<void(TaskReturnType)> reply,
		TaskPriority priority = TaskPriority::NORMAL) {
		auto* result = new std::unique_ptr<TaskReturnType>();
		return PostTaskAndReply(type,
			BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>, std::move(task), result),
			BindOnce(&internal::ReplyAdapter<TaskReturnType>, std::move(reply), Owned(result)), 
			priority);
	}

	template<typename TaskReturnType>
	bool PostTaskAndReplyWithResult(
		TaskThreadType task_thread_type,
		OnceCallback<TaskReturnType()> task,
		TaskThreadType reply_thread_type,
		OnceCallback<void(TaskReturnType)> reply,
		TaskPriority task_priority = TaskPriority::NORMAL,
		TaskPriority reply_priority = TaskPriority::NORMAL) {
		auto* result = new std::unique_ptr<TaskReturnType>();
		return PostTaskAndReply(task_thread_type,
			BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>, std::move(task), result),
			reply_thread_type,
			BindOnce(&internal::ReplyAdapter<TaskReturnType>, std::move(reply), Owned(result)),
			task_priority, reply_priority);
	}

	// 带上下文的任务


	// 当没有消息循环时，需要加上这个，才能正常使用UI的线程模型
	void RunUILoopForTest();
}
