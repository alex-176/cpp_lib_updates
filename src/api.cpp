#include <api_updates/api.hpp>
#include <iostream>
namespace a {

class internal_class {
public:
    int get_value() { return 5; };
};

inline namespace v_0
{
    // case 1: add an implementation of a function with a new argument
    void foo(int arg)
    {
        std::cout << "hello from foo with arg: " << arg << "\n";
    }

    // case 1: change the old function implementation to call a new one or keep the existing implementation as is if necessary
    void foo()
    {
        foo(0);
    }
    // case 4- non-inline part implementation
    void use_some_class(some_class_interface & arg)
    {
        std::cout << "hello from use_some_class. arg.f1(): " << arg.f1() << " arg.f2(): " << arg.f2() << "\n";
    }
}

// case 2: update version namespace
inline namespace v_1
{
    // case 2: change the implementation to use a new struct member
    void init(params const &init_params)
    {
        std::cout << "hello from init:  name: " << init_params.name << " age: " << init_params.age << "\n";
    }
}
}