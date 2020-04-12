#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#pragma once

#include <vector>
#include <list>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <string.h>
#include <boost/preprocessor.hpp>

namespace leech
{

namespace detail 
{
	template<typename T>
	class struct_info;

	template<typename T, typename M>
	struct setter
	{
		typedef M value_type;
		setter(M T::* m) : _m(m) { }
		setter(const setter&) = default;
		void operator()(T& s, const M& v) const
		{
			s.*_m = v;
		}
		void operator()(T& s, M&& v) const
		{
			s.*_m = std::move(v);
		}

	private:
		M T::* _m;
	};

	template<typename T, typename M>
	struct getter
	{
		typedef M value_type;
		getter(M T::* m) : _m(m) { }
		getter(const getter&) = default;
		M& operator()(T& s) const noexcept
		{
			return s.*_m;
		}
		const M& operator()(const T& s) const noexcept
		{
			return s.*_m;
		}

	private:
		M T::* _m;
	};

	template<typename T, typename Getter, typename Setter>
	struct struct_field
	{
	public:
		typedef std::remove_cv_t<std::remove_reference_t<typename std::result_of<Getter(T&)>::type>> value_type;
		template<size_t N>
		struct_field(const char(&name)[N], const Getter& getter, const Setter& setter) noexcept
			: _name(name), _setter(setter), _getter(getter), _optional(false) { }
		const char* name() const noexcept { return _name; }
		
		template<typename = std::enable_if<!std::is_same<Getter, std::nullptr_t>::value>>
		decltype(auto) get_value(const T& v) const { return _getter(v); }
		template<typename = std::enable_if<!std::is_same<Setter, std::nullptr_t>::value>>
		void set_value(T& v, const value_type& f) { _setter(v, f); }
		bool optional() const noexcept { return _optional;  }
		void optional(bool v) noexcept { _optional = v; }
		template<typename Document>
		void put(Document& doc, typename Document::element_type& element, const T& v) const;
		template<typename Document>
		void get(const Document& doc, const typename Document::element_type& element, T& v) const;
		template<typename Pred>
		void visit(const T& v, Pred&& pred) const;
		template<typename Pred>
		void visit(T&& v, Pred&& pred) const;

	protected:
		const char* _name;
		Setter _setter;
		Getter _getter;
		bool _optional;
	};

	template<typename T, typename M>
	struct struct_data_field : public struct_field<T, getter<T, M>, setter<T, M>>
	{
		template<size_t N>
		struct_data_field(const char(&name)[N], M T::*m) noexcept
			: struct_field<T, getter<T, M>, setter<T, M>>(name, getter<T, M>(m), setter<T, M>(m)) { }
		const M& get_value(const T& v) const noexcept { return this->_getter(v); }
		M& get_value(T& v) const noexcept { return this->_getter(v); }
		void set_value(T& v, const M& f) noexcept { this->_setter(v, f); }
		void set_value(T& v, M&& f) noexcept { this->_setter(v, std::forward<M>(f)); }
	};

	template<typename>
	struct field_type;

	template<typename T, typename M>
	struct field_type<M T::*>
	{
		typedef M type;
	};

