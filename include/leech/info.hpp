#ifndef _MODEL_INFO_HPP_
#define _MODEL_INFO_HPP_

#pragma once

#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace leech
{

namespace info
{

using namespace boost::property_tree;
	
class document
{
public:
	typedef ptree element_type;
	document() = default;
	explicit document(ptree&& tree) : _root(std::forward<ptree>(tree)) { }
	document(document&& src) : _root(std::forward<ptree>(src._root)) { }

	document& operator=(document&& src)
	{
		if (this != &src)
		{
			_root = std::forward<ptree>(src._root);
		}
		return *this;
	}

	void save(std::ostream& os)
	{
		write_info(os, _root);
	}
	void save(std::string& text)
	{
		std::ostringstream oss;
		save(oss);
		text = oss.str();
	}
	void save_file(const std::string& filename)
	{
		write_info(filename, _root);
	}

	element_type& root() { return _root; }
	const element_type& root() const { return _root; }
	const element_type& operator[](const char* name) const
	{
		return _root.get_child(name);
	}
	element_type& operator[](const char* name)
	{
		return _root.get_child(name);
	}
	const element_type& child(const element_type& element, const char* name) const
	{
		return element.get_child(name);
	}
	element_type& child(element_type& element, const char* name) const
	{
		return element.get_child(name);
	}

	template<typename T>
	void put(element_type& element, const T& v) const
	{
		element.put_value(v);
	}
	template<typename T>
	void put(element_type& element, const std::vector<T>& v) const
	{
		for (size_t i = 0; i != v.size(); i++)
		{
			document temp;
			leech::put(temp, v[i]);
			element.put_child(std::to_string(i), temp.root());
		}
	}
	template<typename T>
	void put(element_type& element, const std::map<std::string, T>& v) const
	{
		for (const auto& item : v)
		{
			document temp;
			leech::put(temp, item.second);
			element.put_child(item.first, temp.root());
		}
	}
	template<typename T>
	void put(element_type& element, const std::unordered_map<std::string, T>& v) const
	{
		for (const auto& item : v)
		{
			document temp;
			leech::put(temp, item.second);
			element.put_child(item.first, temp.root());
		}
	}

	template<typename T>
	void get(const element_type& element, T& v) const
	{
		element.get_value(v);
	}
	template<typename T>
	void get(const element_type& element, std::vector<T>& v) const
	{
		v.clear();
		for (const auto& item : element)
		{
			T vi;
			leech::get(*this, item.second, vi);
			v.push_back(std::move(vi));
		}
	}
	template<typename T>
	void get(const element_type& element, std::map<std::string, T>& v) const
	{
		v.clear();
		for (const auto& item : element)
		{
			T vi;
			leech::get(*this, item.second, vi);
			v.emplace(item.first, std::move(vi));
		}
	}
	template<typename T>
	void get(const element_type& element, std::unordered_map<std::string, T>& v) const
	{
		v.clear();
		for (const auto& item : element)
		{
			v.emplace(item.first, item.second.get_value<T>());
		}
	}

	static document load(std::istream& is)
	{
		document doc;
		read_info(is, doc._root);
		return doc;
	}
	static document load(const std::string& text)
	{
		std::istringstream iss(text);
		return load(iss);
	}
	static document load_file(const std::string& filename)
	{
		document doc;
		read_info(filename, doc._root);
		return doc;
	}

private:
	ptree _root;

};


}

}

#endif //_MODEL_INFO_HPP_
