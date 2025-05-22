#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define DEFAULT_PORT 8080

void decode_url(char *output, const char *input) {
    char hex1, hex2;
    while (*input) {
        if (*input == '%' && (hex1 = input[1]) && (hex2 = input[2]) && isxdigit(hex1) && isxdigit(hex2)) {
            hex1 = hex1 >= 'a' ? hex1 - 32 : hex1;
            hex1 = hex1 >= 'A' ? hex1 - 55 : hex1 - 48;
            
            hex2 = hex2 >= 'a' ? hex2 - 32 : hex2;
            hex2 = hex2 >= 'A' ? hex2 - 55 : hex2 - 48;
            
            *output++ = (hex1 << 4) | hex2;
            input += 3;
        } else {
            *output++ = *input++;
        }
    }
    *output = 0;
}

void setup_network_service() {
    int main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DEFAULT_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    
    bind(main_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(main_socket, 3);
    printf("Служба запущена на порту %d\n", DEFAULT_PORT);

    while(1) {
        int client_socket = accept(main_socket, NULL, NULL);
        
        char request_data[2048] = {0};
        recv(client_socket, request_data, sizeof(request_data)-1, 0);
        
        char encoded_text[256] = {0};
        char decoded_text[256] = {0};
        char* param_ptr = strstr(request_data, "message=");

        if (param_ptr) {
            sscanf(param_ptr + 8, "%255[^& \n]", encoded_text);
            decode_url(decoded_text, encoded_text);
        }

        char http_response[2048];
        snprintf(http_response, sizeof(http_response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n\r\n"
            "<html><body>"
            "<h1>%s</h1>"
            "<img src='https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg' width='300'>"
            "</body></html>", 
            decoded_text);
            
        send(client_socket, http_response, strlen(http_response), 0);
        close(client_socket);
    }
}

int main() {
    setup_network_service();
    return 0;
}