	template<typename S, typename Field, typename From>
	typename std::enable_if < !std::is_convertible<From, typename Field::value_type>::value>::type assign_helper(S& s, Field& field, From&& from) noexcept
	{
	}
	template<typename S, typename Field, typename From>
	typename std::enable_if < std::is_convertible<From, typename Field::value_type>::value>::type assign_helper(S& s, Field& field, From&& from) noexcept
	{
		field.set_value(s, std::forward<From>(from));
	}
}

template<typename T>
struct is_reflected : public std::integral_constant<bool, false> { };

namespace detail 
{

template<typename Document, typename T>
inline typename std::enable_if<!is_reflected<T>::value, Document&>::type put(Document& doc, typename Document::element_type& element, const T& v, const char* /*name*/ = nullptr)
{
	doc.put(element, v);
	return doc;
}

template<typename Document, typename T>
inline typename std::enable_if<!is_reflected<T>::value, const Document&>::type get(const Document& doc, const typename Document::element_type& element, T& v, const char* /*name*/ = nullptr)
{
	doc.get(element, v);
	return doc;
}

template<typename Document, typename T>
inline typename std::enable_if<is_reflected<T>::value, Document&>::type put(Document& doc, typename Document::element_type& element, const T& v, const char* name = nullptr)
{
	detail::struct_info<T>::instance().put(doc, element, v); 
	return doc;
}

template<typename Document, typename T>
inline typename std::enable_if<is_reflected<T>::value, const Document&>::type get(const Document& doc, const typename Document::element_type& element, T& v, const char* name = nullptr)
{
	detail::struct_info<T>::instance().get(doc, element, v);
	return doc;
}

}

template<typename Document, typename T>
inline Document& put(Document&& doc, const T& v)
{
	return detail::put(doc, doc.root(), v);
}

template<typename Document, typename T>
inline void get(const Document& doc, T& v)
{
	detail::get(doc, doc.root(), v);
}

template<typename Document, typename T>
inline Document& put(Document& doc, typename Document::element_type& element, const T& v, const char* name = nullptr)
{
	return detail::put(doc, element, v, name);
}

template<typename Document, typename T>
inline void get(const Document& doc, const typename Document::element_type& element, T& v, const char* name = nullptr)
{
	detail::get(doc, element, v, name);
}

template<typename T, typename Pred>
inline typename std::enable_if<!is_reflected<T>::value>::type visit(const char* name, T& v, Pred&& pred)
{
	pred(name, v);
}

template<typename T, typename Pred>
inline void visit(const char* name, std::vector<T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(name, i);
}

template<typename T, typename Pred>
inline void visit(const char* name, const std::vector<T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(name, i);
}

template<typename T, typename Pred>
inline void visit(const char* name, std::list<T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(name, i);
}

template<typename T, typename Pred>
inline void visit(const char* name, const std::list<T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(name, i);
}

template<typename K, typename T, typename Pred>
inline void visit(const char* name, std::map<K, T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(i.first, i.second);
}

template<typename K, typename T, typename Pred>
inline void visit(const char* name, const std::map<K, T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(i.first, i.second);
}

template<typename K, typename T, typename Pred>
inline void visit(const char* name, std::unordered_map<K, T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(i.first, i.second);
}

template<typename K, typename T, typename Pred>
inline void visit(const char* name, const std::unordered_map<K, T>& v, Pred&& pred)
{
	for (auto& i : v)
		pred(i.first, i.second);
}

template<typename S, typename Pred>
inline bool find_field(const char* name, Pred&& pred)
{
	return detail::struct_info<S>::instance().find_field(name, std::forward<Pred>(pred));
}

template<typename S, typename Pred>
inline constexpr bool find_field(size_t index, Pred&& pred)
{
	return detail::struct_info<S>::instance().find_field(index, std::forward<Pred>(pred));
}

template<typename S, typename T>
inline bool assign(S& s, const char* name, const T& v)
{
	return find_field<S>(name, [&s, &v](auto& field) mutable {
		detail::assign_helper(s, field, v);
	});
}

template<typename S, typename T>
inline bool assign(S& s, const char* name, T&& v)
{
	return find_field<S>(name, [&s, &v](auto& field) mutable {
		detail::assign_helper(s, field, std::forward<T>(v));
	});
}

template<typename S, typename T>
inline bool assign(S& s, size_t index, const T& v)
{
	return find_field<S>(index, [&s, &v](const auto& field) mutable {
		detail::assign_helper(s, field, v);
	});
}

template<typename S, typename T>
inline bool assign(S& s, size_t index, T&& v)
{
	return find_field<S>(index, [&s, &v](const auto& field) mutable {
		detail::assign_helper(s, field, std::forward<T>(v));
	});
}

template<typename S>
inline constexpr void field_count()
{
	return detail::struct_info<S>::field_count;
}

template<typename T, typename Pred>
inline void for_each(T& v, Pred&& pred)
{
	detail::struct_info<T>::instance().for_each(v, std::forward<Pred>(pred));
}
template<typename T, typename Pred>
inline void for_each(const T& v, Pred&& pred)
{
	detail::struct_info<T>::instance().for_each(v, std::forward<Pred>(pred));
}
template<typename T, typename Pred>
inline typename std::enable_if<is_reflected<T>::value>::type visit(const char* name, T&& v, Pred&& pred) {
	pred(name, v);
	detail::struct_info<T>::instance().for_each(std::forward<T>(v), std::forward<Pred>(pred));
}
template<typename T, typename Pred>
inline typename std::enable_if<is_reflected<T>::value>::type visit(const char* name, const T& v, Pred&& pred) {
	pred(name, v);
	detail::struct_info<T>::instance().for_each(v, std::forward<Pred>(pred));
}

namespace detail
{

template<typename T, typename Getter, typename Setter> template<typename Document>
inline void struct_field<T, Getter, Setter>::put(Document& doc, typename Document::element_type& element, const T& v) const
{
	leech::put(doc, doc.child(element, name()), get_value(v), name());
}

template<typename T, typename Getter, typename Setter> template<typename Document>
inline void struct_field<T, Getter, Setter>::get(const Document& doc, const typename Document::element_type& element, T& v) const
{
	try
	{
		value_type field_value;
		leech::get(doc, doc.child(element, name()), field_value, name());
		_setter(v, std::move(field_value));
	}
	catch (std::exception&)
	{
		if (!optional()) throw;
	}
}

template<typename T, typename Getter, typename Setter> template<typename Pred>
inline void struct_field<T, Getter, Setter>::visit(const T& v, Pred&& pred) const
{
	leech::visit(name(), get_value(v), std::forward<Pred>(pred));
}

template<typename T, typename Getter, typename Setter> template<typename Pred>
inline void struct_field<T, Getter, Setter>::visit(T&& v, Pred&& pred) const
{
	leech::visit(name(), get_value(std::forward<T>(v)), std::forward<Pred>(pred));
}

}

}

