#ifndef _MODEL_CONFIG_HPP_
#define _MODEL_CONFIG_HPP_

#pragma once

#include <libconfig.h++>
#include <fstream>
#include <iomanip>

namespace leech
{

	namespace config
	{

		class document
		{
		public:
			typedef libconfig::Setting element_type;

			document() = default;

			void save(FILE* stream)
			{
				_config.write(stream);
			}
			void save_file(const char* filename)
			{
				_config.writeFile(filename);
			}

			element_type& root() { return _config.getRoot(); }
			const element_type& root() const { return _config.getRoot(); }
			const element_type& operator[](const char* name) const
			{
				return root()[name];
			}
			element_type& operator[](const char* name)
			{
				return root()[name];
			}
			const element_type& child(const element_type& element, const char* name) const
			{
				return element[name];
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
			void put(element_type& element, const std::vector<int>& v) const
			{
				for (auto i : v)
					element.add(element_type::TypeInt) = i;
			}
			void put(element_type& element, const std::vector<int64_t>& v) const
			{
				for (auto i : v)
					element.add(element_type::TypeInt64) = i;
			}
			void put(element_type& element, const std::vector<std::string>& v) const
			{
				for (auto i : v)
					element.add(element_type::TypeString) = i;
			}
			void put(element_type& element, const std::vector<bool>& v) const
			{
				for (auto i : v)
					element.add(element_type::TypeBoolean) = i;
			}
			void put(element_type& element, const std::vector<double>& v) const
			{
				for (auto i : v)
					element.add(element_type::TypeFloat) = i;
			}

			void get(const element_type& element, bool &v) const
			{
				v=element;
			}
			void get(const element_type& element, int &v) const
			{
				v = element;
			}
			void get(const element_type& element, unsigned int &v) const
			{
				v = element;
			}
			void get(const element_type& element, long &v) const
			{
				v = element;
			}
			void get(const element_type& element, unsigned long &v) const
			{
				v = element;
			}
			void get(const element_type& element, long long &v) const
			{
				v = element;
			}
			void get(const element_type& element, unsigned long long &v) const
			{
				v = element;
			}
			void get(const element_type& element, float &v) const
			{
				v = element;
			}
			void get(const element_type& element, double &v) const
			{
				v = element;
			}
			void get(const element_type& element, std::string &v) const
			{
				v = element.operator std::string();
			}
			template<typename T>
			void get(const element_type& element, std::vector<T>& v) const
			{
				v.reserve(element.getLength());
				for (int i = 0; i != element.getLength(); i++)
					v.push_back(element[i]);
			}

			void load(const char* input)
			{
				_config.readString(input);
			}
			void load(const std::string& input)
			{
				_config.readString(input);
			}
			void load(FILE* input)
			{
				_config.read(input);
			}
			void load_file(const char* filename)
			{
				_config.readFile(filename);
			}

		private:
			libconfig::Config _config;
		};

	}

}

#endif //_MODEL_CONFIG_HPP_

