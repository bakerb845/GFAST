#include <math.h>
#include "gfast_core.h"


/*!
 * @brief Unpack the quality channel. It should be a 6 digit integer, with the
 *        first 2 digits being n_satellites, middle 2 digits being pdop, and
 *        the last 2 digits being chi^2 as calculated by CWU (formerly fixtype)
 * 
 * @param[in] q_value          Q channel value
 *
 * @result the computed chi^2 value for the station
 *
 * @author Brendan Crowell (PNSN) and Carl Ulberg (PNSN)
 *
 * @date Dec 2022
 *
 */
double core_waveformProcessor_parseQChannelChi2CWU(
    const double q_value)
{
    double chi2;
    int chimap;
    
    // Extract the last 2 digits
    chimap = core_waveformProcessor_parseQChannelChi2CWUmap(q_value);

    // Apply conversion to real numbers
    // Reverse operation provided by Craig Scrivner of CWU
    // int( ( log10(chi2) + 4 ) * 25 )
    // Should map 0.0001 -> 0, 0.001 -> 25, 0.01 -> 50, 0.99 -> 99
    chi2 = pow(10, (double)(chimap) / 25 - 4);

    return chi2;
}


/*!
 * @brief Unpack the quality channel. It should be a 6 digit integer, with the
 *        first 2 digits being n_satellites, middle 2 digits being pdop, and
 *        the last 2 digits being chi^2 as calculated by CWU (formerly fixtype)
 * 
 * @param[in] q_value          Q channel value. It is a double to stay consistent
 *                             with other data buffers. But treated as integer here.
 *
 * @result the mapped chi^2 value for the channel (between 00-99)
 *
 * @author Brendan Crowell (PNSN) and Carl Ulberg (PNSN)
 *
 * @date Dec 2022
 *
 */
int core_waveformProcessor_parseQChannelChi2CWUmap(
    const double q_value)
{
    int chimap;
    
    // Extract the last 2 digits
    if (q_value < 0) {
        LOG_WARNMSG("parseQChannelChi2CWU - q_value is negative (%lf), changing to positive!",
            q_value);
        chimap = (int)(q_value * -1) % 100;
    } else {
        chimap = (int)(q_value) % 100;
    }

    return chimap;
}


/*!
 * @brief Unpack the quality channel. The most recent definition as of 3/22/23 was:
 *        Q = 100000*int(10*PDOP) + 1000*nsat + 10*fixtyp + goodness
 * 
 * @param[in] q_value          Q channel value. It is a double to stay consistent
 *                             with other data buffers. But treated as integer here.
 *
 * @result the goodness value (0 or 1) - a combination of chi^2, nsats, clocks, etc.
 *
 * @author Brendan Crowell (PNSN) and Carl Ulberg (PNSN)
 *
 * @date Mar 2023
 *
 */
int core_waveformProcessor_parseQChannelGoodness(
    const double q_value)
{
    int goodness;
    
    // Extract the last 1 digit, should only be 0 or 1, so modulo 2 should work
    if (q_value < 0) {
        LOG_WARNMSG("parseQChannelChi2CWU - q_value is negative (%lf), changing to positive!",
            q_value);
        goodness = (int)(q_value * -1) % 2;
    } else {
        goodness = (int)(q_value) % 2;
    }

    return goodness;
}