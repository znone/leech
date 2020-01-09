#include "pch.h"
#include "test.h"
#include <leech/config.hpp>

using namespace std;

void test_config()
{
	MyStruct s{ };

	try
	{
		leech::config::document doc;
		doc.load_file("test.cfg");

		leech::get(doc, doc["aaa"], s);
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


