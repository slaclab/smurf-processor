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

class SmurfChannelMapper(pyrogue.Device):
    """
    SMuRF Channel Mapper Python Wrapper.
    """
    def __init__(self, name, **kwargs):
        self._mapper = smurf.core.mappers.SmurfChannelMapper()
        pyrogue.Device.__init__(self, name=name, description='SMuRF Channel Mapper', **kwargs)

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='Disable',
            description='Disable the processing block. Data will just pass thorough to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self._mapper.setDisable(value),
            localGet=self._mapper.getDisable))

        # Add the number of enabled channels  variable
        self.add(pyrogue.LocalVariable(
            name='NumChannels',
            description='Number enabled channels',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._mapper.getNumCh))

        # Add variable to set the mapping mask
        # Rogue doesn't allow to have an empty list here. Also, the EPICS PV is created
        # with the initial size of this list, and can not be changed later, so we are doing
        # it big enough at this point using the maximum number of channels
        self.add(pyrogue.LocalVariable(
            name='Mask',
            description='Set the mapping mask',
            mode='RW',
            value=[0]*528,
            localSet=lambda value: self._mapper.setMask(value),
            localGet=self._mapper.getMask))

    # Method called by streamConnect, streamTap and streamConnectBiDir to access slave
    def _getStreamSlave(self):
        return self._mapper

    # Method called by streamConnect, streamTap and streamConnectBiDir to access master
    def _getStreamMaster(self):
        return self._mapper