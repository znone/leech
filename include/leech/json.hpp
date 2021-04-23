#ifndef _MODEL_JSON_HPP_
#define _MODEL_JSON_HPP_

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>

namespace leech
{

namespace json
{

	class document
	{
	public:
		typedef nlohmann::json::value_type element_type;

		document() = default;
		explicit document(const element_type& root) noexcept : _root(root) { }
		explicit document(element_type&& root) noexcept : _root(std::move(root)) { }

		std::string save(const int indent = -1,	char indent_char = ' ',
			bool ensure_ascii = false)
		{
			return _root.dump(indent, indent_char, ensure_ascii);
		}
		void save(std::ostream& os, 
			const int indent = -1, const char indent_char = ' ')
		{
			using namespace std;
			os << setw(indent) << setfill(indent_char) << _root;
		}
		void save_file(const char* filename,
			const int indent = -1, const char indent_char = ' ')
		{
			std::ofstream fs(filename, std::ios::trunc);
			save(fs, indent, indent_char);
			fs.close();
		}

		element_type& root() { return _root; }
		const element_type& root() const { return _root; }
		const element_type& operator[](const char* name) const
		{
			return _root.at(name);
		}
		element_type& operator[](const char* name)
		{
			return _root.at(name);
		}
		const element_type& child(const element_type& element, const char* name) const
		{
			return element.at(name);
		}
		element_type& child(element_type& element, const char* name) const
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
			element.get_to(v);
		}

	private:
		nlohmann::json::value_type _root;
	};

	inline document load(const char* input)
	{
		return document(nlohmann::json::parse(input));
	}
	inline document load(const std::string& input)
	{
		return document(nlohmann::json::parse(input));
	}
	inline document load(std::istream& input)
	{
		document::element_type json;
		input >> json;
		return document(std::move(json));
	}
	inline document load_file(const char* filename)
	{
		std::ifstream fs(filename);
		return load(fs);
	}

}

}

#define STRUCT_FROM_JSON(S) \
namespace nlohmann { \
	template <> struct adl_serializer<S> { \
		static void from_json(const json& j, S& field) { \
			leech::get(leech::json::document(j), field); \
		} \
	}; \
}

#define STRUCT_TO_JSON(S) \
namespace nlohmann { \
	template <> struct adl_serializer<S> { \
		static void to_json(json& j, const S& field) { \
			j=move(leech::put(leech::json::document(), field).root()); \
		} \
	}; \
}

#endif //_MODEL_JSON_HPP_

