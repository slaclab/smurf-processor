#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Data GeneralAnalogFilter
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Data GeneralAnalogFilter Python Package
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

class GeneralAnalogFilter(pyrogue.Device):
    """
    SMuRF Data GeneralAnalogFilter Python Wrapper.
    """
    def __init__(self, size, name, **kwargs):
        pyrogue.Device.__init__(self, name=name, description='SMuRF Data GeneralAnalogFilter', **kwargs)
        self._filter = smurf.core.filters.GeneralAnalogFilter(size)

    # Method called by streamConnect, streamTap and streamConnectBiDir to access slave
    def _getStreamSlave(self):
        return self._filter

    # Method called by streamConnect, streamTap and streamConnectBiDir to access master
    def _getStreamMaster(self):
        return self._filter