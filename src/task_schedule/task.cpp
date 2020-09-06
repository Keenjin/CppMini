#include "include/task.h"
#include "utils/atomic_sequence_num.h"

namespace task_schedule {

	namespace {

		utils::AtomicSequenceNumber g_sequence_nums_for_tracing;

	}  // namespace

	Task::Task(OnceClosure task, TaskPriority priority)
		: task_(std::move(task)), priority_(priority) {
		this->sequence_num = g_sequence_nums_for_tracing.GetNext();
	}

	Task::Task(const Task& other)
		: task_(std::move(other.task_)), priority_(other.priority_), sequence_num(other.sequence_num) {

	}

	bool Task::operator<(const Task& other) const {
		if (priority_ < other.priority_)
			return true;

		if (priority_ > other.priority_)
			return false;

		return (sequence_num - other.sequence_num) > 0;
	}

	Task& Task::operator=(Task&& other) = default;
}