// Usage: STRUCT_MODEL(MyStruct, a, b, c)

#ifndef STRUCT_MODEL_FIELD_PREFIX
#define STRUCT_MODEL_FIELD_PREFIX
#endif //STRUCT_MODEL_FIELD_PREFIX

#ifndef STRUCT_MODEL_FIELD_SUFFIX
#define STRUCT_MODEL_FIELD_SUFFIX
#endif //STRUCT_MODEL_FIELD_SUFFIX

#define STRUCT_MODEL_UNBOX(var) \
	BOOST_PP_IIF(BOOST_PP_IS_BEGIN_PARENS(var), BOOST_PP_TUPLE_ELEM(0, var), var)

#define STRUCT_MODEL_FIELD_NAME(field)  \
	BOOST_PP_STRINGIZE(STRUCT_MODEL_UNBOX(field))

#define STRUCT_MODEL_CLASSNAME(fields) BOOST_PP_TUPLE_ELEM(0, fields)

#define STRUCT_MODEL_FIELD_FIX(field) \
	BOOST_PP_CAT(BOOST_PP_CAT(STRUCT_MODEL_FIELD_PREFIX, field), STRUCT_MODEL_FIELD_SUFFIX)

#define STRUCT_MODEL_ELEMENT(i, fields) \
	BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(i, 1), fields)

#define STRUCT_MODEL_FIELD(classname, field) \
	&classname::STRUCT_MODEL_FIELD_FIX(STRUCT_MODEL_UNBOX(field)) 

#define STRUCT_MODEL_GETFUN(classname, index, fields) \
	BOOST_PP_IF(index, std::mem_fn(&classname::BOOST_PP_TUPLE_ELEM(index, fields)), nullptr)

#define STRUCT_MODEL_GETTER(classname, fields) \
	STRUCT_MODEL_GETFUN(classname, BOOST_PP_IIF(BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(fields), 1), 1, 0), fields)
#define STRUCT_MODEL_SETTER(classname, fields) \
	STRUCT_MODEL_GETFUN(classname, BOOST_PP_IIF(BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(fields), 2), 2, 0), fields)

