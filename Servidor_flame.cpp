#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib") // Necessário para linkar a biblioteca de rede

#define SERVER_PORT 5000
#define BUFFER_SIZE 1024
#define LOG_INTERVAL_MS 200
#define RESET_TIME_S 40 

std::atomic<int> tempoDecorrido(0);
std::mutex mtx;
std::vector<std::string> mensagens;

void temporizador() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        tempoDecorrido++;
        if (tempoDecorrido >= RESET_TIME_S) {
            tempoDecorrido = 0;
            // Ao resetar, limpamos as mensagens para reescrever o arquivo
            std::lock_guard<std::mutex> lock(mtx);
            mensagens.clear();
        }
    }
}

void logEmArquivo() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(LOG_INTERVAL_MS));

        std::lock_guard<std::mutex> lock(mtx);
        std::ofstream arquivo("log.txt", std::ios::out | std::ios::trunc);

        if (!arquivo.is_open()) {
            std::cerr << "Erro ao abrir log.txt\n";
            continue;
        }

        // Primeira linha: tempo decorrido atual
        arquivo << "Tempo decorrido: " << tempoDecorrido.load() << "s\n";

        // Linhas seguintes: mensagens recebidas
        for (const auto& msg : mensagens) {
            arquivo << tempoDecorrido.load() << "s → " << msg << "\n";
        }

        arquivo.close();
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sockfd;
    sockaddr_in servAddr{}, cliAddr{};
    int cliLen = sizeof(cliAddr);
    char buffer[BUFFER_SIZE];

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar WinSock.\n";
        return 1;
    }

    // Cria o socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket UDP: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Configura o endereço do servidor
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;  // Recebe de qualquer IP
    servAddr.sin_port = htons(SERVER_PORT);

    // Faz o bind (associa o socket à porta)
    if (bind(sockfd, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        std::cerr << "Erro ao associar socket à porta: " << WSAGetLastError() << "\n";
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor UDP ativo na porta " << SERVER_PORT << " e aguardando mensagens...\n";

    std::thread timerThread(temporizador);

    std::thread logThread(logEmArquivo);

    // Loop para receber mensagens
while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        int recebido = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                (SOCKADDR*)&cliAddr, &cliLen);
        if (recebido == SOCKET_ERROR) {
            std::cerr << "Erro ao receber dados: " << WSAGetLastError() << "\n";
            continue;
        }

        buffer[recebido] = '\0';

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cliAddr.sin_addr, clientIP, sizeof(clientIP));

        std::string mensagemRecebida = std::string(buffer);

        
        // Armazena mensagem em vetor protegido por mutex
        {
            std::lock_guard<std::mutex> lock(mtx);
            mensagens.push_back(std::string(clientIP) + ":" + std::to_string(ntohs(cliAddr.sin_port)) + " → " + mensagemRecebida);
        }

        // Imprime no console
        std::cout << "Tempo decorrido: " << tempoDecorrido.load() << "s | "
                  << clientIP << ":" << ntohs(cliAddr.sin_port)
                  << " → " << mensagemRecebida << std::endl;
    }

    timerThread.join();
    logThread.join();
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
