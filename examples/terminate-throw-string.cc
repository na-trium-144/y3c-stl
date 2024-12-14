#include <y3c/wrap>
#include <string>

int main(){
    y3c::link();
    throw std::string("error");
}
