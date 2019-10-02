#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Data Base Transmitter
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Data Base Transmitter Python Package
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
import pysmurf.core.common

class BaseTransmitter(pysmurf.core.common.BaseSlave):
    """
    SMuRF Data BaseTransmitter Python Wrapper.
    """
    def __init__(self, name, **kwargs):
        pyrogue.Device.__init__(self, name=name, description='SMuRF Data BaseTransmitter', **kwargs)
        self._transmitter = smurf.core.transmitters.BaseTransmitter()
        pysmurf.core.common.BaseSlave.__init__(self, name=name, slave=self._transmitter, description='SMuRF Data BaseTransmitter', **kwargs)
