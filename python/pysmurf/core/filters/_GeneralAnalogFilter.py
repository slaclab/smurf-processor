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
import pysmurf.core.common

class GeneralAnalogFilter(pysmurf.core.common.BaseMasterSlave):
    """
    SMuRF Data GeneralAnalogFilter Python Wrapper.
    """
    def __init__(self, size, name, **kwargs):
        self._filter = smurf.core.filters.GeneralAnalogFilter(size)
        pysmurf.core.common.BaseMasterSlave.__init__(self, name=name, device=self._filter, description='SMuRF Data GeneralAnalogFilter', **kwargs)
