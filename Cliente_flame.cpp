#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>      
#include <sys/socket.h>     
#include <cstring>  

#define SENSOR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage13_raw"
#define IP_servidor  "192.168.42.10"
#define Port_servidor 5000

using namespace std;

/**
 * @brief Cria a classe do sensor de chama
 */
class FlameSensor {
private:
    string path;

public:
    FlameSensor(const std::string& adcPath) {
        path = adcPath;
    }

    string StatusValue() {
        std::ifstream file(path);
        int valor;
        file >> valor;

        std::string S1 = "Valor: ";
        std::string S2 = std::to_string(valor);
        if (valor < 22350) {
            return S1 + S2 + " - Há chama próxima";
        } else {
            return S1 + S2 + " - Não há chama próxima";
        }
    }

    bool lerSensor() {
        std::ifstream file(path);
        int valor;
        file >> valor;
        return (valor >= 22350);
    }

    string Status() {
        if (lerSensor() == 0)
            return "Há chama próxima";
        else
            return "Não há chama próxima";
    }
};

int main() {
    FlameSensor Sensor(SENSOR_PATH);

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

        // Buffer fixo para envio
        char buffer[256];
        // Limpa o conteúdo do buffer (zera todos os bytes)
        memset(buffer, 0, sizeof(buffer));

        // Copia a string para o buffer
        strncpy(buffer, status.c_str(), sizeof(buffer) - 1);

        // Envia o buffer via UDP
        ssize_t enviado = sendto(sock, buffer, strlen(buffer), 0,
                                 (sockaddr*)&servAddr, sizeof(servAddr));
        if (enviado < 0) {
            perror("Erro ao enviar pacote UDP");
            break;
        }

        cout << "Enviado: " << buffer << endl;
        sleep(1);
    }

    close(sock);
    return 0;
}
