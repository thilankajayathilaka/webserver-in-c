#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
char *getFileType(char uri[]);
void Loadfile(char uri[], char *buffer, int newsockfd, char *type);

int main()
{
  char *buffer = malloc(BUFFER_SIZE);

  // create  a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (sockfd == -1)
  {
    perror("webserver (socket)");
    return 1;
  }
  printf("socket created!\n");

  // create the address to bind the socket to
  struct sockaddr_in host_addr;
  int host_addrlen = sizeof(host_addr);

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(PORT);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // create client address
  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  // bind the socket to the address
  if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
  {
    perror("webserver (bind)");
    return 1;
  }

  printf("socket successfully bound to address\n");

  // listen for incoming connections

  if (listen(sockfd, SOMAXCONN) != 0)
  {
    perror("webserver (listen)");
    return 1;
  }

  printf("server listening for connections\n");

  for (;;)
  {
    /* accept incoming connections */
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsockfd < 0)
    {
      perror("webserver (accept)");
      continue;
    }
    printf("connection accepted\n");

    // get client address
    int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
    if (sockn < 0)
    {
      perror("webserver (getsockname)");
      continue;
    }

    // read from the socket
    int valread = read(newsockfd, buffer, BUFFER_SIZE);
    if (valread < 0)
    {
      perror("webserver (read)");
      continue;
    }

    // read the request
    char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
    sscanf(buffer, "%s %s %s", method, uri, version);
    printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, version, uri);
    // printf("%s\n", buffer);

    // file type
    char *type = getFileType(uri);

    // load the file
    Loadfile(uri, buffer, newsockfd, type);
    close(newsockfd);
  }

  return 0;
}

char *getFileType(char uri[])
{
  char *type;

  if (strstr(uri, ".html") != NULL)
  {
    type = "Content-Type: text/html\r\n\r\n";
    return type;
  }

  if (strstr(uri, ".css") != NULL)
  {
    type = "Content-Type: text/css\r\n\r\n";
    return type;
  }

  if (strcmp(uri, "/") == 0)
  {
    type = "Content-Type: text/html\r\n\r\n";
    return type;
  }
  if (strstr(uri, ".gif") != NULL)
  {
    type = "Content-Type: Content-Type: image/gif\r\n\r\n";
    return type;
  }
  if (strstr(uri, ".jpg") != NULL || strstr(uri, ".jpeg") != NULL)
  {
    type = "Content-Type: Content-Type: image/jpeg\r\n\r\n";
    return type;
  }
  if (strstr(uri, ".php") != NULL)
  {
    type = "Content-Type: application/x-httpd-php\r\n\r\n";
    return type;
  }

  return type = "Content-Type: text/html\r\n\r\n";
}

void Loadfile(char uri[], char *buffer, int newsockfd, char *type)
{

  int bytes;
  FILE *fp;
  char uri1[BUFFER_SIZE];
  // printf("%s\n",uri1 );

  memmove(uri, uri + 1, strlen(uri + 1) + 1);
  char filepath[] = "./pages/";
  strcat(filepath, uri);
  printf("%s\n", filepath);

  fp = fopen(filepath, "r");
  if (fp == NULL)
  {
    fp = fopen("./pages/404.html", "r");
  }

  sprintf(buffer, "HTTP/1.1 200 OK \r\n");
  strcat(buffer, type);
  // sprintf(buffer, "Content-Type: text/css\r\n\r\n");
  // write(newsockfd,buffer, strlen(buffer));
  // printf("%s\n",  buffer);
  send(newsockfd, buffer, strlen(buffer), 0);

  while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
  {
    send(newsockfd, buffer, bytes, 0);
  }

  fclose(fp);
}
