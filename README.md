# leech
Leech can serialize C++ objects to json, yaml, config, etc. The field information of the structure obtained by Leech through reflection at compile time is serialized and output to the document according to the information object, or is deserialized from the document to the object.

## Dependencies

Leech is a library of pure header files, which is very small and does not require compilation. Using leech requires boost.preprocessor and a compiler that supports C ++ 14. In addition, according to the document format, you need to install the corresponding library.

| Document Type | Header | Library |
| ------- | ------ | ------ |
| JSON | leech/json.hpp | [nlohmann/json](https://github.com/nlohmann/json)
| YAML | leech/yaml.hpp | [YAML-CPP](https://github.com/jbeder/yaml-cpp)
| TOML |  leech/toml.hpp | [toml11](https://github.com/ToruNiina/toml11)
| config |  leech/config.hpp | [libconfig](https://github.com/hyperrealm/libconfig)
| info |  leech/info.hpp | [boost.property_tree](https://www.boost.org/doc/libs/release/libs/property_tree/)

Leech is decoupled from the document format. As long as a certain document format is wrapped with an interface required by leech, leech can serialize objects into the document.

## Reflection

Leech's reflection is designed for serializing structured data, so only reflections on data members are supported.
There is now a structure like this:

```C++
struct Base
{
	int id;
	bool enabled;
};
```
After including the header file leech/model.hpp, only one line of code is needed to complete the reflection:
```C++
STRUCT_MODEL(Base, id, enabled)
```
If you have a derived class, you can write:
```C++
struct MyStruct : public Base
{
	int a, b, c;
	std::vector<int> d;
	MyNode node;
};

STRUCT_MODEL_INHERIT(MyStruct, (Base), a, b, c, d, node)
```

Because the reflection of leech is defined outside the structure, it is necessary to ensure that leech can access the internal members of the structure. If necessary, declare as a friend:
```C++
STRUCT_MODEL_FRIEND(MyStruct)
```
If the field may not appear in the document, it needs to be marked as optional:
```C++
STRUCT_MODEL_SET_OPTIONAL(MyStruct, a)
```

## Serialization

After doing reflection, Leech provides put and get operations to complete the serialization of the object:
```C++
namespace leech
{
	template<typename Document, typename T>
	Document& put(Document&& doc, const T& v);
	template<typename Document, typename T>
	Document& put(Document& doc, typename Document::element_type& element, const T& v, const char* name = nullptr);


	template<typename Document, typename T>
	void get(const Document& doc, T& v);
	template<typename Document, typename T>
	void get(const Document& doc, const typename Document::element_type& element, T& v, const char* name = nullptr);
}
```
The usage is simple:
```C++
MyStruct s{ };
leech::json::document ar=leech::json::load_file("test.json");
leech::get(ar, ar["aaa"], s);
```

## Other functions provided by leech
#### Find the fields of a structure:
```C++
template<typename S, typename Pred>
bool find_field(const char* name, Pred&& pred);
```
Pred is a function that will be called when the specified field is found. Its parameters are the corresponding field information.
```C++
template<typename S, typename M>
void Pred(struct_field<S, M>& field_info);
```
The definition of the field information is similar:
```C++
template<typename S, typename M>
struct struct_field
{
public:
	typedef M value_type;

	const char* name() const noexcept;
	value_type& value(S& v) const noexcept;
	const value_type& value(const S& v) const noexcept; bool optional() const noexcept;
	void optional(bool v) noexcept;
	template<typename Document>
	void put(Document& doc, typename Document::element_type& element, const T& v) const;
	template<typename Document>
	void get(const Document& doc, const typename Document::element_type& element, S& v) const;
	template<typename Pred>
	void visit(S& v, Pred&& pred) const;
};

```
#### Modify the value of a field
```C++
template<typename S, typename T>
bool assign(S& s, const char* name, const T& v);
template<typename S, typename T>
bool assign(S& s, size_t index, const T& v);
```
#### Get the number of fields that support reflection
```C++
template<typename S>
constexpr void field_count();
```
#### Iterating through the fields of an object
```C++
template<typename S, typename Pred>
void for_each(S& s, Pred&& pred);
template<typename S, typename Pred>
void for_each(const S& s, Pred&& pred);
```