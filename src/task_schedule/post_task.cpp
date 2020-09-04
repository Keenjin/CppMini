#include "post_task.h"
#include "task_executor.h"
#include "task.h"

namespace task_schedule {

	namespace internal {
		class PostTaskAndReply {
		public:
			PostTaskAndReply(OnceClosure task,
				TaskThreadType reply_thread_type, OnceClosure reply,
				TaskPriority reply_priority = TaskPriority::NORMAL)
				: task_(std::move(task))
				, reply_thread_type_(reply_thread_type)
				, reply_(std::move(reply))
				, reply_priority_(reply_priority) {

			}

			PostTaskAndReply(PostTaskAndReply&&) = default;
			PostTaskAndReply& operator=(PostTaskAndReply&&) = delete;

			static void RunTaskAndPostReply(PostTaskAndReply relay) {
				std::move(relay.task_).Run();
				PostTask(relay.reply_thread_type_, std::move(relay.reply_), relay.reply_priority_);
			}

		private:
			OnceClosure task_;

			TaskThreadType reply_thread_type_;
			OnceClosure reply_;
			TaskPriority reply_priority_;

			DISALLOW_COPY_AND_ASSIGN(PostTaskAndReply);
		};
		
	}

	bool Register(TaskThreadType type) {
		return GetTaskExecutor()->CreateTaskRunner(type);
	}

	void UnRegister(TaskThreadType type) {
		GetTaskExecutor()->DestroyTaskRunner(type);
	}

	void UnRegisterAll() {
		for (int8_t i = 0; i < static_cast<int8_t>(TaskThreadType::Max); i++)
		{
			UnRegister(static_cast<TaskThreadType>(i));
		}
	}

	bool PostTask(TaskThreadType type, OnceClosure task, TaskPriority priority) {
		return GetTaskExecutor()->GetTaskRunner(type)->PostTask(std::move(task), priority);
	}

	bool PostTaskAndReply(TaskThreadType type, OnceClosure task, OnceClosure reply, TaskPriority priority) {
		return PostTaskAndReply(type, std::move(task), type, std::move(reply), priority);
	}

	bool PostTaskAndReply(
		TaskThreadType task_thread_type, OnceClosure task,
		TaskThreadType reply_thread_type, OnceClosure reply,
		TaskPriority task_priority/* = TaskPriority::NORMAL*/,
		TaskPriority reply_priority/* = TaskPriority::NORMAL*/) {
		return PostTask(task_thread_type, BindOnce(
				&internal::PostTaskAndReply::RunTaskAndPostReply, 
				internal::PostTaskAndReply(
				std::move(task), 
				reply_thread_type, 
				std::move(reply), 
				reply_priority)), 
			task_priority);
	}

	void RunUILoopForTest() {
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0))
		{
			DispatchMessageW(&msg);
		}
	}
}
