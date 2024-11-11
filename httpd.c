#include <asm-generic/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum type {
	GET,
	PUT,
	POST,
	HEAD,
	DELETE,

} type;

typedef struct request {
	type t;
	char *value;
} request;

int linecount(char *s) {
	int count = 1;
	for (int i = 0; i < strlen(s); i++)
		if (s[i] == '\n') count++;

	return count;
}

request *calculaterequest(char *line){
	request *req = malloc(sizeof(request));

	char *tok = strtok(line, " ");

	if (strcmp(tok, "GET") == 0){
		req->t = GET;	
		tok = strtok(NULL, " ");
		req->value = strdup(tok);
	}else if (strcmp(tok, "PUT") == 0){
		req->t = PUT;	
	}else if (strcmp(tok, "POST") == 0){
		req->t = POST;	
	}else if (strcmp(tok, "HEAD") == 0){
		req->t = HEAD;	
	}else if (strcmp(tok, "DELETE") == 0){
		req->t = DELETE;	
	}
	return req;
}


char *tostring(int num){
	char *str = malloc((int)((ceil(log10(num))+1)*sizeof(char)));
	sprintf(str, "%d", num);
	return str;
}

void processrequest(request *req, int client){
	char *msg;
	FILE *f;
	switch (req->t){
		case GET:
			if (strcmp(req->value, "/") == 0){
				f = fopen("./index.html", "r");
			} else {
				f = fopen(req->value+1, "r");
			}



			if (f == NULL) {
				msg =  strdup(
						"HTTP/1.1 404 Not Found \n"
						"Content-Length: 22\n"
						"\n"
						"<p>404 not found</p>\n"
					     );
			} else {
				msg = malloc(2048);
				fseek(f, 0, SEEK_END);
				int len = ftell(f);
				char *html = malloc(len);
				rewind(f);
				fread(html, len, 1, f);
				fclose(f);

				char *length = tostring(strlen(html) - 1);

				char response[] = 
					"HTTP/1.1 200 OK\n"
					"Content-Length: ";

				for (int i = 0; i < 2048; i++){
					msg[i] = 0;
				}

				snprintf(msg, 2048, "%s%s\n\n%s\n", response, length, html);
				free(length);
				free(html);
			}

			send(client, msg, strlen(msg), 0);
			free(msg);
			break;
		default:
			printf("not implemented!\n");
	}
}

int main(){
	int s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);
	}


	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(80);

	socklen_t len = sizeof(address);
	if ((bind(s, (struct sockaddr *)&address, len)) != 0) {
		perror("bind");
		exit(1);
	}

	listen(s, 10);

	request **requests = malloc(sizeof(request *) * 128);

	for (;;){
		int client; 
		if ((client = accept(s, (struct sockaddr *)&address, &len)) == -1){
			perror("accept");
			exit(1);
		}

		char *msg = malloc(1024);
		recv(client, msg, 1024, 0);

		char *tmp = malloc(1024);
		int j = 0, linecount = 0;
		for (int i = 0; i < strlen(msg); i++){
			tmp[j] = msg[i];
			if (msg[i] == '\r') {
				i++;
				tmp[j] = 0;
				if (strlen(tmp) != 0) requests[linecount] = calculaterequest(tmp);
				j = -1;
				linecount++;
			}
			j++;
		}
		free(tmp);
		free(msg);

		processrequest(requests[0], client);

		close(client);
	}
	close(s);
	for (int i = 0; i < 128; i++){
		if (requests[i] != NULL){
			free(requests[i]->value);
			free(requests[i]);
		}
	}

	free(requests);

	return 0;
}
