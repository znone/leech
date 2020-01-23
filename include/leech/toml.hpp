#ifndef _LEECH_TOML_HPP_
#define _LEECH_TOML_HPP_

#pragma once

#include <toml.hpp>
#include <sstream>
#include <fstream>

namespace leech
{
	namespace toml
	{

		class document
		{
		public:
			typedef ::toml::value element_type;

			explicit document(element_type root) noexcept : _root(root) { }

			std::string save()
			{
				std::ostringstream oss;
				save(oss);
				return oss.str();
			}
			void save(std::ostream& os)
			{
				os << _root;
			}
			void save_file(const char* filename)
			{
				std::ofstream fs(filename, std::ios::trunc);
				save(fs);
				fs.close();
			}

			element_type& root() noexcept { return _root; }
			const element_type& root() const noexcept { return _root; }
			const element_type& operator[](const char* name) const
			{
				return _root.at(name);
			}
			element_type& operator[](const char* name)
			{
				return _root[name];
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
				v = ::toml::get<T>(element);
			}

		private:
			element_type _root;
		};

		inline document load(const char* input)
		{
			return document(::toml::parse(input));
		}
		inline document load(const std::string& input)
		{
			return document(::toml::parse(input));
		}
		inline document load(std::istream& input)
		{
			return document(::toml::parse(input));
		}
		inline document load_file(const char* filename)
		{
			std::ifstream fs(filename);
			return document(::toml::parse(fs));
		}

	}
}

#endif //_LEECH_TOML_HPP_
