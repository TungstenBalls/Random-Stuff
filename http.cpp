#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>      // for memset
#include <sys/socket.h> // for sockets
#include <arpa/inet.h>  // for Af_INET 
#include <netdb.h>      // for addrinfo and getaddrinfo

/*
    so basically, we are trying to send 
    api.ipfy.org an http request, that 
    sends us our ip address back, all in
    the form of an HTTP request

    This tiny project serves as a simple 
    practice about HTTP requests and responses
    and me getting familiar with them

*/

int main()
{   
    // name of the host
    const char* host = "api.ipfy.org";

    //the http request that we want to send:
    const char* request = 
    "GET / HTTP/1.1\r\n"
    "Host: api.ipfy.org\r\n"
    "Accept: */*\r\n"
    "Connection: close\r\n\r\n";
    
    struct addrinfo hints, *res;
    res = nullptr;

    // set all the fields of hints to zero
    memset(&hints, 0,sizeof(hints));

    hints.ai_family = AF_INET; // IPv4 
    hints.ai_socktype = SOCK_STREAM; // TCP 

    /*
        getaddrinfo makes a linked list of addrinfo structures
        it takes the requirements from the hints struct, and then makes a linkedlist
        for matching structures.
        res points to the head of the linkedlist;
    */

    int stat = getaddrinfo(host, "80", &hints, &res); 
    
    if(res == nullptr && stat !=0){
        perror("problem with allocation of the linked list on res!\n");
        return EXIT_FAILURE;
    }

    // now comes the part where we iterate over the linked list and try to connect
    // till we find the connection that worked, then we send the HTTP req and wait to
    // receive a response from the server

    addrinfo *temp = res;
    int socket_fd = -1;

    while(temp!= nullptr)
    {
        socket_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol); // tcp or IPPROTO_TCP is defined as 0
    
        if(socket_fd <0){
            perror("Could not open socket!\n");
            temp = temp->ai_next;
            continue;
        }

        if(connect(socket_fd, temp->ai_addr,temp->ai_addrlen) == -1){
            close(socket_fd);
            socket_fd = -1;
            temp = temp->ai_next;
            continue;
        }
        break; // if we find the connection that works
        
    }
    if(socket_fd == -1){
        perror("We are toast, could not open a connection!\n");
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    // now let's get to the main part of this exercies:

    char* buffer = (char*)malloc(4096);

    if(buffer == nullptr) {

        perror("could not allocate buffer!\n");
        close(socket_fd);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    //send the request to the server
    ssize_t bytes_sent = send(socket_fd, request, strlen(request), 0);

    if(bytes_sent == -1){

        perror("Could not send bytes to the server!\n");
        close(socket_fd);
        free(buffer);
        freeaddrinfo(res);
        return EXIT_FAILURE;
        
    }

    ssize_t bytes_received = -1;

    while((bytes_received = recv(socket_fd, buffer, 4095, 0)) > 0){
        buffer[bytes_received] = '\0';
    }

    for(ssize_t i = 0; i < 4095; i++){
        switch(buffer[i]){
            case '\n' : std::cout << '\\n'; break;
            case '\t' : std::cout << '\\t'; break;
            case '\r' : std::cout << '\\r'; break;
            case '\0' : std::cout << '\\0'; break;
            case '\\' : std::cout << '\\\\'; break;
            case '\"' : std::cout << '\\\"'; break;
            default:

                if (buffer[i] < 32 || buffer[i] > 126) {

                    std::cout << "\\x" << (int)(unsigned char)buffer[i]<<'bchar ';
                } else {
                    std::cout << buffer[i];
                }
                break;
        }
    }

    std::cout << '\n';
    if(bytes_received == -1){
        
        perror("recv failed!\n");
        close(socket_fd);
        free(buffer);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    // free memory after usage
    close(socket_fd);
    free(buffer);
    freeaddrinfo(res);

    return 0;

}