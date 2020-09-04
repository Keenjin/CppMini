#include "task_executor.h"
#include "task_runner_ui.h"
#include "task_runner_worker.h"
#include "utils/scoped_refptr.h"
#include "task_runner_pool.h"

namespace task_schedule {

	utils::scoped_refptr<TaskRunner> CreateTaskRunnerUI() {
		return utils::MakeRefCounted<TaskRunnerUI>();
	}

	utils::scoped_refptr<TaskRunner> CreateTaskRunnerBackground() {
		return utils::MakeRefCounted<TaskRunnerWorker>();
	}

	utils::scoped_refptr<TaskRunner> CreateTaskRunnerPool() {
		return utils::MakeRefCounted<TaskRunnerPool>();
	}

	bool TaskExecutor::CreateTaskRunner(TaskThreadType type) {
		switch (type)
		{
		case TaskThreadType::UI:
		{
			if (!GetTaskRunner(type)) {
				auto task_runner = CreateTaskRunnerUI();
				AddNewTaskRunner(type, task_runner);
			}
			return true;
		}
		case TaskThreadType::Background:
		{
			if (!GetTaskRunner(type)) {
				auto task_runner = CreateTaskRunnerBackground();
				AddNewTaskRunner(type, task_runner);
			}
			return true;
		}
		case TaskThreadType::Pool:
			if (!GetTaskRunner(type)) {
				auto task_runner = CreateTaskRunnerPool();
				AddNewTaskRunner(type, task_runner);
			}
			return true;
		}

		return false;
	}

	void TaskExecutor::AddNewTaskRunner(TaskThreadType type, utils::scoped_refptr<TaskRunner> runner) {
		if (runner) {
			utils::AutoLock lock(runners_factory_lock);
			runners_factory[type] = runner;
		}
	}

	void TaskExecutor::DestroyTaskRunner(TaskThreadType type) {
		utils::AutoLock lock(runners_factory_lock);
		if (runners_factory.find(type) != runners_factory.end())
			runners_factory.erase(type);
	}

	utils::scoped_refptr<TaskRunner> TaskExecutor::GetTaskRunner(TaskThreadType type) {
		utils::AutoLock lock(runners_factory_lock);
		if (runners_factory.find(type) != runners_factory.end())
			return runners_factory[type];
		return nullptr;
	}

	TaskExecutor* GetTaskExecutor() {
		static utils::NoDestructor<TaskExecutor> task_factory;
		return task_factory.get();
	}
}
