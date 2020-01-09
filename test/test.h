#pragma once

#include <leech/model.hpp>
#include <iostream>

struct MyNode
{
	std::string name;
};

struct Base
{
	int id;
	bool enabled;
};

struct MyStruct : public Base
{
	int a, b, c;
	std::vector<int> d;
	MyNode node;
};

STRUCT_MODEL(Base, id, enabled)
STRUCT_MODEL(MyNode, name)
STRUCT_MODEL_INHERIT(MyStruct, (Base), a, b, c, d, node)

inline std::ostream& operator<<(std::ostream& os, const MyNode& v)
{
	std::cout << "{MyNode}";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const MyStruct& v)
{
	std::cout << "{MyStruct}";
	return os;
}
