// 10245102537 wzh
#include "cachelab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_ADDRESS_SIZE 64
typedef struct Node
{
    struct Node *next;
    struct Node *prev;
    uint64_t index;
} Node;
typedef struct LRU
{
    Node *head, *tail;
    uint64_t size; // Number of valid lines in the set
} LRU;
typedef struct Cache
{
    LRU *sets;
    uint64_t t; // Number of tag bits
    uint64_t s; // Number of set index bits
    uint64_t E; // Number of lines per set
    uint64_t b; // Number of block offset bits
} Cache;
typedef enum Operation
{
    LOAD = 'L',
    STORE = 'S',
    MODIFY = 'M',
    INSTRUCTION = 'I',
} Operation;
typedef struct Trace
{
    Operation operation;
    uint64_t address, size;
} Trace;
typedef enum Result
{
    HIT = 1,
    DUAL_HIT = 1 << 1,
    MISS = 1 << 2,
    DUAL_MISS = 1 << 3,
    EVICTION = 1 << 4,
    DUAL_EVICTION = 1 << 5,
    IGNORE = 1 << 6,
} Result;
const char help[] = {
    "\
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
"};
// Print funcs
void printDebug(Cache *cache);            // Print cache state
void printHelp();                         // Print help message
void printResult(Result op, Trace trace); // Print result
// cache funcs
Result accessBlock(Cache *cache, uint64_t addr);                // Access block in cache
Result accessCache(Cache *cache, uint64_t addr, uint64_t size); // Access cache
void freeCache(Cache *cache);                                   // Free cache
Cache *initCache(int s, int E, int b);                          // Initialize cache
// LRU funcs
Node *findNode(LRU *lru, uint64_t index); // Find node in LRU
void freeLRU(LRU *lru);                   // Free LRU list
void insertHead(LRU *lru, uint64_t addr); // Insert node at head of LRU
void removeTail(LRU *lru);                // Remove tail node from LRU
void updateLRU(LRU *lru, Node *node);     // Update LRU list
// trace funcs
bool readTrace(FILE *fp, Trace *traces); // Read trace file

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
    if (!s || !E || !b || !tracefile)
    {
        fprintf(stderr, "%s: Missing required command line argument\n", *argv);
        printHelp();
        return 1;
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
        Result res = 0;
        uint64_t addr = trace.address;
        uint64_t size = trace.size;
        switch (trace.operation)
        {
        case INSTRUCTION:
            res = IGNORE;
            break;
        case LOAD:
            res += accessCache(cache, addr, size);
            break;
        case STORE:
            res += accessCache(cache, addr, size);
            break;
        case MODIFY:
            res += accessCache(cache, addr, size);
            res += accessCache(cache, addr, size);
            break;
        }
        if (res == IGNORE)
            continue;
#ifdef DEBUG
        printDebug(cache);
#endif
        if (verbose)
            printResult(res, trace);
        if (res & HIT)
            hits++;
        if (res & DUAL_HIT)
            hits += 2;
        if (res & MISS)
            misses++;
        if (res & DUAL_MISS)
            misses += 2;
        if (res & EVICTION)
            evictions++;
        if (res & DUAL_EVICTION)
            evictions += 2;
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
    if (op == IGNORE)
        return;
    printf("%c %lx,%ld ", trace.operation, trace.address, trace.size);
    if (op & MISS)
        printf("miss ");
    if (op & DUAL_MISS)
        printf("miss miss ");
    if (op & EVICTION)
        printf("eviction ");
    if (op & DUAL_EVICTION)
        printf("eviction eviction ");
    if (op & HIT)
        printf("hit ");
    if (op & DUAL_HIT)
        printf("hit hit ");
    printf("\n");
}
Cache *initCache(int s, int E, int b)
{
    Cache *cache = malloc(sizeof(Cache));
    cache->sets = malloc((1 << s) * sizeof(LRU));
    for (int i = 0; i < (1 << s); i++)
    {
        // Allocate memory for lines in each set
        cache->sets[i].head = NULL;
        cache->sets[i].tail = NULL;
        cache->sets[i].size = 0;
    }
    cache->s = s;
    cache->E = E;
    cache->b = b;
    cache->t = MAX_ADDRESS_SIZE - (s + b);
    return cache;
}
void freeCache(Cache *cache)
{
    int s = cache->s;
    for (int i = 0; i < (1 << s); i++)
        freeLRU(&cache->sets[i]);
    free(cache->sets);
    free(cache);
}
bool readTrace(FILE *fp, Trace *trace)
{
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char op;
        if (buffer[0] == 'I')
        {
            trace->operation = INSTRUCTION;
            return 1;
        }
        sscanf(buffer, " %c %lx,%ld", &op, &trace->address, &trace->size);
        switch (op)
        {
        case 'L':
            trace->operation = LOAD;
            break;
        case 'S':
            trace->operation = STORE;
            break;
        case 'M':
            trace->operation = MODIFY;
            break;
        default:
            return 0;
        }
        return 1;
    }
    return 0;
}
Result accessCache(Cache *cache, uint64_t addr, uint64_t size)
{
    Result res = 0;
    uint64_t mask = ~((1 << cache->b) - 1);
    uint64_t block_addr = addr & mask;
    // Access the block in the cache
    for (uint64_t offset = 0; offset < size; offset += (1 << cache->b))
        res |= accessBlock(cache, block_addr + offset);
    return res;
}
Result accessBlock(Cache *cache, uint64_t addr)
{
    uint64_t mask = (1 << cache->s) - 1;
    uint64_t index = (addr >> cache->b) & mask;   // Index bits
    uint64_t tag = addr >> (cache->b + cache->s); // Tag bits
    LRU *set = &cache->sets[index];
    Result res = 0;
    Node *node = findNode(set, tag);
    if (node != NULL)
    {
        res |= HIT;
        updateLRU(set, node);
    }
    else
    {
        if (set->size < cache->E)
        {
            res |= MISS;
            insertHead(set, tag);
        }
        else
        {
            res |= MISS | EVICTION;
            removeTail(set);
            insertHead(set, tag);
        }
    }
    return res;
}
void insertHead(LRU *lru, uint64_t val)
{
    Node *node = malloc(sizeof(Node));
    node->index = val;
    node->next = lru->head;
    node->prev = NULL;
    if (lru->head != NULL)
        lru->head->prev = node;
    lru->head = node;
    if (lru->tail == NULL)
        lru->tail = node;
    lru->size++;
}
void removeTail(LRU *lru)
{
    if (lru->tail == NULL)
        return;
    Node *node = lru->tail;
    lru->tail = node->prev;
    if (lru->tail != NULL)
        lru->tail->next = NULL;
    else
        lru->head = NULL;
    free(node);
    lru->size--;
}
void updateLRU(LRU *lru, Node *node)
{
    if (node == lru->head)
        return;
    if (node == lru->tail)
        lru->tail = node->prev;
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    node->next = lru->head;
    node->prev = NULL;
    if (lru->head != NULL)
        lru->head->prev = node;
    lru->head = node;
}
void freeLRU(LRU *lru)
{
    Node *node = lru->head;
    while (node != NULL)
    {
        Node *temp = node;
        node = node->next;
        free(temp);
    }
}
Node *findNode(LRU *lru, uint64_t val)
{
    Node *node = lru->head;

    while (node != NULL)
    {
        if (node->index == val)
            return node;
        node = node->next;
    }
    return NULL;
}
void printDebug(Cache *cache)
{
    for (int i = 0; i < (1 << cache->s); i++)
    {
        if (cache->sets[i].size == 0)
            continue;
        printf("Set %d: ", i);
        Node *node = cache->sets[i].head;
        while (node != NULL)
        {
            printf("%lx ", node->index);
            node = node->next;
        }
        printf("\n");
    }
}