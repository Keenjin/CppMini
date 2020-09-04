#pragma once

#include "post_task.h"
#include "utils/lock.h"

#include <iostream>

utils::Lock g_lock;

void Test1(int a) {
	utils::AutoLock lock(g_lock);
	std::cout << "ThreadId: " << GetCurrentThreadId() << "\tTest1: " << a << std::endl;
}

class CTest : public utils::RefCountedThreadSafe<CTest> {
public:
	void Test2(int a) {
		utils::AutoLock lock(g_lock);
		std::cout << "ThreadId: " << GetCurrentThreadId() << "\tCTest::Test2: " << a << std::endl;
	}
};

void test_post_task() {
	utils::scoped_refptr<CTest> a = new CTest;
	task_schedule::Register(task_schedule::TaskThreadType::UI);
	task_schedule::Register(task_schedule::TaskThreadType::Background);
	task_schedule::PostTask(task_schedule::TaskThreadType::UI, task_schedule::BindOnce(&CTest::Test2, a, 1));
	task_schedule::PostTask(task_schedule::TaskThreadType::UI, task_schedule::BindOnce(Test1, 2));
	task_schedule::PostTask(task_schedule::TaskThreadType::UI, task_schedule::BindOnce(Test1, 3));
	task_schedule::PostTask(task_schedule::TaskThreadType::UI, task_schedule::BindOnce(Test1, 4));
	task_schedule::PostTask(task_schedule::TaskThreadType::UI, task_schedule::BindOnce(Test1, 5));
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 6), task_schedule::TaskPriority::HIGHEST);
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 7));
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 8), task_schedule::TaskPriority::LOWEST);
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 9));
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 10));
	task_schedule::PostTask(task_schedule::TaskThreadType::Background, task_schedule::BindOnce(Test1, 11));

	task_schedule::RunLoop();

	task_schedule::UnRegisterAll();
}
