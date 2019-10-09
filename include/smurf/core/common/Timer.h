#ifndef _SMURF_CORE_COMMON_TIMER_H_
#define _SMURF_CORE_COMMON_TIMER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Timer
 * ----------------------------------------------------------------------------
 * File          : Timer.h
 * Created       : 2019-10-01
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Timer Class.
 *-----------------------------------------------------------------------------
 * This file is part of the smurf software platform. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the smurf software platform, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/

// Class use to measure the time a scope is active
class Timer
{
public:
    Timer(std::string n)
    :
        name(n),
        t((std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())).time_since_epoch().count())
    {
    };

    ~Timer()
    {
        std::cout << name << ", start = " << t << std::endl;
        std::cout << name << ", end   = " <<
            (std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())).time_since_epoch().count() - t
            << std::endl;
    };

private:
    std::string name;
    uint64_t    t;
};
#endif