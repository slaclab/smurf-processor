#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Base Slave
#-----------------------------------------------------------------------------
# File       : _slave.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Base Slave Python Package
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

class BaseSlave(pyrogue.Device):
    """
    SMuRF Base Slave Python Wrapper.
    """
    def __init__(self, name, slave, **kwargs):
        pyrogue.Device.__init__(self, name=name, description='SMuRF Base Slave', **kwargs)
        self._slave = slave

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='DisableRx',
            description='Disable the processing block. Data will just pass thorough to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self._slave.disableRx(value),
            localGet=self._slave.isRxDisabled))

        # Add the frame counter variable
        self.add(pyrogue.LocalVariable(
            name='RxFrameCnt',
            description='Rx Frame counter',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._slave.getRxFrameCnt))

        # Add the last frame size variable
        self.add(pyrogue.LocalVariable(
            name='RxFrameSize',
            description='Last Rx frame size',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._slave.getRxFrameSize))

        # Command to clear all the counters
        self.add(pyrogue.LocalCommand(
            name='clearRxCnt',
            description='Clear all Rx counters',
            function=self._slave.clearRxCnt))

    # Method called by streamConnect, streamTap and streamConnectBiDir to access slave
    def _getStreamSlave(self):
        return self._slave