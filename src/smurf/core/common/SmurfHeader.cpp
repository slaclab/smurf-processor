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

SmurfHeaderROPtr SmurfHeaderRO::create(ris::FrameIterator it)
{
    return std::make_shared<SmurfHeaderRO>(it);
}

// Function to get header words
const uint32_t SmurfHeaderRO::getFrameCounter() const
{
    return getU32Word(headerFrameCounterOffset);
}

// Helper functions
const uint16_t SmurfHeaderRO::getU32Word(std::size_t offset) const
{
    union
    {
        uint16_t w;
        uint8_t  b[2];
    } aux;

    for (std::size_t i{0}; i < 2; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

const uint32_t SmurfHeaderRO::getU32Word(std::size_t offset) const
{
    union
    {
        uint32_t w;
        uint8_t  b[4];
    } aux;

    for (std::size_t i{0}; i < 4; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

const uint64_t SmurfHeaderRO::getU32Word(std::size_t offset) const
{
    union
    {
        uint64_t w;
        uint8_t  b[8];
    } aux;

    for (std::size_t i{0}; i < 8; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

