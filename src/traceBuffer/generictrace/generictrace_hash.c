#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "gfast_core.h"
#include "gfast_traceBuffer.h"


uint32_t traceBuffer_generictrace_hash(const char *s) {
    uint32_t hashval;
    // K&R[2]. In testing, 21 collisions with ~20k NSLCs
    // for (hashval = 0; *s != '\0'; s++) {
    //     hashval = *s + 31 * hashval;
    // }

    // djb2. In testing, zero collisions with ~20k NSLCs
    for (hashval = 5381; *s != '\0'; s++) {
        // hashval = *s + 33 * hashval;
        hashval = *s + ((hashval << 5) + hashval);
    }

    return hashval;
}

uint32_t traceBuffer_generictrace_make_hash(const char *s, uint32_t hashsize) {
    return traceBuffer_generictrace_hash(s) % hashsize;
}

struct generictrace_node *traceBuffer_generictrace_hashmap_contains(struct generictrace_hashmap_struct *hashmap,
                                                   const char *name)
{
    struct generictrace_node *np;

    // Advance through linked list
    for (np = hashmap->map[traceBuffer_generictrace_make_hash(name, hashmap->hashsize)];
         np != NULL;np = np->next)
    {
        if (strcmp(name, np->name) == 0) {
            // hashmap contains name
            return np;
        }
    }
    return NULL;
}

int traceBuffer_generictrace_hashmap_remove(struct generictrace_hashmap_struct *hashmap, const char *name) {
    struct generictrace_node *np, *np_prev;

    // Advance through linked list
    for (np = hashmap->map[traceBuffer_generictrace_make_hash(name, hashmap->hashsize)], np_prev = NULL;
         np != NULL; np = np->next)
    {
        if (strcmp(name, np->name) == 0) {
            // hashmap contains name
            if (np_prev != NULL) {
                np_prev->next = np->next;
            } else {
                hashmap->map[traceBuffer_generictrace_make_hash(name, hashmap->hashsize)] = np->next;
            }
            traceBuffer_generictrace_free_node(np);
            return 1;
        }
        np_prev = np;
    }
    return 0;
}

struct generictrace_node *traceBuffer_generictrace_hashmap_add(struct generictrace_hashmap_struct *hashmap,
                                              const char *name,
                                              int index)
{
    struct generictrace_node *np;
    uint32_t hashval;

    if ((np = traceBuffer_generictrace_hashmap_contains(hashmap, name)) == NULL) {
        // Not found
        np = (struct generictrace_node *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL) {
            printf("ERROR! No space for %s!\n", name);
            return NULL;
        }
        hashval = traceBuffer_generictrace_make_hash(name, hashmap->hashsize);
        np->next = hashmap->map[hashval];
        hashmap->map[hashval] = np;
    } else {
        np->i = -1;
    }
    np->i = index;
    return np;
}

void traceBuffer_generictrace_free_node(struct generictrace_node *np) {
    free(np->name);
    free(np);
}

void traceBuffer_generictrace_free_hashmap(struct generictrace_hashmap_struct *hashmap) {
    struct generictrace_node *np, *np_next;
    uint32_t i;
    for (i = 0; i < hashmap->hashsize; i++) {
        for (np = hashmap->map[i]; np != NULL; np = np_next) {
            np_next = np->next;
            traceBuffer_generictrace_free_node(np);
        }
    }
    free(hashmap->map);
    free(hashmap);
}

void traceBuffer_generictrace_print_hashmap(struct generictrace_hashmap_struct *hashmap) {
    unsigned i;
    struct generictrace_node *np;
    int cx, n_nodes, max_nodes = -1;
    const int max_nodes_to_print = 250;
    double running_sum = 0;

    for (i = 0; i < hashmap->hashsize; i++) {
        // First count number of nodes, is it even possible?
        for (np = hashmap->map[i], n_nodes = 0; np != NULL; np = np->next, n_nodes++);
        running_sum += n_nodes;
        if (n_nodes > max_nodes) {
            max_nodes = n_nodes;
        }

        if (n_nodes > (int)(.9 * max_nodes_to_print)) {
            LOG_DEBUGMSG("%5d (#%d) Not printing all, too many nodes in this position (>%d)!",
                i, n_nodes, (int)(.9 * max_nodes_to_print));
            continue;
        }

        // N characters * max number of stations per hash
        int nchar = 24 * max_nodes_to_print + 4;
        char str[nchar];
        cx = 0;
        cx += snprintf(str + cx, nchar - cx, "%5d (#%5d)", i, n_nodes);
        for (np = hashmap->map[i]; np != NULL; np = np->next) {
            cx += snprintf(str + cx, nchar - cx, " -> %s(%d)", np->name, np->i);
        }
        LOG_DEBUGMSG("%s", str);
    }

}

int traceBuffer_generictrace_print_true_collisions(struct generictrace_hashmap_struct *hashmap) {
    int n_nodes;
    struct generictrace_node *np;
    uint32_t *hashvals;
    char **names;
    int i, j, k;
    int n_collisions = 0;

    // Loop over list indices
    for (i = 0; i < (int)(hashmap->hashsize); i++) {
        // Get total nodes at this position
        for (np = hashmap->map[i], n_nodes = 0; np != NULL; np = np->next, n_nodes++);
        hashvals = (uint32_t *) malloc(n_nodes * sizeof(uint32_t));
        names = (char **) malloc(n_nodes * sizeof(char *));

        // Loop over linked nodes to get full hash values
        for (np = hashmap->map[i], j = 0; np != NULL; np = np->next, j++) {
            hashvals[j] = traceBuffer_generictrace_hash(np->name);
            names[j] = strdup(np->name);
        }

        // Loop over hashvals
        for (j = 0; j < n_nodes - 1; j++) {
            for (k = j + 1; k < n_nodes; k++) {
                if (hashvals[j] == hashvals[k]) {
                    n_collisions++;
                    LOG_DEBUGMSG("%d Collision! %s:%u, %s:%u",
                        i, names[j], hashvals[j], names[k], hashvals[k]);
                }
            }
        }
        
        // Free stuff
        for (j = 0; j < n_nodes; j++) {
            free(names[j]);
        }
        free(names);
        free(hashvals);
    }
    LOG_DEBUGMSG("%d collisions", n_collisions);

    return n_collisions;
}
