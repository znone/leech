#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#pragma once

#include <vector>
#include <list>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <string.h>
#include <boost/preprocessor.hpp>

namespace leech
{

namespace detail 
{
	template<typename T>
	class struct_info;

	template<typename T, typename M>
	struct struct_field
	{
	public:
		typedef M value_type;
		template<size_t N>
		struct_field(const char(&name)[N], M T::*field) noexcept
			: _name(name), _field(field), _optional(false) { }
		const char* name() const noexcept { return _name; }
		value_type& value(T& v) const noexcept { return v.*_field; }
		const value_type& value(const T& v) const noexcept { return v.*_field; }
		bool optional() const noexcept { return _optional;  }
		void optional(bool v) noexcept { _optional = v; }
		template<typename Document>
		void put(Document& doc, typename Document::element_type& element, const T& v) const;
		template<typename Document>
		void get(const Document& doc, const typename Document::element_type& element, T& v) const;
		template<typename Pred>
		void visit(T& v, Pred&& pred) const;

	private:
		const char* _name;
		M T::* _field;
		bool _optional;
	};

	template<typename>
	struct field_type;

	template<typename T, typename M>
	struct field_type<M T::*>
	{
		typedef M type;
	};

	template<typename From, typename To>
	typename std::enable_if < !std::is_convertible<From, To>::value>::type assign_helper(To& to, From& from) noexcept
	{
	}
	template<typename From, typename To>
	typename std::enable_if < std::is_convertible<From, To>::value>::type assign_helper(To& to, From& from) noexcept
	{
		to = from;
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
inline void visit(const char* name, std::list<T>& v, Pred&& pred)
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
inline void visit(const char* name, std::unordered_map<K, T>& v, Pred&& pred)
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
		detail::assign_helper(field.value(s), v);
	});
}

template<typename S, typename T>
inline bool assign(S& s, size_t index, const T& v)
{
	return find_field<S>(index, [&s, &v](const auto& field) mutable {
		detail::assign_helper(field.value(s), v);
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
inline typename std::enable_if<is_reflected<T>::value>::type visit(const char* name, T& v, Pred&& pred) {
	pred(name, v);
	detail::struct_info<T>::instance().for_each(v, std::forward<Pred>(pred));
}
template<typename T, typename Pred>
inline typename std::enable_if<is_reflected<T>::value>::type visit(const char* name, const T& v, const Pred&& pred) {
	pred(name, v);
	detail::struct_info<T>::instance().for_each(v, std::forward<Pred>(pred));
}

namespace detail
{

template<typename T, typename M> template<typename Document>
inline void struct_field<T, M>::put(Document& doc, typename Document::element_type& element, const T& v) const
{
	leech::put(doc, doc.child(element, name()), value(v), name());
}

template<typename T, typename M> template<typename Document>
inline void struct_field<T, M>::get(const Document& doc, const typename Document::element_type& element, T& v) const
{
	try
	{
		leech::get(doc, doc.child(element, name()), value(v), name());
	}
	catch (std::exception&)
	{
		if (!optional()) throw;
	}
}

template<typename T, typename M> template<typename Pred>
inline void struct_field<T, M>::visit(T& v, Pred&& pred) const
{
	leech::visit(name(), value(v), std::forward<Pred>(pred));
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

#define STRUCT_MODEL_FIELD_NAME(field) BOOST_PP_STRINGIZE(field)

#define STRUCT_MODEL_CLASSNAME(fields) BOOST_PP_TUPLE_ELEM(0, fields)

#define STRUCT_MODEL_FIELD_FIX(field) \
	BOOST_PP_CAT(BOOST_PP_CAT(STRUCT_MODEL_FIELD_PREFIX, field), STRUCT_MODEL_FIELD_SUFFIX)

#define STRUCT_MODEL_FIELD(i, fields) \
	&STRUCT_MODEL_CLASSNAME(fields)::STRUCT_MODEL_FIELD_FIX(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(i, 1), fields)) 

#define STRUCT_MODEL_FIELDVAR(i, fields) \
	BOOST_PP_CAT(_, BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(i, 1), fields))

#define STRUCT_MODEL_INIT_FIELD(z, i, fields) \
	struct_field<STRUCT_MODEL_CLASSNAME(fields), typename field_type<decltype(STRUCT_MODEL_FIELD(i, fields))>::type> STRUCT_MODEL_FIELDVAR(i, fields) \
	{ STRUCT_MODEL_FIELD_NAME(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(i, 1), fields)),  STRUCT_MODEL_FIELD(i, fields) } ;

#define STRUCT_MODEL_PUT_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR(i, fields).put(doc, element, v);

#define STRUCT_MODEL_GET_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR(i, fields).get(doc, element, v);

#define STRUCT_MODEL_VISIT_FIELD(z, i, fields) \
	STRUCT_MODEL_FIELDVAR(i, fields).visit(v, std::forward<Pred>(pred));

#define STRUCT_MODEL_FIND_FIELD(z, i, fields) \
	if (strcmp(STRUCT_MODEL_FIELDVAR(i, fields).name(), name)==0) {\
		pred(STRUCT_MODEL_FIELDVAR(i, fields)); \
		return true; \
	}

#define STRUCT_MODEL_FIELD_INDEX(z, i, fields) \
	if(i==index) {\
		pred(STRUCT_MODEL_FIELDVAR(i, fields)); \
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
			void for_each(S& v, Pred&& pred) const { \
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

