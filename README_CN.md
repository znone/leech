# leech
leech可以序列化C++对象到 json、yaml、config等。leech通过编译期反射得到的结构的字段信息，根据该信息对象序列化输出到文档，或者从文档反向序列化到对象。

## 依赖

leech是纯头文件的库，不需要编译。使用leech需要boost.preprocessor和支持C++14的编译器。另外根据文档格式，还需要安装对应的库。

| 文档格式 | 头文件 | 库 |
| ------- | ------ | ------ |
| json | leech/json.hpp | [nlohmann/json](https://github.com/nlohmann/json)
| yaml | leech/yaml.hpp | [YAML-CPP](https://github.com/jbeder/yaml-cpp)
| config |  leech/config.hpp | [libconfig](https://github.com/hyperrealm/libconfig)

leech和文档格式之间是解耦合的，只要为某种文档格式包装一个leech需要的接口，leech就可以将对象序列化到该文档中。

## 反射

leech的反射是为序列化结构数据设计的，因此只支持对数据成员的反射。
现在有一个这样的结构：

```C++
struct Base
{
	int id;
	bool enabled;
};
```
包含头文件leech/model.hpp后，只需要一行代码可以完成反射:
```C++
STRUCT_MODEL(Base, id, enabled)
```
如果有派生类，可以这样写：
```C++
struct MyStruct : public Base
{
	int a, b, c;
	std::vector<int> d;
	MyNode node;
};

STRUCT_MODEL_INHERIT(MyStruct, (Base), a, b, c, d, node)
```

因为leech的反射是定义在结构之外的，要保证leech能访问到结构内部成员。如果有必要，需要申明为友元：
```C++
STRUCT_MODEL_FRIEND(MyStruct)
```
如果字段可以不出现在文档中，需要标明该字段是可选的：
```C++
STRUCT_MODEL_SET_OPTIONAL(MyStruct, a)
```

## 序列化

做好反射后，leech提供put和get操作完成对对象的序列化：
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
用法很简单：
```C++
MyStruct s{ };
leech::json::document ar=leech::json::load_file("test.json");
leech::get(ar, ar["aaa"], s);
```

## leech提供的其他操作
#### 查找结构的字段：
```C++
template<typename S, typename Pred>
bool find_field(const char* name, Pred&& pred);
```
Pred是一个函数，当找到指定的字段时，会被调用。它的参数是对应的字段信息。
```C++
template<typename S, typename M>
void Pred(struct_field<S, M>& field_info);
```
字段信息的定义类似：
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
#### 修改对象某个字段的值
```C++
template<typename S, typename T>
bool assign(S& s, const char* name, const T& v);
template<typename S, typename T>
bool assign(S& s, size_t index, const T& v);
```
#### 获取支持反射的字段数量
```C++
template<typename S>
constexpr void field_count();
```
#### 遍历对象的字段
```C++
template<typename S, typename Pred>
void for_each(S& s, Pred&& pred);
template<typename S, typename Pred>
void for_each(const S& s, Pred&& pred);
```