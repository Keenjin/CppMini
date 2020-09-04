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

		template <typename TaskReturnType, typename ReplyArgType>
		void ReplyAdapter(OnceCallback<void(ReplyArgType)> callback,
			std::unique_ptr<TaskReturnType>* result) {
			assert(result->get());
			std::move(callback).Run(std::move(**result));
		}
	}

	bool Register(TaskThreadType type);
	void UnRegister(TaskThreadType type);
	void UnRegisterAll();

	// �ʺ�һЩ�첽���񣬲���Ҫ�ظ�
	bool PostTask(
		TaskThreadType type,
		OnceClosure task, 
		TaskPriority priority = TaskPriority::NORMAL);

	// �ʺ�һЩ�첽���񣬻ظ�����ִ�����
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

	template<typename TaskReturnType, typename ReplyArgType>
	bool PostTaskAndReplyWithResult(
		TaskThreadType type,
		OnceCallback<TaskReturnType()> task,
		OnceCallback<void(ReplyArgType)> reply,
		TaskPriority priority = TaskPriority::NORMAL) {
		auto* result = new std::unique_ptr<TaskReturnType>();
		return PostTaskAndReply(type,
			BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>, std::move(task), result),
			BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>, std::move(reply), Owned(result)), 
			priority);
	}

	// ��û����Ϣѭ��ʱ����Ҫ�������
	void RunUILoopForTest();
}
