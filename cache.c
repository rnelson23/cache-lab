#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct Line {
    struct Line *next;
    int numReferences;
    int lastReferenced;
    int index;
    int valid;
    int tag;
};

struct Set {
    struct Set *next;
    struct Line *lineHead;
    int index;
};

struct Set *initializeCache(int numSets, int numLines);
void addSet(struct Set *current, int numLines);
void addLine(struct Line *current);
struct Set *findSet(struct Set *set, int index);
int findLine(struct Line *line, int tag);
void replaceLine(struct Line *lineHead, int tag, char replacementPolicy[3]);
struct Line *findLFU(struct Line *lineHead);
struct Line *findLRU(struct Line *lineHead);

int numCycles = 0;

int main() {
    int S, E, B, m;

    printf("Input cache parameters:\n\n");

    printf("  S: ");
    scanf("%d", &S);

    printf("  E: ");
    scanf("%d", &E);

    printf("  B: ");
    scanf("%d", &B);

    printf("  m: ");
    scanf("%d", &m);

    struct Set *setHead = initializeCache(S, E);

    char replacementPolicy[3];
    int h, p;

    printf("\nInput replacement policy:\n\n  ");
    scanf("%s", replacementPolicy);

    printf("\nInput hardware parameters:\n\n");

    printf("  h: ");
    scanf("%d", &h);

    printf("  p: ");
    scanf("%d", &p);

    int s = (int)(log(S) / log(2));
    int b = (int)(log(B) / log(2));

    unsigned int input;
    unsigned int addresses[100];
    int numAddresses;

    int tag;
    int index;

    double numMisses = 0;
    double numReferences = 0;

    printf("\nInput Addresses:\n\n");

    do {
        printf("  ");
        scanf("%x", &input);

        addresses[numAddresses] = input;
        numAddresses++;

    } while (input != -1);

    printf("\nOutput:\n\n");

    for (int i = 0; i < numAddresses; i++) {
        unsigned int *address = &addresses[i];

        if (*address == -1) {
            int missRate = (int)round((numMisses / numReferences) * 100);
            printf("  %d %d\n\n", missRate, numCycles);

            break;
        }

        tag = (int)(*address >> (s + b));
        index = (int)((*address >> b) & ((1 << s) - 1));

        struct Set *set = findSet(setHead, index);
        int hit = findLine(set->lineHead, tag);

        numReferences++;
        numCycles += h;

        if (hit == 1) {
            printf("  %x H\n", *address);

        } else if (hit == 0) {
            numMisses++;
            numCycles += p;
            printf("  %x M\n", *address);

        } else if (hit == -1) {
            replaceLine(set->lineHead, tag, replacementPolicy);

            numMisses++;
            numCycles += p;
            printf("  %x M\n", *address);
        }
    }

    return 0;
}

struct Set *initializeCache(int numSets, int numLines) {
    struct Set *setHead = malloc(sizeof(struct Set));
    struct Line *lineHead = malloc(sizeof(struct Line));

    setHead->next = NULL;
    setHead->lineHead = lineHead;
    setHead->index = 0;

    lineHead->next = NULL;
    lineHead->numReferences = 0;
    lineHead->index = 0;
    lineHead->valid = 0;
    lineHead->tag = 0;

    for (int i = 0; i < numLines - 1; i++) addLine(lineHead);
    for (int i = 0; i < numSets - 1; i++) addSet(setHead, numLines);

    return setHead;
}

void addSet(struct Set *current, int numLines) {
    if (current->next == NULL) {
        struct Set *next = malloc(sizeof(struct Set));
        struct Line *lineHead = malloc(sizeof(struct Line));

        next->next = NULL;
        next->lineHead = lineHead;
        next->index = current->index + 1;

        lineHead->next = NULL;
        lineHead->numReferences = 0;
        lineHead->index = 0;
        lineHead->valid = 0;
        lineHead->tag = 0;

        for (int i = 0; i < numLines - 1; i++) addLine(lineHead);

        current->next = next;

    } else {
        addSet(current->next, numLines);
    }
}

void addLine(struct Line *current) {
    if (current->next == NULL) {
        struct Line *next = malloc(sizeof(struct Line));

        next->next = NULL;
        next->numReferences = 0;
        next->index = current->index + 1;
        next->valid = 0;
        next->tag = 0;

        current->next = next;

    } else {
        addLine(current->next);
    }
}

struct Set *findSet(struct Set *set, int index) {
    if (set->index == index) {
        return set;

    } else if (set->next != NULL) {
        return findSet(set->next, index);

    } else {
        return NULL;
    }
}

int findLine(struct Line *line, int tag) {
    if (line->index == 0 && line->next == NULL) {
        if (line->valid == 1 && line->tag == tag) {
            return 1;

        } else {
            line->valid = 1;
            line->tag = tag;

            return 0;
        }

    } else {
        if (line->valid == 1) {
            if (line->tag == tag) {
                line->numReferences++;
                line->lastReferenced = numCycles;

                return 1;

            } else if (line->next != NULL) {
                findLine(line->next, tag);

            } else {
                return -1;
            }

        } else {
            line->valid = 1;
            line->tag = tag;
            line->numReferences++;
            line->lastReferenced = numCycles;

            return 0;
        }
    }
}

void replaceLine(struct Line *lineHead, int tag, char *replacementPolicy) {
    struct Line *line;

    if (strcmp(replacementPolicy, "LFU") == 0) line = findLFU(lineHead);
    else if (strcmp(replacementPolicy, "LRU") == 0) line = findLRU(lineHead);

    line->numReferences++;
    line->lastReferenced = numCycles;
    line->tag = tag;
}

struct Line *findLFU(struct Line *lineHead) {
    struct Line *line = lineHead;
    struct Line *LFU = lineHead;

    while (line->next != NULL) {
        if (line->next->numReferences < line->numReferences) {
            LFU = line->next;
        }

        line = line->next;
    }

    return LFU;
}

struct Line *findLRU(struct Line *lineHead) {
    struct Line *line = lineHead;
    struct Line *LRU = lineHead;

    while (line->next != NULL) {
        if (line->next->lastReferenced < line->lastReferenced) {
            LRU = line->next;
        }

        line = line->next;
    }

    return LRU;
}
