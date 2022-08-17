#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "cachelab.h"

void update(uint64_t address);
void printInfo(void);

int T = 0;
int h,v,s,E,b,S;
int hit_count, miss_count, eviction_count;
char t[1000];

typedef struct
{
    int time;
    uint64_t tag;
}cache_line;
cache_line** cache = NULL;

int main(int argc, char** argv)
{
    int opt;
    h = v = 0;
    hit_count = miss_count = eviction_count = 0;
    while (-1 != (opt = (getopt(argc, argv, "hvs:E:b:t:"))))
    {
        switch(opt)
        {
            case 'h':
                h = 1;
                break;
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(t, optarg);
                break;
            default:
                printInfo();
                break;
        }
    }

    if (h == 1)
    {
        printInfo();
        exit(0);
    }

    if (s <= 0 || E <= 0 || b <= 0)
    {
        printf("Illegal parameters\n");
        exit(-1);
    }
    S = 1 << s;

    FILE* fp = fopen(t, "r");
    if (fp == NULL)
    {
        printf("Open file failed\n");
        exit(-1);
    }

    cache = (cache_line**) malloc(sizeof(cache_line*) * S);
    for (int i = 0; i < S; i++)
    {
        cache[i] = (cache_line*) malloc(sizeof(cache_line) * E);
        for (int j = 0; j < E; j++)
            cache[i][j].time = 0;
    }

    char operation;
    uint64_t address;
    int size;
    while (fscanf(fp, " %c %lx,%d\n", &operation, &address, &size) > 0)
    {
        switch(operation)
        {
            case 'I':
                continue;
            case 'M':
                update(address);
            case 'L':
            case 'S':
                update(address);
                break;
        }
    }

    fclose(fp);
    for (int i = 0; i < S; i++)
        free(cache[i]);
    free(cache);

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

void update(uint64_t address)
{
    uint64_t set = (address >> b) & ((-1U) >> (64 - s));
    uint64_t tag = address >> (s + b);

    for (int i = 0; i < E; i++)
    {
        if (cache[set][i].tag == tag && cache[set][i].time > 0)
        {
            cache[set][i].time = ++T;
            hit_count++;
            return;
        }
    }

    for (int i = 0; i < E; i++)
    {
        if (cache[set][i].time == 0)
        {
            cache[set][i].time = ++T;
            cache[set][i].tag = tag;
            miss_count++;
            return;
        }
    }

    int time_min = cache[set][0].time;
    int time_index = 0;
    for (int i = 1; i < E; i++)
    {
        if (cache[set][i].time < time_min)
        {
            time_min = cache[set][i].time;
            time_index = i;
        }
    }
    cache[set][time_index].time = ++T;
    cache[set][time_index].tag = tag;
    miss_count++;
    eviction_count++;
    return;
}

void printInfo()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

