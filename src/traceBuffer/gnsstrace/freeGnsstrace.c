#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_traceBuffer.h"
#include "iscl/memory/memory.h"

/*!
 * @brief Clears the memory, and optionally the SNCLs, on the gnsstrace_struct
 *        struct
 *
 * @param[in] clearSNCL  if true then also clear the SNCL on the trace
 *
 * @param[out] trace     if clearSNCL is true then all memory is freed, counters
 *                       are reset to zero, and the SNCL's are cleared.
 *                       if false then all memory is free and counters are
 *                       cleared.
 *
 * @author Ben Baker (ISTI)
 *
 * @copyright Apache 2
 *
 */
void traceBuffer_gnsstrace_freeGnsstrace(const bool clearSNCL,
                                   struct gnsstrace_struct *trace)
{
    memory_free32i(&trace->data);
    memory_free64f(&trace->times);
    memory_free32i(&trace->chunkPtr);
    trace->nchunks = 0;
    trace->npts = 0;
    trace->dt = 0.0;
    if (clearSNCL)
    {
        memset(trace, 0, sizeof(struct gnsstrace_struct));
    }
    return;
}
