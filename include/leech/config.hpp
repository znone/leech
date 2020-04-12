#ifndef _MODEL_CONFIG_HPP_
#define _MODEL_CONFIG_HPP_

#pragma once

#include <libconfig.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <tuple>
#include <unordered_map>

namespace leech
{

	namespace config
	{
		class error : public std::exception
		{
		public:
			explicit error(const char* text = nullptr) : _error(CONFIG_ERR_NONE), _text(text) { }
			explicit error(config_t& config)
			{
				_error = config_error_type(&config);
				const char* text = config_error_text(&config);
				const char* file = config_error_file(&config);
				int line = config_error_line(&config);

				if (_error != CONFIG_ERR_NONE)
				{
					std::ostringstream oss;
					if (file) oss << file;
					if (line) oss << "(" << line << ")";
					oss << "error(" << _error << "): " << text;
					_text = oss.str();
				}
			}

			const char* what() const noexcept override { return _text.data(); }
			config_error_t code() const noexcept { return _error;  }

		protected:
			config_error_t _error;
			std::string _text;
		};

		class invalid_setting : public error
		{
		public:
			invalid_setting() : error("invalid config setting") { }
		};

		class convert_error : public error
		{
		public:
			explicit convert_error(const char* to)
			{
				std::ostringstream oss;
				oss << "cannot convert the config setting to \"" << to << "\"";
				_text = oss.str();
			}
		};

		template<typename T>
		struct config_type;

		template<>
		struct config_type<int>
		{
			enum { value = CONFIG_TYPE_INT };
		};

		template<>
		struct config_type<long long>
		{
			enum { value = CONFIG_TYPE_INT64 };
		};

		template<>
		struct config_type<double>
		{
			enum { value = CONFIG_TYPE_FLOAT };
		};

		template<>
		struct config_type<bool>
		{
			enum { value = CONFIG_TYPE_BOOL };
		};

		template<>
		struct config_type<const char*>
		{
			enum { value = CONFIG_TYPE_STRING };
		};

		template<>
		struct config_type<std::string>
		{
			enum { value = CONFIG_TYPE_STRING };
		};

		template<typename T>
		struct config_type<std::vector<T>>
		{
			enum { value = CONFIG_TYPE_ARRAY };
		};

		template<typename T>
		struct config_type<std::map<std::string, T>>
		{
			enum { value = CONFIG_TYPE_GROUP };
		};

		template<typename T>
		struct config_type<std::unordered_map<std::string, T>>
		{
			enum { value = CONFIG_TYPE_GROUP };
		};

		template<typename T1, typename T2>
		struct config_type<std::pair<T1, T2>>
		{
			enum { value = CONFIG_TYPE_LIST };
		};

		template<typename... Types>
		struct config_type<std::tuple<Types...>>
		{
			enum { value = CONFIG_TYPE_LIST };
		};

		class setting;

		template<typename T>
		struct assign
		{
			void operator()(setting& e, const T& v) const;
		};
		template<typename T>
		struct as_to
		{
			T& operator()(const setting& e, T& v) const;
		};

		class setting
		{
		public:
			explicit setting(config_setting_t* setting)
			{
				if (setting == nullptr)
					throw invalid_setting();
				_setting = setting;
			}

			bool is_root() const { return config_setting_is_root(_setting); }
			size_t size() const { return config_setting_length(_setting);  }
			int type() const { return config_setting_type(_setting); }
			const char* name() const { return config_setting_name(_setting); }
			setting parent() const { return setting(config_setting_parent(_setting)); }
			setting lookup(const char* path) const
			{
				return setting(config_setting_lookup(_setting, path));
			}

			setting operator[](int index) const
			{
				setting(config_setting_get_elem(_setting, index));
			}

			setting operator[](const char* name)
			{
				config_setting_t * child = config_setting_get_member(_setting, name);
				if (child)
					return setting(child);
				else if (config_setting_is_group(_setting))
					return setting(config_setting_add(_setting, name, CONFIG_TYPE_NONE));
				else
					throw invalid_setting();
			}
			setting operator[](const char* name) const
			{
				return setting(config_setting_get_member(_setting, name));
			}

