#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class FlameSensor{
private:
    string path;

public:
    FlameSensor(const std::string& adcPath) {
        path = adcPath;
    }

    int lerValor() {
        std::ifstream file(path);
        int valor;
        file >> valor;
        return valor;
    }

    string Status(){
        if(lerValor() > 60000){
            return "Há chama próxima";
        } else {
            return "Não há chama próxima";
        }
    }

};


int main() {

}

