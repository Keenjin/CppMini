#pragma once

#include "base\task\post_task.h"
#include "base\callback_helpers.h"

#include <iostream>

void Task1() {
	std::cout << "run task1" << std::endl;
}

void test_post_task() {
	base::ThreadPool::PostTask(FROM_HERE, base::BindOnce(&Task1));
}