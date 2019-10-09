#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Processor (Monolithic)
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Processor device, in its monolithic version.
#-----------------------------------------------------------------------------
# This file is part of the smurf software platform. It is subject to
# the license terms in the LICENSE.txt file found in the top-level directory
# of this distribution and at:
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
# No part of the smurf software platform, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------

import pyrogue
import smurf
import smurf.core.processors

class SmurfProcessor(pyrogue.Device):
    """
    SMuRF Processor device.

    This device accept a raw SMuRF Streaming data stream from
    the FW application, and process it by doing channel mapping,
    data unwrapping, filtering and downsampling in a monolithic
    C++ module.
    """
    def __init__(self, name, description, master, **kwargs):
        pyrogue.Device.__init__(self, name=name, description=description, **kwargs)

        self.master = master
        self.smurf_processor = smurf.core.processors.SmurfProcessor()
        pyrogue.streamConnect(self.master, self.smurf_frame_stats)

