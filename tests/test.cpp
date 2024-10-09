#include <iostream>
#include <api_updates/api.hpp>

int main() {
    a::params params;
    params.name = "John";
    a::init(params);
    a::foo();
    std::cout << "bar(): " << a::bar() << "\n";
    return 0;
}
