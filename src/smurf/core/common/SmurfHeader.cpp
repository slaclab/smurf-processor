/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Header
 * ----------------------------------------------------------------------------
 * File          : SmurfHeader.cpp
 * Created       : 2019-10-01
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Header Class.
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

#include "smurf/core/common/SmurfHeader.h"


SmurfHeaderRO::SmurfHeaderRO(ris::FrameIterator it)
:
    headerIt(it)
{
}

static SmurfHeaderROPtr SmurfHeaderRO::create(ris::FrameIterator it)
{
    return std::make_shared<SmurfHeaderRO>(it);
}

const uint32_t SmurfHeaderRO::getFrameCounter() const
{
    U32 fn;

    for (std::size_t i{0}; i < 4; ++i)
        fn.b[i] = *(headerIt+headerFrameCounterOffset+i);

    return fn.w;
}
