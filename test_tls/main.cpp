#include <thread>
#include <iostream>
#include <Windows.h>

void allocTlsInex() { 
	for (auto i = 0; i < 100; i++) {
		std::cout << "tid: " << std::this_thread::get_id() << ", index: " << ::TlsAlloc() << std::endl;
	}
}

int main() {
	allocTlsInex();

	std::cout << std::endl;

	std::thread([]() {
		allocTlsInex();
		}).join();
}