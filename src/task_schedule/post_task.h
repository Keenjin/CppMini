#pragma once
#include "task_def.h"
#include "bind.h"
#include "task.h"

namespace task_schedule {
	bool Register(TaskThreadType type);
	void UnRegister(TaskThreadType type);
	void UnRegisterAll();

	bool PostTask(TaskThreadType type, OnceClosure task, TaskPriority priority = TaskPriority::NORMAL);
	bool PostTaskAndReply(TaskThreadType type, OnceClosure task, OnceClosure reply, TaskPriority priority = TaskPriority::NORMAL);

	// ��û����Ϣѭ��ʱ����Ҫ�������
	void RunLoop();
}
