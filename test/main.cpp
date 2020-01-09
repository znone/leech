#include "pch.h"
#include <iostream>

using namespace std;

void test_yaml();
void test_json();
void test_config();

int main()
{
	cout << "yaml: " << endl;
	test_yaml();
	cout << "json: " << endl;
	test_json();
	cout << "config: " << endl;
	test_config();

	return 0;
}