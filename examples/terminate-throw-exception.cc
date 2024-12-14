#include <y3c/wrap>
#include <stdexcept>

int main(){
    y3c::link();
    throw std::runtime_error("error");
}