			template<typename T>
			T as() const 
			{ 
				T v;
				return as_to(v);
			}
			template<typename T>
			T& as_to(T& v) const;

			template<typename T1, typename T2>
			std::pair<T1, T2>& as_to(std::pair<T1, T2>& v) const
			{
				if (type() != CONFIG_TYPE_LIST)
					throw convert_error("list");

				leech::config::as_to<T1>()(setting(config_setting_get_elem(_setting, 0)), v.first);
				leech::config::as_to<T2>()(setting(config_setting_get_elem(_setting, 1)), v.second);
				return v;
			}

			template<typename... Types>
			std::tuple<Types...>& as_to(std::tuple<Types...>& v) const
			{
				if (type() != CONFIG_TYPE_LIST)
					throw convert_error("list");

				as_tuple < std::make_index_sequence < std::tuple_size<std::tuple<Types...>>::value> >(v);
				return v;
			}

			template<typename T>
			std::vector<T>& as_to(std::vector<T>& v) const
			{
				if (type() != CONFIG_TYPE_ARRAY)
					throw convert_error("array");

				size_t length = config_setting_length(_setting);
				v.resize(length);
				for (size_t i = 0; i != length; i++)
				{
					leech::config::as_to<T>()(setting(config_setting_get_elem(_setting, i)), v[i]);
				}

				return v;
			}

			template<typename T>
			std::map<std::string, T>& as_to(std::map<std::string, T>& v) const
			{
				if (type() != CONFIG_TYPE_GROUP)
					throw convert_error("group");

				size_t length = config_setting_length(_setting);
				v.clear();
				for (size_t i = 0; i != length; i++)
				{
					setting element(config_setting_get_elem(_setting, i));
					leech::config::as_to<T>()(element, v[element.name()]);
				}

				return v;
			}

			template<typename T>
			std::unordered_map<std::string, T>& as_to(std::unordered_map<std::string, T>& v) const
			{
				if (type() != CONFIG_TYPE_GROUP)
					throw convert_error("group");

				size_t length = config_setting_length(_setting);
				v.clear();
				for (size_t i = 0; i != length; i++)
				{
					setting element(config_setting_get_elem(_setting, i));
					leech::config::as_to<T>()(element, v[element.name()]);
				}

				return v;
			}

			setting& operator=(int value)
			{				
				if (type() != CONFIG_TYPE_INT)
					change(CONFIG_TYPE_INT);
				return verify(config_setting_set_int(_setting, value));
			}
			setting& operator=(long long value)
			{
				if (type() != CONFIG_TYPE_INT)
					change(CONFIG_TYPE_INT64);
				return verify(config_setting_set_int64(_setting, value));
			}
			setting& operator=(double value)
			{
				if (type() != CONFIG_TYPE_INT)
					change(CONFIG_TYPE_FLOAT);
				return verify(config_setting_set_float(_setting, value));
			}
			setting& operator=(bool value)
			{
				if (type() != CONFIG_TYPE_INT)
					change(CONFIG_TYPE_BOOL);
				return verify(config_setting_set_bool(_setting, value));
			}
			setting& operator=(const char* value)
			{
				change(CONFIG_TYPE_STRING);
				return verify(config_setting_set_string(_setting, value));
			}
			setting& operator=(const std::string& value)
			{
				return *this = value.data();
			}
			template<typename T>
			setting& operator=(const std::vector<T>& value)
			{
				change(CONFIG_TYPE_ARRAY);
				clear();
				for (const auto& item : value)
				{
					setting element(config_setting_add(_setting, NULL, config_type<T>::value));
					leech::config::assign<T>()(element, item);
				}
				return *this;
			}

			template<typename T1, typename T2>
			setting& operator=(const std::pair<T1, T2>& value)
			{
				change(CONFIG_TYPE_LIST);
				clear();
				assign_tuple < std::make_index_sequence < std::tuple_size<std::pair<T1, T2>>::value> >(value);
				return *this;
			}
			template<typename... Types>
			setting& operator=(const std::tuple<Types...>& value)
			{
				change(CONFIG_TYPE_LIST);
				clear();
				assign_tuple < std::make_index_sequence < std::tuple_size<std::tuple<Types...>>::value> > (value);
				return *this;
			}

