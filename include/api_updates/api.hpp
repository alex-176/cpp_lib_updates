#ifndef API_UPDATES_API_HPP
#define API_UPDATES_API_HPP
#include <string>
#include <memory>

namespace a{

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
}

// inline part inside its own inline namespace
// case 3 - change inline namespace
inline namespace inline_v_1 {
    inline int bar() { return 20; } // case 3 - change any inline part causes the change of inline namespace name
}

}
#endif //API_UPDATES_API_HPP
