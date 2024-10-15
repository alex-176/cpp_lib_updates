#ifndef API_UPDATES_API_HPP
#define API_UPDATES_API_HPP
#include <string>
#include <memory>

namespace a{

// case 5 - use forward declaration + shared_ptr
class internal_class;
using internal_class_sptr = std::shared_ptr<internal_class>;

// case 2: update version namespace
inline namespace v_1 {
    struct params {
        std::string name;
        int         age = 0; // case 2: add a new field
    };

    void init(params const &init_params);
}

inline namespace v_0 {
    void foo(int arg = 0); // case 1: add an argument with a default value (original version has no args)

    // case 4 - non-inline part
    class some_class_interface{
    public:
        virtual int f1() = 0;
        virtual int f2() = 0;
    };
    // non-inline function that uses some_class_interface that exposes f1 and f2 only
    void use_some_class(some_class_interface & arg);

    // case 5 - provide a function that instantiates an object of internal_class
    internal_class_sptr create_internal_class_instance(int value);
    // case 5 - provide functions that redirect calls to internal_class methods
    int get_value(internal_class_sptr const & class_ptr);
}

// inline part inside its own inline namespace
// case 3 - change inline namespace
inline namespace inline_v_1 {
    inline int bar() { return 20; } // case 3 - change any inline part causes the change of inline namespace name

    // inline class that can be freely modified without touching the versioning of use_some_class
    class some_class : public some_class_interface{
    public:
        some_class(int arg1, int arg2) : _m1(arg1), _m2(arg2) {}
        int f1() override { return _m1; }
        int f2() override { return _m2; }
        void f3() { /*...*/}
        void f4() { /*...*/}
    private:
        int _m1;
        int _m2;
    };

    // case 5 - If you want to expose the functionality as a class
    // provide a class that redirects calls to free functions
    class exposed_internal_class{
    public:
        exposed_internal_class(int value) : _impl(a::create_internal_class_instance(value)){}
        int get_value() { return a::get_value(_impl); } // redirect call to a free function
    private:
        internal_class_sptr  _impl;
    };
}

}
#endif //API_UPDATES_API_HPP
