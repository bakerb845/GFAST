#include "gfast_traceBuffer.h"
#include "gfast_core.h"
#include <string.h>
#include <stdlib.h>


// Defining comparator function as per the requirement
int traceBuffer_gnsstrace_myCompare2(const void *x, const void *y)
{
    // Should sort by s,n,l,c,time
    const struct string_index xx = *(const struct string_index *) x;
    const struct string_index yy = *(const struct string_index *) y;
    int ista, inet, iloc, icha;

    ista = strcmp(xx.sta, yy.sta);
    if (ista == 0) {
        inet = strcmp(xx.net, yy.net);
        if (inet == 0) {
            iloc = strcmp(xx.loc, yy.loc);
            if (iloc == 0) {
                icha = strcmp(xx.cha, yy.cha);
                if (icha == 0) {
                    if (xx.time > yy.time) {
                        return 1;
                    }
                    else if (xx.time < yy.time) {
                        return -1;
                    }
                    else {
                        return 0;
                    }
                }
                else { // order by {LYZ, LYN, LYE} to match gnsstraceData
                    return -1 * icha;
                }
            }
            else {
                return iloc;
            }
        }
        else {
            return inet;
        }
    }
    else {
        return ista;
    }
}

// Function to sort the array
void traceBuffer_gnsstrace_sort2(struct string_index values[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort((void *) values, (size_t) n, sizeof(struct string_index), traceBuffer_gnsstrace_myCompare2);
}

void traceBuffer_gnsstrace_printStringindex(struct string_index *d, int n) {
    int i;
    for (i = 0; i < n; i++){
        LOG_DEBUGMSG("CCC struct[%3d] indx:%5d: nscl:%s nsamps:%d time:%.2f",
            i, d[i].indx, d[i].nscl, d[i].nsamps, d[i].time);
    }
}