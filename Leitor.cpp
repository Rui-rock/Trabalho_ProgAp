#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class FlameSensor{

    int ReadGPIO(){

    };

    string Status(int Sinal){
        if(Sinal == 1){
            return "Há chama próxima";
        } else {
            return "Não há chama próxima";
        }
    }

};


int main() {

}

