#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Base Master and Slave
#-----------------------------------------------------------------------------
# File       : _device.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Base Master and Slave Python Package
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

class BaseMasterSlave(pyrogue.Device):
    """
    SMuRF Base Slave Python Wrapper.
    """
    def __init__(self, name, device, **kwargs):
        pyrogue.Device.__init__(self, name=name, description='SMuRF Base Master and Slave', **kwargs)
        self._device = device

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='DisableRx',
            description='Disable the processing block. Data will just pass thorough to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self._device.disableRx(value),
            localGet=self._device.isRxDisabled))

        # Add the frame counter variable
        self.add(pyrogue.LocalVariable(
            name='RxFrameCnt',
            description='Rx Frame counter',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._device.getRxFrameCnt))

        # Add the last frame size variable
        self.add(pyrogue.LocalVariable(
            name='RxFrameSize',
            description='Last Rx frame size',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._device.getRxFrameSize))

        # Command to clear all the counters
        self.add(pyrogue.LocalCommand(
            name='clearRxCnt',
            description='Clear all Rx counters',
            function=self._device.clearRxCnt))

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='DisableTx',
            description='Disable the frame transmission. No frames will be send to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self._master.disableTx(value),
            localGet=self._master.isTxDisabled))

        # Add the frame counter variable
        self.add(pyrogue.LocalVariable(
            name='TxFrameCnt',
            description='Tx Frame counter',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._master.getTxFrameCnt))

        # Add the last frame size variable
        self.add(pyrogue.LocalVariable(
            name='TxFrameSize',
            description='Last Tx frame size',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._master.getTxFrameSize))

        # Command to clear all the counters
        self.add(pyrogue.LocalCommand(
            name='clearTxCnt',
            description='Clear all Tx counters',
            function=self._master.clearTxCnt))

    # Method called by streamConnect, streamTap and streamConnectBiDir to access slave
    def _getStreamSlave(self):
        return self._device

    # Method called by streamConnect, streamTap and streamConnectBiDir to access master
    def _getStreamMaster(self):
        return self._device