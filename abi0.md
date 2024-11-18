# Guide for gcc ABI0/ABI1 support

## Motivation
Suppose you’re developing a C++ library that needs to be compatible with both ABI0 (e.g. PyTorch plugin) and ABI1 (regular `C++11` binaries). With the transition to `C++11`, the ABI (Application Binary Interface) was broken for `std::string` and `std::list`. If your library’s API avoids using these types, you’re already compatible across ABIs.

However, adapting an existing API that uses `std::string` requires additional steps (we’ll ignore std::list, as it’s rarely used in APIs). Here’s a guide:

## 1. function argument: use `std::string_view` instead of `std::string const&` and  `std::string`
If your function accepts a `std::string` argument, switch to `std::string_view` to avoid ABI conflicts.
```cpp
// before:
void f(std::string const &);
void f(std::string);
// after:
void f(std::string_view);
```

## 2. Use abi-dependent inline namespace for inline code that uses `std::string`
For inline code, the compiler may choose not to inline functions, potentially leading to different ABI versions in separate modules, violating the One Definition Rule (ODR). This can cause runtime crashes.

To avoid this, use ABI-dependent inline namespaces. Consider the example below:
```cpp
struct S{
    std::string name;
    int age;
};
inline void print(S const& s)
{
    std::cout << "name: " << s.name << " age: " << s.age <<"\n";
}
// client code that can be used in both ABI0/ABI1 clients 
void client_code()
{
  S s{"John", 25};
  print(s);
}
```
In this case, both `print` and `S`(including its constructors and destructor) are inline, but the compiler may not inline them, leading to ABI conflicts. Using an inline namespace tied to the ABI separates ABI0 and ABI1 implementations:

```cpp
#if _GLIBCXX_USE_CXX11_ABI == 0
#  define INLINE_NAMESPACE_NAME inline_code_abi0
#else
#  define INLINE_NAMESPACE_NAME inline_code
#endif
inline namespace INLINE_NAMESPACE_NAME{
struct S{
    std::string name;
    int age;
};
inline void print(S const& s)
{
    std::cout << "name: " << s.name << " age: " << s.age <<"\n";
}
}// namespace

// Client code that can be used in both ABI0/ABI1 clients
void client_code()
{
  S s{"John", 25};// not ambiguous.inline_code_abi0::S or inline_code::S depending on ABI
  print(s); // not ambiguous. inline_code_abi0::print or inline_code::print depending on ABI
}
```
## 3. use a custom `string_wrapper` for functions that return `std::string`
If a function in your API returns a `std::string`, you can’t directly use it in ABI0 clients. Instead, create a custom `string_wrapper` class that provides ownership and returns a `std::string_view` when needed.

Here’s an example of a simple wrapper class:
```cpp
// a simple class that wraps a string ownership and returns string_view on request
class string_wrapper {
public:
    string_wrapper() = default;
    string_wrapper(string_wrapper &&) = default;
    string_wrapper(string_wrapper const &) = default;
    string_wrapper(const char * str)
    {
        _buffer.assign(str, str + strlen(str));
    }
    string_wrapper(std::string_view str)
    {
        _buffer.assign(str.begin(), str.end());
    }
    string_wrapper & operator=(string_wrapper const & ) = default;
    string_wrapper & operator=(string_wrapper && ) = default;
    string_wrapper & operator=(const char * str)
    {
        _buffer.assign(str, str + strlen(str));
        return *this;
    }
    string_wrapper & operator=(std::string_view str)
    {
        _buffer.assign(str.begin(), str.end());
        return *this;
    }
    operator std::string_view() const
    {
        return std::string_view(_buffer.data(), _buffer.size());
    }
    std::string_view str() const
    {
        return std::string_view(_buffer.data(), _buffer.size());
    }

private:
    std::vector<char> _buffer;
};
```
With `string_wrapper`, we can implement ABI-neutral functions as follows:

```cpp
// ABI-neutral implementation
string_wrapper getNameAN();
// ABI-dependent inline API for ABI0 and ABI1 clients
inline namespace INLINE_NAMESPACE_NAME{
std::string getName(){
  return std::string(getNameAN().str());
}
} // namespace
```

## Summary
Designing a C++ API that works with both ABI0 and ABI1 modules isn’t complicated if you follow these guidelines:
1. use `string_view` whenever it's possible
2. use `string_wrapper` class when returning a `std::string`
3. Use ABI-dependent inline namespaces for inline code to prevent ODR violations.

This approach will make your C++ library ABI-compatible with both ABI0 and ABI1 environments.