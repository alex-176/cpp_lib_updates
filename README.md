[![C++ microservices](cpp_microservices.webp 'C++ microservices')]()
# Guide to C++ API changes in microservice architecture

Table of contents:

[Motivation](#motivation)  
[Microservices - short intro](#microservices---short-intro)  
[Goals](#goals)  
[Why no C APi?](#why-not-c-api)  
[Names visibility](#names-visibility)  
[Cases of possible changes and ways to add them:](#cases-of-possible-changes-and-ways-to-add-them)  
[1. a new function argument](#1-a-new-function-argument)  
[2. a new struct member](#2-a-new-struct-member-and-a-function-that-takes-it-as-an-argument)  
[3. changes in inline parts](#3-changes-in-inline-parts)  
[4. inline classes and non-inline functions that use them](#4-inline-classes-and-non-inline-functions-that-use-them)  
[6. enumerations](#6-enumerations)  
[7. breaking changes](#7-breaking-changes)  

## Motivation
Assume you have a C++ library with a simple API:
```cpp
struct S{
  int a;
}; 
void f(S const & s);
```
One day, you change the structure `S` by adding another member. Tests of the library work but tests of other previously built binaries that use `f` start crashing. Why? You just broke the binary interface of your library and all the modules that use the library must be recompiled. But what if you have to provide backward binary compatibility? In this case, this article can help.

## Microservices - short intro
Let's review a simple microservices CICD flow for 2 modules (A and B). Assume ModuleA is independent and ModuleB depends on ModuleA. We want our CI to be fast, so we run only tests of the corresponding module in CI. CD runs tests of dependent modules. Assume all the modules use C++.      

[![CICD flow example for microservices](microservices_cicd.png 'CICD flow example for microservices')]()


ModuleA source code file structure:
```
├── include
│   └── A
│        └──api.hpp
├── src
│   └── api.cpp
├── tests
│   └── test.cpp
└── ...
```

include/A/api.hpp:
```cpp
namespace a{
struct params{
    std::string name;
};
void init(params const & init_params);
void foo();
inline int bar() { return 10; }
}
src/api.cpp

#include "A/api.hpp"
#include <iostream>
namespace a{
void init(params const & init_params)
{
    std::cout << "hello from init: " << init_params.name << "\n";
}
void foo()
{
    std::cout << "hello from foo\n" ;
}
}
```

ModuleA artifact file structure (for linux):
```
├── include
│   └── A
│        └──api.hpp
├── lib
│   └── lib/libA.so
├── tests
│   └── testA
└── ...
```
ModuleB source code file structure:

src/main.cpp

tests/testB.cpp:
```cpp
#include <A/api.hpp>
int main()
{
    a::Params params;
    a::init(params);
    a::foo();
    auto v = a::bar();
    return  v / 2;
}
```

## Goals:
1. Backward compatibility must be both at API and ABI level.  
*if we naively change `void foo();` to `void foo(int arg = 0);` we break ABI compatibility only. CI tests will pass and CD tests will fail to start because ModuleB expects `f()` in libA.so.* 

2. API header should be clean and contain only one name for a function (preferably without a version suffix)  

### Why not C API?
Often, people avoid C++ API because of an old trauma of C++ name mangling and ABI instability (on Linux it was std::string/std::list change in C++11, and on Windows is was VC++ that was changing name mangling in every version). Since 2015 C++ ABI and name mangling are quite stable. Of course, bugs happen and you can use C API with [hourglass api pattern](https://github.com/JarnoRalli/hourglass-c-api) if you want to be on the safe side. In reality, it causes more support work to save some headaches in case of a bug in your compiler. In my view, the price is quite high for mitigating only one type of possible compiler bugs. But if you insist - use C names with versions (foo->foo_v1->foo_v2) and C++ inline API on top ([hourglass api pattern](https://github.com/JarnoRalli/hourglass-c-api)). 

### Names visibility
In examples for simplicity, we assume all the symbols have default visibility (e.i. symbol names are saved into a binary and available at runtime for name resolution).  In reality, only public  API names should be visible and marked accordingly. see [Introduction to symbol visibility](https://developer.ibm.com/articles/au-aix-symbol-visibility/).


## Cases of possible changes and ways to add them:
### 1. a new function argument
Change the function signature by adding a new argument with a default value and define an old function that calls a new one:

include/A/api.hpp:
```cpp
namespace a{
    // ....
    void foo(int arg = 0); 
}
```
src/api.cpp:
```cpp
#include <A/api.hpp>
#include <iostream>
namespace a{
void foo(int arg)
{
    std::cout << "foo() arg: " << arg << "\n"; 
}
// add old function implementation that calls a new one or keep an old implementation if necessary
void foo()
{
    foo(0);
}
}
```

Now ModuleA exports both foo() and foo(int), but only foo(int) is exposed in the header. So after the next recompilation of ModuleB, it will start using foo(int).


### 2. a new struct member and a function that takes it as an argument 
Inline namespaces come to rescue.

[Inline namespaces](https://en.cppreference.com/w/cpp/language/namespace#Inline_namespaces) exist since c++11. They don't change the way the client code looks but change compiler-generated mangled names inside inline namespace (usually by adding a namespace name to the generated names). [example on godbolt](https://godbolt.org/z/WsGM5TKe5)

The idea is to keep an old implementation inside an old inline namespace in a cpp file and a new implementation inside a new inline namespace. This way, old clients can resolve old names at runtime, and clients will use new names after recompilation.

include/A/api.hpp:
```cpp
namespace a{
// add inline namespace with a version for the changed part:
inline namespace v_1{
struct params{ // this name 
    std::string name;
    int age = 10;
};
void init(params const & _params);
}
// ...
}
src/api.cpp

#include <A/api.hpp>
#include <iostream>
namespace a{
inline namespace v_1{
void init(params const & _params)
{
    std::cout << "init() name: " << _params.name << " age: " << _params.age << "\n";
}
}}
```
add another file that implements translation from the old struct into the new one. This way we provide binary compatibility for old clients.

src/api_compatibility.cpp
```cpp
#include <A/api.hpp>
namespace a{
// provide old definition in the old version namespace
struct params{
    std::string name;
};
// function in old namespace fills up new struct and calls new function
void init(params const & init_params)
{
   v_1::params new_params;
   new_params.name = init_params.name;
   init(new_params);
}
}
```
### 3. changes in inline parts
Inline functions are not always inlined into compiled code. It means if several modules have an implementation of an inline function, only one of them will be used by the loader. For example:

lib1 defines: inline int bar() { return 10; }

lib2 defines: inline int bar() { return 20; }

an app is using both lib1 and lib2. if `bar` was not inlined (it's possible) then loader will resolve both bar functions to point to one of them. it violates [ODR](https://en.cppreference.com/w/cpp/language/definition). To fix it we reside the inline code inside its own inline namespace and in case of any change we update the name of the inline namespace:

  include/A/api.hpp:
```cpp
namespace a{
// ...
// all the inline code is under inline namespace. its name must be changed for each change in any of the inline function 
inline namespace inline_code_v_1{
inline int bar() { return 10; }
// other inline functions, classes etc.
}}
```
### 4. inline classes and non-inline functions that use them
assume we have the following interface:

include/A/api.hpp:
```cpp
namespace a{
class some_class{
   public:
   some_class(int arg1, int arg2) {/*...*/}
   void f1() { /*...*/}
   void f2() { /*...*/}
   void f3() { /*...*/}
   void f4() { /*...*/}
   private:
   int memeber1;
   int memeber2;
};
void use_some_class(some_class & arg);
}
```
we have non-inline use_some_class (case 2) that depends on inline part - some_class ([case 3.](#3-changes-in-inline-parts)). The concept of inline changes (just update its namespace name) does not work here because it causes an update of non-inline use_some_class. A possible approach is to extract functionality that is used by non-inline functions into an interface and put the interface into the namespace of the corresponding non-inline functions

include/A/api.hpp
```cpp
namespace a{
class some_class_interface{
   public:
   virtual void f1() = 0;
   virtual void f2() = 0;
};
void use_some_class(some_class & arg);

inline namespace inline_code_v_1{
class some_class : some_class_interface{
   public:
   some_class(int arg1, int arg2) {/*...*/}
   void f1() { /*...*/}
   void f2() { /*...*/}
   void f3() { /*...*/}
   void f4() { /*...*/}
   private:
   int member1;
   int member2;
};
}}
```
### 5. exposing internal class
Assume your API has a function that creates an internal object and you want to expose this object in API. std::shared_ptr and forward declaration can help. This way allows hiding changes of internal class from users and all users will use the same version of internal class at runtime. If you want to expose your internal class as a class with some functionality - consider adding a wrapper class in addition to free functions API. This class is inline one so it should follow case 3.

include/A/api.hpp
```cpp
#include <memory>
namespace a{
// use forward declaration + shared_ptr
class internal_class;
using internal_class_ptr = std::shared_ptr<internal_class>;

// provide functions that redirect calls to internal_class methods
int get_value(Internal_class_ptr class_ptr);

// If you want to expose the functionality as a class
// provide class that redirect calls to free functions
inline namespace inline_code_v_1{
class exposed_internal_class{
public:
    exposed_internal_class(Internal_class_ptr class_ptr) : class_ptr(class_ptr){}
    int get_value() { get_value(get_value); }
private:
    internal_class_ptr  class_ptr;
};
}}
```
src/api.cpp
```cpp
#include "A/api.hpp"
namespace a{
class internal_class { 
public:
    int get_value() { return 5; }; 
};
int get_value(Internal_class_ptr class_ptr){
    return class_ptr->get_value();
}
}
```
### 6. enumerations
For an enum type not to change its size in case of a change - make sure you use an underlying type (e.g. uint32_t).  The only change that will not break backward compatibility is adding values to the enum. 
```cpp
namespace a{ 
enum class Enum : uint32_t{
   value0, // old value
   value1, // old value
   value2 // a new value 
};
}
```
### 7. breaking changes 
Sometimes there is no other choice and we have to make a breaking change such as a struct member deletion.  There is no C++ solution. The only way I know - CI/CD should support the simultaneous promotion of several repositories.

