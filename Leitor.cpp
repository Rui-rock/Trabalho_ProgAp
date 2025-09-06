#include <fstream>
#include <iostream>
#include <string>

using namespace std;

/**
 * @brief Cria a classe do sensor de chama
 */
class FlameSensor{
private:
    string path;

public:

    /**
     * @brief Método construtor da classe 
     * @param{path} caminho para o diretório onde são armazenados os dados do sensor 
     */
    FlameSensor(const std::string& adcPath) {
        path = adcPath;
    }

    /**
     * @brief Método que retorna o valor indicado pelo sensor de chama
     */
    bool lerSensor() {
        std::ifstream file(path);
        int valor;
        file >> valor;
        if(valor < 22350) {
            return 0;
        } else {
            return 1;
        }
    }
    
    /**
     * @brief Método que conclui se há ou não chama presente, baseado na leitura do sensor
     */
    string Status(){
        if(lerSensor() == 0){
            return "Há chama próxima";
        } else {
            return "Não há chama próxima";
        }
    }
    /** @return string que fala se há ou não chama */
};


int main() {
    
}
