#include <api_updates/api.hpp>
#include <iostream>
namespace a {

class internal_class {
public:
    int get_value() { return 5; };
};

inline namespace v_0
{
    void foo()
    {
        std::cout << "hello from foo\n";
    }

    void init(params const &init_params)
    {
        std::cout << "hello from init:  name: " << init_params.name << "\n";
    }
}
}