			template<typename T>
			setting& operator=(const std::map<std::string, T>& value)
			{
				change(CONFIG_TYPE_GROUP);
				clear();
				for (const auto& item : value)
				{
					setting element(config_setting_add(_setting, item.first, config_type<T>::value));
					leech::config::assign<T>()(element, item.second);
				}
				return *this;
			}
			template<typename T>
			setting& operator=(const std::unordered_map<std::string, T>& value)
			{
				change(CONFIG_TYPE_GROUP);
				clear();
				for (const auto& item : value)
				{
					setting element(config_setting_add(_setting, item.first, config_type<T>::value));
					leech::config::assign<T>()(element, item.second);
				}
				return *this;
			}

			void clear()
			{
				int count = config_setting_length(_setting);
				for (int i = 0; i != count; i++)
					verify(config_setting_remove_elem(_setting, i));
			}

		private:
			config_setting_t* _setting;

			setting& verify(int code)
			{
				if (code == CONFIG_FALSE)
					throw error(*_setting->config);
				return *this;
			}
			void change(int type)
			{
				if (this->type() != type)
				{
					config_setting_t* parent = config_setting_parent(_setting);
					if (parent)
					{
						std::string name = this->name();
						config_setting_remove(parent, name.data());
						config_setting_add(parent, name.data(), type);
					}
					else
					{
						config_t* config = _setting->config;
						const char** filenames = config->filenames;
						config->filenames = NULL;
#if LIBCONFIG_VER_MAJOR*100+LIBCONFIG_VER_MINOR > 170 
						config_clear(config);
#else
						config_read(config, NULL);
#endif 
						config->filenames = filenames;
						_setting = config_root_setting(config);
						_setting->type = type;
					}
				}
			}

			template<typename Tuple, size_t I>
			void assign_tuple_impl(const Tuple& value)
			{
				typedef typename std::tuple_element<I, Tuple>::value value_type;
				setting element(config_setting_add(_setting, NULL, config_type<value_type>()));
				leech::config::assign<value_type>()(element, std::get<I>(value));
			}
			template<typename Tuple, size_t I, size_t... Other>
			void assign_tuple_impl(const Tuple& value)
			{
				assign_tuple_impl<I>(value);
				assign_tuple_impl<Other...>(value);
			}
			template<typename Tuple, size_t I>
			void as_tuple_impl(Tuple& value)
			{
				typedef typename std::tuple_element<I, Tuple>::value value_type;
				setting element(config_setting_get_elem(_setting, I));
				leech::config::as_to<value_type>()(element, std::get<I>(value));
			}
			template<typename Tuple, size_t I, size_t... Other>
			void as_tuple_impl(Tuple& value)
			{
				as_tuple_impl<I>(value);
				as_tuple_impl<Other...>(value);
			}
			template<typename Tuples, std::size_t... Is>
			void assign_tuple(const Tuples& value, std::index_sequence<Is...>)
			{
				assign_tuple_impl<Is...>(value);
			}
			template<typename Tuples, std::size_t... Is>
			void as_tuple(const Tuples& value, std::index_sequence<Is...>)
			{
				as_tuple_impl<Is...>(value);
			}
		};

		template<>
		inline int& setting::as_to(int& v) const
		{
			if (!config_setting_is_number(_setting))
				throw convert_error("int");
			v = config_setting_get_int(_setting);
			return v;
		}
		template<>
		inline long long& setting::as_to(long long& v) const
		{
			if (!config_setting_is_number(_setting))
				throw convert_error("long long");
			v = config_setting_get_int64(_setting);
			return v;
		}
		template<>
		inline double& setting::as_to(double& v) const
		{
			if (!config_setting_is_number(_setting))
				throw convert_error("double");
			v = config_setting_get_float(_setting);
			return v;
		}
		template<>
		inline bool& setting::as_to(bool& v) const
		{
			if (type() != CONFIG_TYPE_BOOL)
				throw convert_error("bool");
			v = config_setting_get_bool(_setting) == CONFIG_TRUE;
			return v;
		}
		template<>
		inline const char* setting::as() const
		{
			if (type() != CONFIG_TYPE_STRING)
				throw convert_error("string");
			return config_setting_get_string(_setting);
		}
		template<>
		inline std::string& setting::as_to(std::string& v) const
		{
			if (type() != CONFIG_TYPE_STRING)
				throw convert_error("string");
			v = config_setting_get_string(_setting);
			return v;
		}

