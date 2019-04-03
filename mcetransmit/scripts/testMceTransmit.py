#!/usr/bin/env python3
#-----------------------------------------------------------------------------
# Title      : Test file for MceTransmit
#-----------------------------------------------------------------------------
# File       : exoTest.py
# Created    : 2018-02-28
#-----------------------------------------------------------------------------
# This file is part of the rogue_example software. It is subject to
# the license terms in the LICENSE.txt file found in the top-level directory
# of this distribution and at:
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
# No part of the rogue_example software, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------
import rogue.utilities
import MceTransmit
import pyrogue
import time

# Data generator
prbsTx = rogue.utilities.Prbs()

# Our receiver
rx = MceTransmit.SmurfProcessor()

# Connect the stream
pyrogue.streamConnect(prbsTx,rx)

# Generate Data
#prbsTx.enable(1000)

try:
    while (True):
        prbsTx.genFrame(2176)
        #print(" Rx: Count {}, Bytes {}, Last {}".format(rx.getCount(), rx.getBytes(), rx.getLast()))
        time.sleep(.0025)

except KeyboardInterrupt:
    pass
