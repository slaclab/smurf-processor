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
    def __init__(self, name, **kwargs):
        self._filter = smurf.core.filters.GeneralAnalogFilter()
        pyrogue.Device.__init__(self, name=name, description='SMuRF Data GeneralAnalogFilter', **kwargs)

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='Disable',
            description='Disable the processing block. Data will just pass thorough to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self._filter.setDisable(value),
            localGet=self._filter.getDisable))

        # Add the filter order variable
        self.add(pyrogue.LocalVariable(
            name='Order',
            description='Filter order',
            mode='RW',
            value=1,
            localSet=lambda value : self._filter.setOrder(value),
            localGet=self._filter.getOrder))

        # Add the filter gain variable
        self.add(pyrogue.LocalVariable(
            name='Gain',
            description='Filter gain',
            mode='RW',
            value=1.0,
            localSet=lambda value : self._filter.setGain(value),
            localGet=self._filter.getGain))

        # Add the filter a coefficients variable
        # Rogue doesn't allow to have an empty list here. Also, the EPICS PV is created
        # with the initial size of this list, and can not be changed later, so we are doing
        # it big enough at this point (we are probably not going to use an order > 10)
        self.add(pyrogue.LocalVariable(
            name='A',
            description='Filter a coefficients',
            mode='RW',
            value=[1.0]+[0]*9,
            localSet=lambda value: self._filter.setA(value),
            localGet=self._filter.getA))

        # Add the filter b coefficients variable
        # Rogue doesn't allow to have an empty list here. Also, the EPICS PV is created
        # with the initial size of this list, and can not be changed later, so we are doing
        # it big enough at this point (we are probably not going to use an order > 10)
        self.add(pyrogue.LocalVariable(
            name='B',
            description='Filter b coefficients',
            mode='RW',
            value=[0.0]*10,
            localSet=lambda value: self._filter.setB(value),
            localGet=self._filter.getB))

    # Method called by streamConnect, streamTap and streamConnectBiDir to access slave
    def _getStreamSlave(self):
        return self._filter

    # Method called by streamConnect, streamTap and streamConnectBiDir to access master
    def _getStreamMaster(self):
        return self._filter

