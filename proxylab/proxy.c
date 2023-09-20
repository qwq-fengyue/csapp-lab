#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"
#include "cache.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

#define NTHREADS 4
#define SBUFSIZE 16
sbuf_t sbuf;

cache_t cache;
void doit(int fd);
void parse_uri(char *uri, char *hostname, char *port, char *path);
void build_new_req(rio_t *rp, char *new_req, char *method, char *hostname, char *port, char *path);
void send_to_server(int fd, char *uri, char *hostname, char *port, char *request);
void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "usage :%s <port> \n", argv[0]);
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);

    cache_init(&cache);

    sbuf_init(&sbuf, SBUFSIZE);
    for (int i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }

    return 0;
}

void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
    char new_req[MAXLINE];

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return;
    sscanf(buf, "%s %s %s", method, uri, version);

    if (read_cache(&cache, uri, fd))
        return;
    // parse request line
    parse_uri(uri, hostname, port, path);
    // parse request header and build new request
    build_new_req(&rio, new_req, method, hostname, port, path);
    // send request to server and send response to client
    send_to_server(fd, uri, hostname, port, new_req);
}

void parse_uri(char *uri, char *hostname, char *port, char *path)
{
    char uri_copy[MAXLINE];
    strcpy(uri_copy, uri);
    if (strstr(uri_copy, "http://") != uri_copy)
    {
        fprintf(stderr, "Error: invalid uri!\n");
        exit(0);
    }

    char *host_pos = uri_copy + strlen("http://");
    char *port_pos = strstr(host_pos, ":");
    char *path_pos = strstr(host_pos, "/");
    if (port_pos == NULL)
    {
        strcpy(port, "80");
        if (path_pos == NULL)
        {
            strcpy(path, "/");
            strcpy(hostname, host_pos);
        }
        else
        {
            strcpy(path, path_pos);
            *path_pos = '\0';
            strcpy(hostname, host_pos);
        }
    }
    else
    {
        if (path_pos == NULL)
        {
            strcpy(port, port_pos + 1);
            strcpy(path, "/");
            *port_pos = '\0';
            strcpy(hostname, host_pos);
        }
        else
        {
            strcpy(path, path_pos);
            *path_pos = '\0';
            strcpy(port, port_pos + 1);
            *port_pos = '\0';
            strcpy(hostname, host_pos);
        }
    }
}

void build_new_req(rio_t *rp, char *new_req, char *method, char *hostname, char *port, char *path)
{
    sprintf(new_req, "%s %s HTTP/1.0\r\n", method, path);
    char *write_pos = new_req + strlen(new_req);
    char buf[MAXLINE];
    while (Rio_readlineb(rp, buf, MAXLINE) > 0)
    {
        if (strstr(buf, "\r\n"))
        {
            sprintf(write_pos, "\r\n");
            break;
        }
        else if (strstr(buf, "Host:"))
        {
            sprintf(write_pos, "Host: %s:%s\r\n", hostname, port);
        }
        else if (strstr(buf, "User-Agent:"))
        {
            sprintf(write_pos, "%s", user_agent_hdr);
        }
        else if (strstr(buf, "Connection:"))
        {
            sprintf(write_pos, "%s", conn_hdr);
        }
        else if (strstr(buf, "Proxy-Connection:"))
        {
            sprintf(write_pos, "%s", proxy_conn_hdr);
        }
        else
        {
            sprintf(write_pos, "%s", buf);
        }
        write_pos += strlen(buf);
    }
}

void send_to_server(int fd, char *uri, char *hostname, char *port, char *request)
{
    int n, total_size = 0;
    int clientfd = Open_clientfd(hostname, port);
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    Rio_writen(clientfd, request, strlen(request));
    char buf[MAXLINE], object_buf[MAX_OBJECT_SIZE];
    // memset(object_buf, 0, sizeof(object_buf));
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        Rio_writen(fd, buf, n);
        if (total_size + n < MAX_OBJECT_SIZE)
        {
            strcpy(object_buf + total_size, buf);
        }
        total_size += n;
    }

    if (total_size < MAX_OBJECT_SIZE)
        write_cache(&cache, uri, object_buf, total_size);

    Close(clientfd);
}