#define STRUCT_MODEL_FIELDVAR(field) \
	BOOST_PP_CAT(_, STRUCT_MODEL_UNBOX(field))

#define STRUCT_MODEL_FIELDVAR_EX(i, fields) \
	STRUCT_MODEL_FIELDVAR(STRUCT_MODEL_ELEMENT(i, fields))

#define STRUCT_MODEL_INIT_DATA_FIELD(classname, field) \
	struct_data_field<classname, typename field_type<decltype(STRUCT_MODEL_FIELD(classname, field))>::type> STRUCT_MODEL_FIELDVAR(field) \
	{ STRUCT_MODEL_FIELD_NAME(field), STRUCT_MODEL_FIELD(classname, field) };

#define STRUCT_MODEL_INIT_SETTER_GETTER(classname, fields) \
		struct_field<classname, decltype(STRUCT_MODEL_GETTER(classname, fields)), decltype(STRUCT_MODEL_SETTER(classname, fields))> STRUCT_MODEL_FIELDVAR(fields) \
		{ STRUCT_MODEL_FIELD_NAME(fields), STRUCT_MODEL_GETTER(classname, fields), STRUCT_MODEL_SETTER(classname, fields) };

#define STRUCT_MODEL_INIT_FIELD_IMPL(classname, fields) \
	BOOST_PP_TUPLE_ENUM(BOOST_PP_IIF(BOOST_PP_IS_BEGIN_PARENS(fields), \
		(STRUCT_MODEL_INIT_SETTER_GETTER(classname, fields)), \
		(STRUCT_MODEL_INIT_DATA_FIELD(classname, fields))))

#define STRUCT_MODEL_INIT_FIELD(z, i, fields) \
	STRUCT_MODEL_INIT_FIELD_IMPL(STRUCT_MODEL_CLASSNAME(fields), STRUCT_MODEL_ELEMENT(i, fields))

#define STRUCT_MODEL_PUT_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR_EX(i, fields).put(doc, element, v);

#define STRUCT_MODEL_GET_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR_EX(i, fields).get(doc, element, v);

#define STRUCT_MODEL_VISIT_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR_EX(i, fields).visit(v, std::forward<Pred>(pred));

#define STRUCT_MODEL_FIND_FIELD(z, i, fields) \
	if (strcmp(STRUCT_MODEL_FIELDVAR_EX(i, fields).name(), name)==0) {\
		pred(STRUCT_MODEL_FIELDVAR_EX(i, fields)); \
		return true; \
	}

#define STRUCT_MODEL_FIELD_INDEX(z, i, fields) \
	if(i==index) {\
		pred(STRUCT_MODEL_FIELDVAR_EX(i, fields)); \
		return true; \
	}

#define STRUCT_MODEL_FUNCTIONS(S) \
	template<> struct is_reflected<S> : std::integral_constant<bool, true> { };

#define STRUCT_MODEL(S, ...)  \
	namespace leech { \
		namespace detail { \
		template<> class struct_info<S> { public: \
			struct_info() = default; \
			enum { field_count = BOOST_PP_TUPLE_SIZE((__VA_ARGS__)) };\
			template<typename Document> \
			void put(Document& doc, typename Document::element_type& element,  const S& v) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_PUT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Document> \
			void get(Document& doc, const typename Document::element_type& element,  S& v) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_GET_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			void for_each(S&& v, Pred&& pred) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_VISIT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			void for_each(const S& v, Pred&& pred) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_VISIT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			bool find_field(const char* name, Pred&& pred) { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_FIND_FIELD, (S, __VA_ARGS__)) \
				return false; \
			} \
			template<typename Pred> \
			constexpr bool find_field(size_t index, Pred&& pred) { \
				if(index>=field_count) return false; \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_FIELD_INDEX, (S, __VA_ARGS__)) \
				return false; \
			} \
			static struct_info<S>& instance() noexcept { \
				static struct_info<S> object; return object; \
			} \
			private: \
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_INIT_FIELD, (S, __VA_ARGS__)) \
		}; \
	} \
	STRUCT_MODEL_FUNCTIONS(S) \
}

