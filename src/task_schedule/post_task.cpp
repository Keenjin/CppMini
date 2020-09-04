#include "post_task.h"
#include "task_executor.h"


namespace task_schedule {

	bool Register(TaskThreadType type) {
		return GetTaskExecutor()->CreateTaskRunner(type);
	}

	void UnRegister(TaskThreadType type) {
		GetTaskExecutor()->DestroyTaskRunner(type);
	}

	void UnRegisterAll() {
		UnRegister(TaskThreadType::UI);
		UnRegister(TaskThreadType::Background);
	}

	bool PostTask(TaskThreadType type, OnceClosure task, TaskPriority priority) {
		return GetTaskExecutor()->GetTaskRunner(type)->PostTask(std::move(task), priority);
	}

	bool PostTaskAndReply(TaskThreadType type, OnceClosure task, OnceClosure reply, TaskPriority priority) {
		return GetTaskExecutor()->GetTaskRunner(type)->PostTaskAndReply(std::move(task), std::move(reply), priority);
	}

	void RunLoop() {
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0))
		{
			DispatchMessageW(&msg);
		}
	}
}
