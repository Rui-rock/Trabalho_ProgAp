#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

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
     * @details É estipulado um valor threshold de 22350 para o código tomar a decisão
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
    /** @return 0 se a leitura for abaixo do thershold e 1 se a leitura for acima do threshold */
    
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

/** @brief Declara o estado do sensor em loop, para o programa se pressionar ENTER */
int main() {
    FlameSensor Sensor("/sys/bus/iio/devices/iio:device0/in_voltage19_raw");

    while (true) 
    {
        cout << "Status: " << Sensor.Status() << endl;

        /**
        * @details Se o usuário pressionar ENTER, o loop para
        */
        if (cin.rdbuf()->in_avail() > 0) {
            char c = cin.get();
            if (c == '\n') break;
        }

        /** 
        * @details Pausa a execução por um intervalo de 500ms para não deixar muito rápido
        */ 
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
