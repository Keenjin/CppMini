// test_any_console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <initializer_list>
#include <memory>
#include <any>
#include <set>
#include <functional>

class Test
{
public:
	template<class T>
	constexpr Test(std::initializer_list<T> arg)
	{
		for (auto a : arg)
		{
			std::cout << a << std::endl;
		}
	}
};

template<class T>
bool Print(T t)
{
	std::cout << t << std::endl;
	return true;
}

// Note EmptyTrait is always regarded as valid to support filtering.
//template <class ValidTraits, class T>
//inline constexpr bool IsValidTrait() {
//	return std::is_constructible<ValidTraits, T>::value ||
//		std::is_same<T, EmptyTrait>::value;
//}

class Test1
{
public:
	template<class... ArgType>
	constexpr Test1(ArgType... arg)
	{
		auto a = std::initializer_list<bool>({ Print(arg)... });
	}
};

class A {
public:
	virtual ~A() = default;
	virtual void Print() {
		std::cout << "A" << std::endl;
	}
};

class B : public A {
public:
	virtual void Print() {
		std::cout << "B" << std::endl;
	}
};

void FTest1() {
	int a = 3;
	a += 4;
	int b = a + 4;
}

void FTest2() {
	int a = 3;
	a += 4;
	int b = a + 4;
}

void FuncTest(int a, int b, void* context) {
	
}


#include <windows.h>

extern "C" WCHAR* sub_1025E453(WCHAR* a, WCHAR* b);

int main()
{
	HKEY keyResult = nullptr;
	DWORD dwLen = 0;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Services\\nvlddmkm\\", 0, KEY_READ, &keyResult) &&
		ERROR_SUCCESS == RegGetValueW(keyResult, 0, L"ImagePath", 2, 0, 0, &dwLen)) {
		TCHAR *pPath = (TCHAR*)malloc(dwLen + sizeof(TCHAR));
		ZeroMemory(pPath, dwLen + sizeof(TCHAR));
		if (ERROR_SUCCESS == RegGetValueW(keyResult, 0, L"ImagePath", 2, 0, pPath, &dwLen)) {
			sub_1025E453(pPath, (WCHAR*)L"System32");

			dwLen = dwLen / 2;
			// 将\SystemRoot\替换为GetSystemDirectW的值
			TCHAR szRoot[13] = { 0 };
			memcpy(szRoot, pPath, 12 * sizeof(TCHAR));
			if (_wcsicmp(szRoot, L"\\SystemRoot\\") == 0) {
				TCHAR szWinDir[1024] = { 0 };
				GetSystemWindowsDirectoryW(szWinDir, 1023);
				DWORD dwLenWin = wcslen(szWinDir);
				TCHAR *pRealPath = (TCHAR*)malloc((dwLenWin + dwLen - 10) * sizeof(TCHAR));
				ZeroMemory(pRealPath, (dwLenWin + dwLen - 10) * sizeof(TCHAR));
				memcpy(pRealPath, szWinDir, dwLenWin * sizeof(TCHAR));
				memcpy(pRealPath + dwLenWin, pPath + 11, (dwLen - 11) * sizeof(TCHAR));

				DWORD dwAttr = GetFileAttributesW(pRealPath);
				if (dwAttr != FILE_INVALID_FILE_ID) {
					bool isSymlnk = dwAttr & FILE_ATTRIBUTE_REPARSE_POINT;

					std::cout << isSymlnk;
				}
				

				free(pRealPath);
			}
		}
		free(pPath);
	}
	if (keyResult) {
		RegCloseKey(keyResult);
	}
	
    std::cout << "Hello World!\n";

	std::shared_ptr<A> obj_a = std::make_shared<B>();
	obj_a->Print();

	std::shared_ptr<B> obj_b = std::dynamic_pointer_cast<B>(obj_a);
	obj_b->Print();

	std::initializer_list<int> a = { 1,2,3 };
	//std::initializer_list<int> b = a;
	Test b = { 1, 2 };
	Test1 c = { 1,2,3 };

	std::any any_obj = 1.0f;

	//std::set<int, int> mysets;
	//mysets.insert(1, 2);
	//mysets.insert(1, 3);
	//mysets.insert(1, 2);

	A test_a;
	std::function<void()> fa = std::bind(&A::Print, test_a);
	std::function<void()> fb = FTest2;
	auto fa_t = fa.target_type().name();
	auto fb_t = fb.target_type().name();

	std::string aaa = "111";
	std::string bbb = "111";
	if (aaa == bbb)
		std::cout << "aaa == bbb" << std::endl;



	system("pause");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
