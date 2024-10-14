#include <api_updates/api.hpp>
namespace a{

inline namespace v_0{
    // case 2: provide the old definition within the old version namespace
    struct params{
        std::string name;
    };
    // case 2: function in the old namespace fills up a new struct and calls a new function
    void init(params const & init_params)
    {
        v_1::params new_params;
        new_params.name = init_params.name;
        // don't set a new field if the default is good enough

        // call a new function
        init(new_params);
    }
}}