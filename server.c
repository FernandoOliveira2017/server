#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERROR   (-1)
#define LENGTH  4096

#define E_404	"404 Not Found"
#define S_200	"200 OK"

void error(const char * msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void http_response(int client, char * response, size_t size)
{
	if (send(client, response, size, 0) == ERROR)
		error("send");
}

bool http_request(int client, char * request)
{
	int ret;
	
	ret = recv(client, request, LENGTH, 0);
	
	if (ret == ERROR)
		error("recv");
	else
		return ret;
}

void send_page(int client, char * page)
{
    int index;
    struct stat st;
    
    index = open(page, O_RDONLY);
    
    if (index == ERROR)
		http_response(client, E_404, sizeof (E_404));
	else {
		//http_response(client, S_200, sizeof (S_200));
		
		stat(page, &st);
		
		char response[st.st_size];
		
		if (read(index, response, st.st_size) == ERROR)
			error("read");
		
		http_response(client, response, st.st_size);
		
		close(index);
	}
}

void parse_http(int client)
{
    char request[LENGTH];
	size_t size;
	
	while (http_request(client, request)) {
		if (strncmp(request, "GET ", 4) == 0) {
			size = strcspn(request+5, " ");
			
			char name[size];
			
			strncpy(name, request+5, size);
			
			send_page(client, name);
		}
	}
	
	close(client);
}

bool server_loop(int server, FILE * log)
{
    int client;
    struct sockaddr_in addr;
    socklen_t addrlen;
	pthread_t thread;
	
	addrlen = sizeof (addr);
    client = accept(server, (struct sockaddr *)&addr, &addrlen);
    
    if (client == ERROR)
        return true;
    
    fprintf(log, "%s\n", inet_ntoa(addr.sin_addr));
	
	if (pthread_create(&thread, NULL, parse_http, client))
		error("pthread_create");
	
	return true;
}

int main(int argc, char * argv[])
{
    int server;
    FILE * log;
    struct sockaddr_in addr;
    
    bzero(&addr, sizeof(addr));
    
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(argv[1]);
    addr.sin_family      = AF_INET;
    
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (server == ERROR)
        error("socket");
    
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) == ERROR)
        error("bind");
    
    if (listen(server, 1024) == ERROR)
        error("listen");
    
    log = fopen("log.txt", "w+");
    
    if (index == NULL)
        error("fopen");
    
    while (server_loop(server, log));
    
    fclose(log);
    close(server);
    
    return 0;
}
