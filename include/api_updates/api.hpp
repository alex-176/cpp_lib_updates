#ifndef API_UPDATES_API_HPP
#define API_UPDATES_API_HPP
#include <string>
#include <memory>

namespace a{

// api before any changes:
inline namespace v_0 {
    struct params {
        std::string name;
    };

    void init(params const &init_params);

    void foo(int arg = 0); // case 1: add an argument with a default value (original version has no args)
}

// inline part inside its own inline namespace
inline namespace inline_v_0 {
    inline int bar() { return 10; }
}

}
#endif //API_UPDATES_API_HPP
