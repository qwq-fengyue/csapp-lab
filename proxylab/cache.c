#include "cache.h"

sem_t read_cnt_mutex, time_mutex, write_mutex;

void cache_init(cache_t *cp)
{
  cp->buf = (cacheline *)Calloc(CACHE_LINE, sizeof(cacheline));
  memset(cp->buf, 0, CACHE_LINE * sizeof(cacheline));
  cp->read_cnt = 0;
  cp->curr_time = 0;
  Sem_init(&read_cnt_mutex, 0, 1);
  Sem_init(&time_mutex, 0, 1);
  Sem_init(&write_mutex, 0, 1);
}

void cache_deinit(cache_t *cp)
{
  Free(cp->buf);
}

int read_cache(cache_t *cp, char *uri, int fd)
{
  P(&read_cnt_mutex);
  cp->read_cnt++;
  if (cp->read_cnt == 1)
    P(&write_mutex);
  V(&read_cnt_mutex);

  int find = -1;
  for (int i = 0; i < CACHE_LINE; i++)
  {
    if (cp->buf[i].size && !strcmp(uri, cp->buf[i].uri))
    {
      find = i;
      break;
    }
  }
  if (find != -1)
  {
    Rio_writen(fd, cp->buf[find].object, cp->buf[find].size);
    P(&time_mutex);
    cp->buf[find].time = ++cp->curr_time;
    V(&time_mutex);
  }

  P(&read_cnt_mutex);
  cp->read_cnt--;
  if (cp->read_cnt == 0)
    V(&write_mutex);
  V(&read_cnt_mutex);

  return find != -1;
}

void write_cache(cache_t *cp, char *uri, char *object, int size)
{
  P(&write_mutex);

  int min_time = cp->buf[0].time;
  int min_index = 0;
  for (int i = 1; i < CACHE_LINE; i++)
  {
    if (cp->buf[i].time < min_time)
    {
      min_time = cp->buf[i].time;
      min_index = i;
    }
  }

  strcpy(cp->buf[min_index].uri, uri);
  strcpy(cp->buf[min_index].object, object);
  cp->buf[min_index].size = size;
  P(&time_mutex);
  cp->buf[min_index].time = ++cp->curr_time;
  V(&time_mutex);
  printf("cached\n");

  V(&write_mutex);
}