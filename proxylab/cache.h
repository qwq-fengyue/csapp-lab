#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_LINE 10

typedef struct
{
  int size;
  int time;
  char uri[MAXLINE];
  char object[MAX_OBJECT_SIZE];
} cacheline;

typedef struct
{
  cacheline *buf;
  int read_cnt;
  int curr_time;
} cache_t;

void cache_init(cache_t *cp);
void cache_deinit(cache_t *cp);
int read_cache(cache_t *cp, char *uri, int fd);
void write_cache(cache_t *cp, char *uri, char *object, int size);

#endif