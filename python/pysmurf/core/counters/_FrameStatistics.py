#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Frame Statistics
#-----------------------------------------------------------------------------
# File       : _FrameStatistics.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Frame Statistics Python Package
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

class FrameStatistics(pysmurf.core.common.BaseMasterSlave):
    """
    SMuRF Frame Statistics Python Wrapper.
    """
    def __init__(self, name, **kwargs):
        self._FrameStatistics = smurf.core.counters.FrameStatistics()
        pysmurf.core.common.BaseMasterSlave.__init__(self, name=name, device=self._FrameStatistics, description='SMuRF Frame Statistics', **kwargs)

        # Add the frame lost counter  variable
        self.add(pyrogue.LocalVariable(
            name='FrameLossCnt',
            description='Number of lost frames',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._FrameStatistics.getFrameLossCnt))

        # Add the out-of-order frames variable
        self.add(pyrogue.LocalVariable(
            name='FrameOutOrderCnt',
            description='Number of time we have received out-of-order frames',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._FrameStatistics.getFrameOutOrderCnt))