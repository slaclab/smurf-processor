#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Data Re-orderer
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Data Re-orderer Python Package
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

class SmurfChannelMapper(pysmurf.core.common.BaseMasterSlave):
    """
    SMuRF Channel Mapper Python Wrapper.
    """
    def __init__(self, name, **kwargs):
        self._mapper = smurf.core.mappers.SmurfChannelMapper()
        pysmurf.core.common.BaseMasterSlave.__init__(self, name=name, device=self._mapper, description='SMuRF Channel Mapper', **kwargs)