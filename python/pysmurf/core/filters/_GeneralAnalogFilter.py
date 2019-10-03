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

        # Add the number of enabled channels  variable
        self.add(pyrogue.LocalVariable(
            name='NumChannels',
            description='Number of channels being processed',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self._filter.getNumCh))

        # Add the filter order variable
        self.add(pyrogue.LocalVariable(
            name='Order',
            description='Filter order',
            mode='RW',
            value=1,
            localSet=lambda value : self._filter.setOrder(value)))

        # Add the filter gain variable
        self.add(pyrogue.LocalVariable(
            name='Gain',
            description='Filter gain',
            mode='RW',
            value=1.0,
            localSet=lambda value : self._filter.setGain(value)))

        # Add the filter a coefficients variable
        self.add(pyrogue.LocalVariable(
            name='A',
            description='Filter a coefficients',
            mode='RW',
            value=[1.0],  # Rogue doesn't allow to have an empty list here.
            localSet=lambda value: self._filter.setA(value)))

        # Add the filter b coefficients variable
        self.add(pyrogue.LocalVariable(
            name='B',
            description='Filter b coefficients',
            mode='RW',
            value=[1.0],  # Rogue doesn't allow to have an empty list here.
            localSet=lambda value: self._filter.setB(value)))

