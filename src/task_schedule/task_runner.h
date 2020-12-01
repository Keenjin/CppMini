#pragma once
#include "include/task_def.h"
#include "utils/ref_counted.h"
#include "include/task.h"
#include <stdint.h>

namespace task_schedule {

	class TaskRunner : public utils::RefCountedThreadSafe<TaskRunner> {
	public:
		class Delegate {
		public:
			virtual ~Delegate() = default;
			virtual void AddTask(Task task) = 0;
			virtual Task GetNextTask() = 0;
			virtual bool ScheduleTask(Task task) = 0;
			virtual void TaskDone(Task task, bool result) = 0;
		};
		virtual bool PostTask(OnceClosure task, TaskPriority priority = TaskPriority::NORMAL) = 0;
		virtual void CleanupTasksImmediately(bool disableForever = true) = 0;
		virtual void StopAndWaitTasksFinish() = 0;
		virtual uint32_t ThreadId() { return kThreadInvalidId; }

	private:
		friend class utils::RefCountedThreadSafe<TaskRunner>;
	};
}
