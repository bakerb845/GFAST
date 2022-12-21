#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"

/*!
 * @brief Computes the right hand side in the peak ground displacement 
 *        estimation s.t.
 *        \f$ \textbf{b} = \left \{ \log_{10}(d) - A \right \} \f$
 *        where A is a scalar shift and d is the distance at each station.
 *
 * @param[in] n         number of points
 * @param[in] dist_tol  distance tolerance - if d is less than this then
 *                      it will be set to a default value (cm)
 * @param[in] dist_def  distance default value (cm)
 * @param[in] A         shift so that b = log10(d) - A
 * @param[in] d         max distance (cm) at each site [n]
 *
 * @param[out] b        right hand side in Gm = b [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_setRHS(const int n,
                            const double dist_tol,
                            const double dist_def,
                            const double A,
                            const double *__restrict__ d,
                            double *__restrict__ b)
{
    double dist;
    int i;
    if (n < 1)
    {
        LOG_ERRMSG("Invalid number of points: %d", n);
        return -1;
    }
    for (i=0; i<n; i++)
    {
        dist = d[i];
        if (dist - dist_tol < 0.0){dist = dist_def;}
        b[i] = log10(dist) - A;
    }
    return 0;
}
