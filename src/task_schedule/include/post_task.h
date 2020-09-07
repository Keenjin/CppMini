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

	// ����������ͱ��뾭��ע�ᣬ����ʹ�á�Ĭ�ϲ�ע�ᣬ���ٲ���Ҫ����Դ����
	bool Register(TaskThreadType type);
	void UnRegister(TaskThreadType type);
	void UnRegisterAll();

	// �������񣬾����ܿ�
	void CleanupTasksImmediately(TaskThreadType type);

	// �������񣬲��ȴ������������
	void StopAndWaitTasksFinish(TaskThreadType type);

	// �ʺ�һЩ�첽���񣬲���Ҫ�ظ�
	bool PostTask(
		TaskThreadType type,
		OnceClosure task, 
		TaskPriority priority = TaskPriority::NORMAL);

	// ͬ�����񣬽��ʺϵ����ߺ������̲߳���һ���̣߳�����ͬ��ִ�У�ͬһ���̲߳���ͬ�����˽ӿ����ã�Ŀǰ������ʵ�ֽ��ͬ����������
	// ��ʱ��֧���̳߳����� -- todo
	bool PostTaskAndWaitFinish(
		TaskThreadType type,
		OnceClosure task,
		uint32_t timeout = 3000,		// Ĭ�ϵ�3s
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

	// ��һ������ִ�н��ͨ������ֵ�ش���reply������reply���������������ǰ��ִ������ķ��ؽ��
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

	// �������ĵ�����


	// ��û����Ϣѭ��ʱ����Ҫ�����������������ʹ��UI���߳�ģ��
	void RunUILoopForTest();
}
