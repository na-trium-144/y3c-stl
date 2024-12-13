#include "y3c/internal.h"
#include <string>

int main(){
    y3c::link();
    throw std::string("error");
}