		template<typename T>
		inline void assign<T>::operator()(setting& e, const T& v) const
		{
			e = v;
		}

		template<typename T>
		inline T& as_to<T>::operator()(const setting& e, T& v) const
		{
			return e.as_to(v);
		}


		class document
		{
		public:
			typedef setting element_type;

			document()
			{
				config_init(&_config);
				config_set_auto_convert(&_config, CONFIG_TRUE);
			}
			document(const document&) = delete;
			document(document&& src)
			{
				move_config(&_config, &src._config);
			}
			~document()
			{
				config_destroy(&_config);
			}
			document& operator=(const document& src) = delete;
			document& operator=(document&& src)
			{
				if (this != &src)
				{
#if LIBCONFIG_VER_MAJOR*100+LIBCONFIG_VER_MINOR > 170 
					config_clear(&_config);
#else
					config_read(&_config, NULL);
#endif 
					move_config(&_config, &src._config);
				}
				return *this;
			}

			void save(FILE* stream)
			{
				config_write(&_config, stream);
			}
			void save_file(const char* filename)
			{
				verify(config_write_file(&_config, filename));
			}

			element_type root() const { return element_type(config_root_setting(&_config)); }
			element_type lookup(const char* path) const
			{
				return element_type(config_lookup(&_config, path));
			}

			void options(int value)
			{
				config_set_options(&_config, value);
			}
			int option() const
			{
				return config_get_options(&_config);
			}

			void tab_width(int width)
			{
				config_set_tab_width(&_config, width);
			}
			int tab_width() const
			{
				return config_get_tab_width(&_config);
			}

			void default_format(short width)
			{
				config_set_default_format(&_config, width);
			}
			short default_format() const
			{
				return config_get_default_format(&_config);
			}

			const element_type operator[](const char* name) const
			{
				return root()[name];
			}
			element_type operator[](const char* name)
			{
				return root()[name];
			}
			const element_type child(const element_type& element, const char* name) const
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
				put(std::forward<element_type>(element), v);
			}
			template<typename T>
			void put(element_type&& element, const T& v) const
			{
				assign<T>()(element, v);
			}

			template<typename T>
			void get(const element_type& element, T &v) const
			{
				as_to<T>()(element, v);
			}

			void load(const char* input)
			{
				verify(config_read_string(&_config, input));
			}
			void load(const std::string& input)
			{
				verify(config_read_string(&_config, input.data()));
			}
			void load(FILE* input)
			{
				verify(config_read(&_config, input));
			}
			void load_file(const char* filename)
			{
				verify(config_read_file(&_config, filename));
			}

		private:
			config_t _config;

			void verify(int code)
			{
				if (code == CONFIG_FALSE)
				{
					throw error(_config);
				}
			}

			void move_config(config_t* dest, config_t* src)
			{
				memcpy(dest, src, sizeof(config_t));
				move_config(dest, src->root);
				config_init(src);
				config_set_auto_convert(src, CONFIG_TRUE);
			}
			void move_config(config_t* dest, config_setting_t* setting)
			{
				setting->config = dest;
				if (config_setting_is_array(setting) ||
					config_setting_is_list(setting) ||
					config_setting_is_group(setting))
				{
					int length = config_setting_length(setting);
					for (int i = 0; i != length; i++)
					{
						config_setting_t* element = config_setting_get_elem(setting, i);
						element->config = dest;
						move_config(dest, element);
					}
				}
			}
		};

	}

}

#endif //_MODEL_CONFIG_HPP_

