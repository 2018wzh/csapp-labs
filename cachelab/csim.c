// 10245102537 wzh
#include "cachelab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#define NO_CACHE
typedef struct Block
{
    void *data;
} Block;
typedef struct Line
{
    bool valid;
    void *tags;
    Block *blocks;
} CacheLine;
typedef struct Set
{
    CacheLine *lines;
} CacheSet;
typedef struct Cache
{
    CacheSet *sets;
    int s, E, b;
} Cache;
typedef struct Trace
{
    char operation;
    unsigned long address;
    int size;
} Trace;
typedef enum Result
{
    HIT = 1 << 1,
    MISS = 1 << 2,
    EVICTION = 1 << 3,
    IGNORE = 1 << 4,
} Result;
const char help[] = "\
Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n\
Options:\n\
  -h         Print this help message.\n\
  -v         Optional verbose flag.\n\
  -s <num>   Number of set index bits.\n\
  -E <num>   Number of lines per set.\n\
  -b <num>   Number of block offset bits.\n\
  -t <file>  Trace file.\n\
\n\
Examples:\n\
  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n\
  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n\
";
Cache *initCache(int s, int E, int b);
void freeCache(Cache *cache);
void printHelp();
void printResult(Result op, Trace trace);
bool readTrace(FILE *fp, Trace *traces);
Result accessCache(Cache *cache, Trace trace);
int main(int argc, char *argv[])
{
    int s, E, b;
    char *tracefile;
    int hits = 0, misses = 0, evictions = 0;
    FILE *trace_fp;
    Cache *cache;
    Trace trace;
    bool verbose = 0;
    char opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1)
    {
        switch (opt)
        {
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
            tracefile = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            printHelp();
            return 0;
        default:
            printHelp();
            return 1;
        }
    }
    trace_fp = fopen(tracefile, "r");
    if (trace_fp == NULL)
    {
        fprintf(stderr, "%s: No such file or directory", tracefile);
        return 1;
    }
    cache = initCache(s, E, b);
    while (readTrace(trace_fp, &trace))
    {
        if (trace.operation == 'I')
            continue;
        Result op = accessCache(cache, trace);
        if (verbose)
            printResult(op, trace);
        if (op & HIT)
            hits++;
        if (op & MISS)
            misses++;
        if (op & EVICTION)
            evictions++;
    }
    printSummary(hits, misses, evictions);
    freeCache(cache);
    fclose(trace_fp);
    return 0;
}
void printHelp()
{
    printf("%s", help);
}
void printResult(Result op, Trace trace)
{
    printf("%c %lx,%d ", trace.operation, trace.address, trace.size);
    if (op & MISS)
        printf("miss ");
    if (op & EVICTION)
        printf("eviction ");
    if (op & HIT)
        printf("hit ");
    printf("\n");
}
Cache *initCache(int s, int E, int b)
{
    Cache *cache = malloc(sizeof(Cache));
    cache->sets = malloc((1 << s) * sizeof(CacheSet));
    for (int i = 0; i < (1 << s); i++)
    {
        cache->sets[i].lines = malloc(E * sizeof(CacheLine));
        for (int j = 0; j < E; j++)
        {
            cache->sets[i].lines[j].valid = false;
            cache->sets[i].lines[j].tags = NULL;
            cache->sets[i].lines[j].blocks = malloc((1 << b) * sizeof(Block));
        }
    }
    cache->s = s;
    cache->E = E;
    cache->b = b;
    return cache;
}
void freeCache(Cache *cache)
{
    int s = cache->s;
    int E = cache->E;
    for (int i = 0; i < (1 << s); i++)
    {
        for (int j = 0; j < E; j++)
        {
            free(cache->sets[i].lines[j].blocks);
        }
        free(cache->sets[i].lines);
    }
    free(cache->sets);
    free(cache);
}
bool readTrace(FILE *fp, Trace *trace)
{
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        sscanf(buffer, "%c %lx,%d", &trace->operation, &trace->address, &trace->size);
        return 1;
    }
    return 0;
}
#ifdef NO_CACHE
Result accessCache(Cache *cache, Trace trace)
{
    return MISS;
}
#endif