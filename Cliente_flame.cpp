#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>      
#include <sys/socket.h>     
#include <cstring>  

/**
 * @param 
 * Path do arquivo na placa que lê as tensões do sensor
 * IP da interface Ethernet da máquina a ser usada como servidor
 * Porta a ser usada pelo servidor
 */
#define SENSOR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage19_raw"
#define IP_servidor  "192.168.42.10"
#define Port_servidor 5000

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

    string StatusValue(){
        std::ifstream file(path);
        int valor;
        file >> valor;
        std::string S1 = "Valor: ";
        std::string S2 = std::to_string(valor);
        if(valor < 22350) {
            return S1 + S2 + " - Há chama próxima";
        } else {
            return S1 + S2 + " - Não há chama próxima";
        }
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

int main() {
    
    FlameSensor Sensor(SENSOR_PATH);

    /**
     * @brief Cria o socket do cliente (placa STM)
     */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket UDP");
        return 1;
    }

    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(Port_servidor);
    servAddr.sin_addr.s_addr = inet_addr(IP_servidor);

    cout << "Cliente UDP iniciado. Enviando status do sensor para "
         << IP_servidor << ":" << Port_servidor << endl;


    /**
     * @brief Loop de leitura do sensor e envio da leitura para o IP do servidor
     */
    while (true) {
        string status = Sensor.StatusValue();

        ssize_t enviado = sendto(sock, status.c_str(), status.size(), 0,
                                 (sockaddr*)&servAddr, sizeof(servAddr));
        if (enviado < 0) {
            perror("Erro ao enviar pacote UDP");
            break;
        }

        cout << "Enviado: " << status << endl;
        sleep(1);
        /** 
        * @details Pausa a execução do loop por um intervalo de 1s para não deixar muito rápido
        */ 
    }

    close(sock);
    return 0;
}
