// test_any_console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <initializer_list>

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

int main()
{
    std::cout << "Hello World!\n";

	std::initializer_list<int> a = { 1,2,3 };
	Test b = { 1, 2 };
	Test1 c = { 1,2,3 };

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
