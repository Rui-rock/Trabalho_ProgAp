#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib") // Link da biblioteca de rede

#define SERVER_PORT 5000
#define BUFFER_SIZE 1024
#define RESET_TIME_S 40 

std::atomic<int> tempoDecorrido(0);
std::mutex mtx;

// Função para atualizar apenas a linha correspondente ao segundo atual
void atualizarLinhaNoArquivo(int segundo, const std::string& conteudo) {
    std::lock_guard<std::mutex> lock(mtx);

    // Carrega o arquivo inteiro
    std::ifstream in("log.txt");
    std::vector<std::string> linhas;
    std::string linha;

    while (std::getline(in, linha)) {
        linhas.push_back(linha);
    }
    in.close();

    // Proteção caso o arquivo esteja corrompido
    if (linhas.size() < RESET_TIME_S + 1) {
        linhas.clear();
        linhas.push_back("Tempo decorrido: 0s");
        for (int i = 0; i < RESET_TIME_S; i++)
            linhas.push_back(std::to_string(i) + "s → -");
    }

    // Atualiza cabeçalho
    linhas[0] = "Tempo decorrido: " + std::to_string(segundo) + "s";

    // Atualiza APENAS a linha do segundo atual
    linhas[segundo + 1] = std::to_string(segundo) + "s → " + conteudo;

    // Escreve tudo de volta
    std::ofstream out("log.txt", std::ios::out | std::ios::trunc);
    for (auto& l : linhas)
        out << l << "\n";
    out.close();
}

// Thread para atualizar o tempo e linhas com traço, mesmo sem mensagens
void atualizaTempoSozinho() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int segundo = ++tempoDecorrido;
        if (segundo >= RESET_TIME_S) {
            tempoDecorrido = 0;
            segundo = 0;
        }
        atualizarLinhaNoArquivo(segundo, "-");
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sockfd;
    sockaddr_in servAddr{}, cliAddr{};
    int cliLen = sizeof(cliAddr);
    char buffer[BUFFER_SIZE];

    // Inicializa WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar WinSock.\n";
        return 1;
    }

    // Cria socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket UDP: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Configura endereço do servidor
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;  // Recebe de qualquer IP
    servAddr.sin_port = htons(SERVER_PORT);

    // Faz o bind
    if (bind(sockfd, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        std::cerr << "Erro ao associar socket à porta: " << WSAGetLastError() << "\n";
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor UDP ativo na porta " << SERVER_PORT << "...\n";

    // Inicializa log.txt fixo com traços
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::ofstream arquivo("log.txt", std::ios::out | std::ios::trunc);
        arquivo << "Tempo decorrido: 0s\n";
        for (int i = 0; i < RESET_TIME_S; i++)
            arquivo << i << "s → -\n";
        arquivo.close();
    }

    // Inicia thread que atualiza o tempo mesmo sem mensagens
    std::thread tempoThread(atualizaTempoSozinho);

    // Loop principal para receber mensagens UDP
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        int recebido = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                (SOCKADDR*)&cliAddr, &cliLen);
        if (recebido == SOCKET_ERROR) {
            std::cerr << "Erro ao receber dados: " << WSAGetLastError() << "\n";
            continue;
        }

        buffer[recebido] = '\0';

        // Recebe corretamente UTF-8
        std::string conteudo(buffer, recebido);

        int segundo = tempoDecorrido.load();

        // Atualiza log.txt na linha correta
        atualizarLinhaNoArquivo(segundo, conteudo);

        std::cout << "Tempo decorrido: " << segundo << "s | " << conteudo << "\n";
    }

    tempoThread.join();
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