#define STRUCT_INFO_BASE_CLASS(i, bases) \
	struct_info<BOOST_PP_TUPLE_ELEM(i, bases)>

#define STRUCT_INFO_INHERIT_BASE_CLASS(z, i, bases) \
	public STRUCT_INFO_BASE_CLASS(i, bases)

#define STRUCT_MODEL_INVOKE_PUT(z, i, bases) \
	STRUCT_INFO_BASE_CLASS(i, bases)::put(doc, element, v);

#define STRUCT_MODEL_INVOKE_GET(z, i, bases) \
	STRUCT_INFO_BASE_CLASS(i, bases)::get(doc, element, v);

#define STRUCT_MODEL_INVOKE_FOR_EACH(z, i, bases) \
	STRUCT_INFO_BASE_CLASS(i, bases)::for_each(v, std::forward<Pred>(pred));

#define STRUCT_MODEL_INVOKE_VISIT(z, i, bases) \
	if(STRUCT_INFO_BASE_CLASS(i, bases)::find_field(name, std::forward<Pred>(pred))) return true;

#define STRUCT_MODEL_FIND_INDEX(z, i, bases) \
	if(index>=STRUCT_INFO_BASE_CLASS(i, bases)::field_count) \
		index-=STRUCT_INFO_BASE_CLASS(i, bases)::field_count; \
	else \
		return STRUCT_INFO_BASE_CLASS(i, bases)::find_field(index, std::forward<Pred>(pred));

#define STRUCT_MODEL_INHERIT(S, bases, ...)  \
	namespace leech { \
		namespace detail { \
		template<> class struct_info<S> : \
			BOOST_PP_ENUM(BOOST_PP_TUPLE_SIZE(bases), STRUCT_INFO_INHERIT_BASE_CLASS, bases) \
		{ public: \
			struct_info() = default; \
			enum { field_count = BOOST_PP_TUPLE_SIZE((__VA_ARGS__)) };\
			template<typename Document> \
			void put(Document& doc, typename Document::element_type& element,  const S& v) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_INVOKE_PUT, bases) \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_PUT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Document> \
			void get(Document& doc, const typename Document::element_type& element,  S& v) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_INVOKE_GET, bases) \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_GET_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			void for_each(S& v, Pred&& pred) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_INVOKE_FOR_EACH, bases) \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_VISIT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			void for_each(const S& v, Pred&& pred) const { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_INVOKE_FOR_EACH, bases) \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_VISIT_FIELD, (S, __VA_ARGS__)) \
			} \
			template<typename Pred> \
			bool find_field(const char* name, Pred&& pred) { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_INVOKE_VISIT, bases) \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_FIND_FIELD, (S, __VA_ARGS__)) \
				return false; \
			} \
			template<typename Pred> \
			constexpr bool find_field(size_t index, Pred&& pred) { \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(bases), STRUCT_MODEL_FIND_INDEX, bases) \
				if(index>=field_count) return false; \
				BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_FIELD_INDEX, (S, __VA_ARGS__)) \
				return false; \
			} \
			static struct_info<S>& instance() noexcept { \
				static struct_info<S> object; return object; \
			} \
			private: \
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), STRUCT_MODEL_INIT_FIELD, (S, __VA_ARGS__)) \
		}; \
	} \
	STRUCT_MODEL_FUNCTIONS(S) \
}

#define STRUCT_MODEL_SET_OPTIONAL_VALUE(S, field, value) \
	leech::detail::struct_info<S>::instance().find_field(#field, [](auto& field_info) { field_info.optional(value); })

#define STRUCT_MODEL_SET_OPTIONAL(S, field) \
	STRUCT_MODEL_SET_OPTIONAL_VALUE(S, field, true);

#define STRUCT_MODEL_SET_REQUIRED(S, field) \
	STRUCT_MODEL_SET_OPTIONAL_VALUE(S, field, false);

#define STRUCT_MODEL_FRIEND(S)  \
	friend class leech::detail::struct_info<S>;


#endif //_MODEL_HPP_

