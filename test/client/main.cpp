#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error al crear el socket." << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Convertir la dirección IP de texto a binario
    if (inet_pton(AF_INET, "192.168.100.186", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección IP inválida o no soportada." << std::endl;
        return -1;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Conexión fallida." << std::endl;
        return -1;
    }

    // Enviar mensaje al servidor
    std::string message = "Hola, servidor!";
    send(sock, message.c_str(), message.length(), 0);
    std::cout << "Mensaje enviado: " << message << std::endl;

    // Leer la respuesta del servidor
    int valread = read(sock, buffer, 1024);
    std::cout << "Respuesta del servidor: " << buffer << std::endl;

    // Cerrar el socket
    close(sock);
    
    return 0;
}

