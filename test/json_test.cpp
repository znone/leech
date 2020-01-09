#include "pch.h"
#include "test.h"
#include <leech/json.hpp>
#include <fstream>

using namespace std;

void test_json()
{
	MyStruct s{ };

	try
	{
		leech::json::document ar=leech::json::load_file("test.json");
		leech::get(ar, ar["aaa"], s);
		cout << boolalpha;
		leech::for_each(s, [](const char* name, const auto& v) {
			cout << name << ":" << v << ", ";
		});
		cout << endl;
		leech::find_field<MyStruct>("a", [](auto& field_info) {
			cout << "found field:" << field_info.name() << endl;
		});
	}
	catch (std::exception& e)
	{
		cerr << e.what() << endl;
	}
}

