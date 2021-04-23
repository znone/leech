#ifndef _MODEL_YAML_HPP_
#define _MODEL_YAML_HPP_

#pragma once

#include <yaml-cpp/yaml.h>
#include <sstream>
#include <fstream>

namespace leech 
{
namespace yaml
{

class document
{
public:
	typedef YAML::Node element_type;

	explicit document(element_type root) noexcept : _root(root) { }

	std::string save()
	{
		std::ostringstream oss;
		save(oss);
		return oss.str();
	}
	void save(std::ostream& os)
	{
		YAML::Emitter emitter(os);
		emitter << _root;
		if(os) os.flush();
	}
	void save_file(const char* filename)
	{
		std::ofstream fs(filename, std::ios::trunc);
		save(fs);
		fs.close();
	}

	element_type& root() noexcept { return _root;  }
	const element_type& root() const noexcept { return _root; }
	const element_type& operator[](const char* name) const
	{
		return _root[name];
	}
	element_type operator[](const char* name)
	{
		return _root[name];
	}
	const element_type& child(const element_type& element, const char* name) const
	{
		return element[name];
	}
	element_type child(element_type& element, const char* name) const
	{
		return element[name];
	}

	template<typename T>
	void put(element_type& element, const T& v) const
	{
		element = v;
	}

	template<typename T>
	void get(const element_type& element, T& v) const
	{
		v=element.as<T>();
	}

private:
	element_type _root;
};

inline document load(const char* input)
{
	return document(YAML::Load(input));
}
inline document load(const std::string& input)
{
	return document(YAML::Load(input));
}
inline document load(std::istream& input)
{
	return document(YAML::Load(input));
}
inline document load_file(const char* filename)
{
	return document(YAML::LoadFile(filename));
}

}
}

#define STRUCT_FROM_YAML(S) \
namespace YAML { \
	template <> struct as_if<S, void> { \
	explicit as_if(const Node& node_) : node(node_) {} \
		const Node& node; \
		S operator()() const { \
			S value; \
			leech::get(leech::yaml::document(node), value); \
			return value; \
		} \
	}; \
}

 
#endif //_MODEL_YAML_HPP_

