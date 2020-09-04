#pragma once
#include "utils/macros.h"
#include "task_def.h"
#include "utils/scoped_refptr.h"
#include "task_runner.h"
#include <map>
#include "utils/lock.h"

namespace task_schedule {

	class TaskExecutor {
	public:
		bool CreateTaskRunner(TaskThreadType type);
		void DestroyTaskRunner(TaskThreadType type);
		utils::scoped_refptr<TaskRunner> GetTaskRunner(TaskThreadType type);

	private:
		void AddNewTaskRunner(TaskThreadType type, utils::scoped_refptr<TaskRunner> runner);

		utils::Lock runners_factory_lock;
		std::map<TaskThreadType, utils::scoped_refptr<TaskRunner>> runners_factory;
	};

	TaskExecutor* GetTaskExecutor();
}
