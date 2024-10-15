#include <iostream>
#include <api_updates/api.hpp>

int main() {
    a::params params;
    params.name = "John";
    a::init(params);
    a::foo();
    std::cout << "bar(): " << a::bar() << "\n";
    // case 4 - usage
    a::some_class some_class_instance(5, 6);
    a::use_some_class(some_class_instance);

    return 0;
}
