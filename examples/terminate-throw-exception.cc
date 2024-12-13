#include "y3c/internal.h"
#include <stdexcept>

int main(){
    y3c::link();
    throw std::runtime_error("error");
}
