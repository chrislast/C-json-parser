/*
 * main.c
 *
 *  Created on: 23 Mar 2016
 *      Author: clast
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "json.h"
#include "json_string.txt"

int socket_connect(char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;

	if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	return sock;
}

#define BUFFER_SIZE 8096
char data[BUFFER_SIZE*10];
char* pdata=&data[0];

#define QUERY "GET /data/2.5/weather?id=2649672&APPID=ae692266a3e7c34a68a3cf001fad799b&units=metric HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n"

int main(int argc, char *argv[]){
	int fd;
	char buffer[BUFFER_SIZE];
	ssize_t total=0;

	if(argc < 3){
//		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
//		exit(1);
		fd = socket_connect("api.openweathermap.org", 80);
	} else

	fd = socket_connect(argv[1], atoi(argv[2]));
	fcntl(fd, F_SETFL, O_NONBLOCK);
	write(fd, QUERY, strlen(QUERY));
	bzero(buffer, BUFFER_SIZE);
	while(1)
	{
		ssize_t size;
		size=read(fd, buffer, BUFFER_SIZE - 1);
		if (size>0)
		{
			fprintf(stderr, "%s\n", buffer);
			strncpy(pdata,buffer,size);
			total+=size;
			pdata+=size;
			*pdata='\0';
			bzero(buffer, BUFFER_SIZE);
		}
		else if (total > 0) break;
	}

	shutdown(fd, SHUT_RDWR);
	close(fd);
	printf("@%s@\n",data);
#ifdef USE_JSON_TEXT
	JsonLoad(json_text1);
	JsonLoad(json_text2);
	JsonLoad(json_text3);
	JsonLoad(json_text4);
	JsonLoad(json_text5);
	JsonLoad(json_text6);
#else
	JsonLoad(data);
#endif
	{
		JsonKey* v1=JsonGet("root:coord:lat");
		JsonKey* v2=JsonGet("root:coord:lon");
		JsonKey* t;
		char key[JSON_LABEL_LENGTH_MAX];
		int i;

		if (v1 && v2)
			printf ("\nlatitude=%.3lf longitude=%.3lf\n\n",v1->value.real,v2->value.real);
		for (i=0;i<10;i++)
		{
			int l;
			l=sprintf(key,"root:weather[%d]:main",i);
			t=JsonGet(key);
			if (t)
				printf ("weather %d = %s\n",i,t->value.string);
		}
	}
	return 0;
}

