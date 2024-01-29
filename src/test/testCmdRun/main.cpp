#include <string>
#include <format>
#include <iostream>
#include <array>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>
#include <shared_mutex>
#include <filesystem>
#include <fstream>

#include "string_convert.hpp"


int main() {
	//auto fmt = "I Love{}, and PI is{}, not {}";
	//auto s1 = std::format(fmt, "math", 3.1415926, 3);
	//auto s2 = std::vformat(fmt, std::make_format_args("math", 3.1415926, 3));
	//auto s3 = std::to_string(42);
	//std::cout << s1 << std::endl;
	//std::cout << s2 << std::endl;
	//std::cout << s3 << std::endl;

	//std::array<char, 10> s4 = {0};
	//if (auto [p, err] = std::to_chars(s4.data(), s4.data() + s4.size(), 3.14); err == std::errc()) {
	//	std::cout << std::string_view(s4.data(), p - s4.data()) << std::endl;
	//}

	//std::string output;
	//std::format_to(std::back_inserter(output), "{}", "keen");
	//std::format_to(std::back_inserter(output), " {}", 32323);
	//std::format_to(std::back_inserter(output), " {}", 3.14);
	//std::cout << output << std::endl;

	//char output1[10] = {0};
	//std::format_to_n(output1, sizeof(output1), "{} {}", "keenjin test test test test test", 32131232);
	//std::cout << output1 << std::endl;

	//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	//auto s5 = converter.to_bytes(L"这是一个宽字符串");
	//auto s6 = converter.from_bytes(s5);
	////std::cout << s5 << std::endl;
	//std::wcout << s6 << std::endl;

	//std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter1(new std::codecvt_byname<wchar_t, char, std::mbstate_t>("zh_CN"));
	//auto s7 = converter1.to_bytes(L"这是一个宽字符串");
	//auto s8 = converter1.from_bytes("这是一个窄字符串");
	//std::cout << s7 << std::endl;
	//std::wcout << s8 << std::endl;

	//std::string test1 = "A43gbFr";
	//std::string test2 = "a43GbfR";
	//
	//std::string test1Lower = test1;
	//std::string test1Upper = test1;
	//std::transform(test1Lower.begin(), test1Lower.end(), test1Lower.begin(), ::tolower);
	//std::transform(test1Upper.begin(), test1Upper.end(), test1Upper.begin(), ::toupper);

	//auto ret1 = std::equal(test1.begin(), test1.end(), test2.begin(), test2.end(), [](char a, char b) {return std::tolower(a) == std::tolower(b); });
	//auto ret2 = std::equal(test1.begin(), test1.end(), test2.begin(), [](char a, char b) {return std::tolower(a) == std::tolower(b); });

	//std::string test3 = "35B78A";
	//auto ret3 = std::stoi(test3);
	//std::string test4 = "3.141592632323984769238382746";
	//auto ret4 = std::stod(test4);
	//auto ret5 = std::stof(test4);

	//{
	//	std::string param1 = "test test";
	//	int param2 = 321;
	//	std::cout << "caller thread is " << std::this_thread::get_id() << std::endl;
	//	std::jthread jt{ [param1, param2](std::stop_token st) {
	//		std::cout << "param1: " << param1 << ", param2: " << param2 << std::endl;
	//		std::cout << "thread start, tid: " << std::this_thread::get_id() << std::endl;
	//		while (!st.stop_requested()) {
	//			std::cout << "tid: " << std::this_thread::get_id() << std::endl;
	//			// 当前cpu时间片直接放弃，有限调度其他cpu
	//			std::jthread jt1([]() {
	//				std::cout << "second thread, tid: " << std::this_thread::get_id() << std::endl;
	//			});
	//			std::this_thread::yield();				
	//			
	//			std::this_thread::sleep_for(std::chrono::seconds(1));
	//		}

	//		std::cout << "thread stop, tid: " << std::this_thread::get_id() << std::endl;
	//	} };

	//	std::stop_source ss = jt.get_stop_source();

	//	std::stop_callback callback(jt.get_stop_token(), []() {
	//		std::cout << "thread stop callback, tid: " << std::this_thread::get_id() << std::endl;
	//	});

	//	std::this_thread::sleep_for(std::chrono::seconds(5));

	//	// 在jt析构的时候，会自动调用下面这俩
	//	jt.request_stop();
	//	jt.join();
	//}

	//auto t_func = [&]() {
	//	static std::mutex mtx;
	//	std::lock_guard<std::mutex> lock(mtx);
	//};

//{
//	std::async(std::launch::async, [](int param) { std::cout << "first thread: " << std::this_thread::get_id() << ", param: " << param << std::endl; std::this_thread::sleep_for(std::chrono::seconds(5)); }, 1111);
//	std::async(std::launch::async, [](std::string param) { std::cout << "second thread: " << std::this_thread::get_id() << ", param: " << param << std::endl; std::this_thread::sleep_for(std::chrono::seconds(5)); }, "Keen");
//	auto f1 = std::async(std::launch::deferred, [](std::string param) { std::cout << "three thread: " << std::this_thread::get_id() << ", param: " << param << std::endl; std::this_thread::sleep_for(std::chrono::seconds(5)); }, "Keen");
//	auto f2 = std::async(std::launch::deferred, [](std::string param) { std::cout << "four thread: " << std::this_thread::get_id() << ", param: " << param << std::endl; std::this_thread::sleep_for(std::chrono::seconds(5)); }, "Keen");
//	f1.wait();
//	f2.wait();
//}
//{
//	std::async(
//		std::launch::async, 
//		[](std::string param) { 
//		std::cout << "five thread: " << std::this_thread::get_id() << ", param: " << param << std::endl; 
//		std::this_thread::sleep_for(std::chrono::seconds(5)); 
//		std::async(std::launch::async, []() {
//			std::cout << "six thread: " << std::this_thread::get_id() << std::endl;
//		});
//	}, "Keen");
//}

/*int a = 10;
auto f = std::async(std::launch::async, [=]() { return a + 10; });
auto r = f.get();

std::cout << "calling thread, tid: " << std::this_thread::get_id() << std::endl;
std::packaged_task<int(int, int)> task([](int a, int b) {
	std::cout << "work thread, tid: " << std::this_thread::get_id() << std::endl;
	return a + b;
});
auto f1 = task.get_future();
std::jthread task_td{std::move(task), 14, 15};
task_td.join();
auto r1 = f1.get();
std::cout << "task result: " << r1 << std::endl;

{
	std::mutex mtx;
	std::condition_variable cv;
	bool ready = false;
	std::jthread t1{ [&]() {
		std::cout << "begin working now, tid: " << std::this_thread::get_id() << std::endl;
		std::unique_lock<std::mutex> lck(mtx);
		while (!ready)
			cv.wait(lck);

		std::cout << "end working now, tid: " << std::this_thread::get_id() << std::endl;
	} };

	std::jthread t2{ [&]() {
		std::cout << "begin working now, tid: " << std::this_thread::get_id() << std::endl;
		std::unique_lock<std::mutex> lck(mtx);
		while (!ready)
			cv.wait(lck);

		std::cout << "end working now, tid: " << std::this_thread::get_id() << std::endl;
	} };

	std::jthread t3{ [&]() {
		std::unique_lock<std::mutex> lck(mtx);
		std::cout << "begin notify, tid: " << std::this_thread::get_id() << std::endl;
		ready = true;
		cv.notify_all();
		std::cout << "end notify, tid: " << std::this_thread::get_id() << std::endl;
	} };

	t1.join();
	t2.join();
	t3.join();

}

{
	std::shared_mutex mtx;
	int sharedVariable = 30;
	std::jthread read1{ [&]() {
		std::shared_lock<std::shared_mutex> rl(mtx);
		int s = sharedVariable;
		std::cout << "tid: " << std::this_thread::get_id() << ", v: " << s << std::endl;
	} };

	std::jthread read2{ [&]() {
		std::shared_lock<std::shared_mutex> rl(mtx);
		int s = sharedVariable;
		std::cout << "tid: " << std::this_thread::get_id() << ", v: " << s << std::endl;
	} };

	std::jthread write1{ [&]() {
		std::unique_lock<std::shared_mutex> rl(mtx);
		sharedVariable = 100;
		std::cout << "tid: " << std::this_thread::get_id() << ", v: " << sharedVariable << std::endl;
	} };

	std::jthread write2{ [&]() {
		std::unique_lock<std::shared_mutex> rl(mtx);
		sharedVariable = 300;
		std::cout << "tid: " << std::this_thread::get_id() << ", v: " << sharedVariable << std::endl;
	} };
}*/

	{
		// 字符串分割
		std::istringstream ss("");
		std::string elem;
		while (std::getline(ss, elem, '|')) {
			std::cout << elem << std::endl;
		}
	}

	{
		// 创建文件
		std::ofstream fout("E:\\keen.txt");
		fout.close();

		// 文件拷贝
		std::error_code err;
		auto ret1 = std::filesystem::copy_file("E:\\keen.txt", "E:\\Cache\\keen.txt", std::filesystem::copy_options::skip_existing, err);
		std::cout << "ret: " << ret1 << ", err: " << err.message() << std::endl;
		auto ret2 = std::filesystem::remove("E:\\keen.txt", err);
		std::cout << "ret: " << ret2 << ", err: " << err.message() << std::endl;

		// 文件读取
		std::basic_ifstream<char8_t> fin("E:\\Cache\\test.txt", std::ios::binary);
		fin.imbue(std::locale(std::locale::empty(), new std::codecvt<wchar_t, char8_t, std::mbstate_t>));
		std::wcout.imbue(std::locale("chs"));
		std::array<char8_t, 1024> buffer;
		while (!fin.eof()) {
			buffer.fill(0);
			fin.read(&buffer[0], buffer.size());

			// utf8转wchar_t
			auto [ret, err] = string_convert::Utf8ToGBK(buffer.data());
			auto len = ret.length();
			if (!err) {
				auto result = string_convert::Utf16AsWide(ret);
				auto ll = result.size();
			}
			/*const std::codecvt<char16_t, char8_t, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char16_t, char8_t, std::mbstate_t>>(std::locale(".936"));
			std::vector<char16_t> buf(cvt.max_length() * (buffer.size() + 1));

			std::mbstate_t state;
			const char8_t* from_next = nullptr;
			char16_t* to_next = nullptr;
			auto result = cvt.in(state, buffer.data(), buffer.data() + buffer.size(), from_next, buf.data(), buf.data() + buf.size(), to_next);
			if (result == std::codecvt_base::ok) {
				std::wstring ws(buf.data(), to_next);
				auto new_size = ws.size();
			}*/			
		}
	}

	{
		// 遍历当前目录
		auto path = "E:\\Cache";
		std::cout << "enum path: " << path << std::endl;
		for (auto& p : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied)) {
			std::cout << p << std::endl;
		}

		// 递归遍历所有子目录（深度优先）
		auto path1 = "E:\\Cache";
		std::cout << "enum path: " << path1 << std::endl;
		for (auto& p : std::filesystem::recursive_directory_iterator(path1, std::filesystem::directory_options::skip_permission_denied)) {
			std::cout << p << std::endl;
		}

		std::filesystem::path file = "E:\\Cache\\test.txt";
		std::cout << "path: " << file << std::endl;
		std::cout << "filename: " << file.filename() << std::endl;
		std::cout << "parentpath: " << file.parent_path() << std::endl;
		std::cout << "relative: " << file.relative_path() << std::endl;
		std::cout << "root directory: " << file.root_directory() << std::endl;
		std::cout << "root path: " << file.root_path() << std::endl;
		std::cout << "root name: " << file.root_name() << std::endl;
		std::cout << "ext: " << file.extension() << std::endl;
		std::cout << "path append: " << file.append("subpath") << std::endl;

		std::filesystem::path dir = "E:\\Cache\\test";
		std::cout << "path: " << file << std::endl;
		std::cout << "filename: " << file.filename() << std::endl;
		std::cout << "parentpath: " << file.parent_path() << std::endl;
		std::cout << "relative: " << file.relative_path() << std::endl;
		std::cout << "root directory: " << file.root_directory() << std::endl;
		std::cout << "root path: " << file.root_path() << std::endl;
		std::cout << "root name: " << file.root_name() << std::endl;
		std::cout << "ext: " << file.extension() << std::endl;
		std::cout << "path append: " << file.append("subpath") << std::endl;
	}

	return 0;
}