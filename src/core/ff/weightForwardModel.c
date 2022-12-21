#include <stdio.h>
#include <stdlib.h>
#include "gfast_core.h"
#include "iscl/array/array.h"
/*!
 * @brief Weights a forward modeling matrix by the diagonal data weights
 *
 * @param[in] mrows     number of rows in forward modeling matrix (should
 *                      be 3*number_of_observations)
 * @param[in] ncols     number of columns in forward modeling matrix
 * @param[in] diagWt    diagonal of data weight matrix [mrows]
 * @param[in] G         unweighted forward modeling matrix.  This is
 *                      in row major format and is of size [mrows x ncols]
 *                      Notice, because G is in row major order that it
 *                      is okay to apply this function to the regularized
 *                      matrix G understanding the regulizartion terms of
 *                      G will be scaled by unity (i.e. unchanged).
 *
 * @param[out] diagWtG  weighted forward modeling matrix obtained by
 *                      computing \f$ \tilde{G} \leftarrow diag\{W\} G \f$.
 *                      This is in row major format and is of size
 *                      [mrows x ncols]
 *
 * @result -1 indicates an input error
 *          0 indicates success
 *          1 indicates the diagonal weight matrix is NULL.  G will
 *            not be modified in this instance.
 *
 * @author Ben Baker, ISTI
 *
 */
int core_ff_weightForwardModel(const int mrows, const int ncols,
                               const double *__restrict__ diagWt,
                               const double *__restrict__ G,
                               double *__restrict__ diagWtG)
{
    int i, j;
    if (mrows < 1 || ncols < 1)
    {
        LOG_ERRMSG("Error G has invalid dimension %d %d", mrows, ncols);
        return -1;
    }
    if (G == NULL || diagWtG == NULL)
    {
        if (G == NULL){LOG_ERRMSG("%s", "Error G cannot be NULL");}
        if (diagWtG == NULL)
        {
            LOG_ERRMSG("%s", "Error diagWtG cannot be NULL");
        }
        return -1;
    }
    // Avoid a segmentation fault
    if (diagWt == NULL)
    {
        LOG_WARNMSG("%s", "Warning diagWt is NULL; G will not be weighted");
        array_copy64f_work(mrows*ncols, G, diagWtG);
        return 1;
    }
    // Compute \tilde{G} = diag\{W\}*G
#ifdef _OPENMP
    #pragma omp simd collapse(2)
#endif
    for (i=0; i<mrows; i++)
    {
        for (j=0; j<ncols; j++)
        {
            diagWtG[i*ncols+j] = G[i*ncols+j]*diagWt[i];
        }
    }
    return 0;
}
