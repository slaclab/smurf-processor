#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Processor (Monolithic)
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Processor device, in its monolithic version.
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
import pysmurf.core.counters
import smurf.core.processors
import pysmurf.core.conventers

class SmurfChannelMapper(pyrogue.Device):
    """
    SMuRF Channel Mapper Python Wrapper.
    """
    def __init__(self, name, device, **kwargs):
        pyrogue.Device.__init__(self, name=name, description='SMuRF Channel Mapper', **kwargs)
        self.device = device

        # Add "Disable" variable
        self.add(pyrogue.LocalVariable(
            name='Disable',
            description='Disable the processing block. Data will just pass thorough to the next slave.',
            mode='RW',
            value=False,
            localSet=lambda value: self.device.setDisable(value),
            localGet=self.device.getDisable))

        # Add the number of enabled channels  variable
        self.add(pyrogue.LocalVariable(
            name='NumChannels',
            description='Number enabled channels',
            mode='RO',
            value=0,
            pollInterval=1,
            localGet=self.device.getNumCh))

        # Add variable to set the mapping mask
        # Rogue doesn't allow to have an empty list here. Also, the EPICS PV is created
        # with the initial size of this list, and can not be changed later, so we are doing
        # it big enough at this point using the maximum number of channels
        self.add(pyrogue.LocalVariable(
            name='Mask',
            description='Set the mapping mask',
            mode='RW',
            value=[0]*528,
            localSet=lambda value: self.device.setMask(value),
            localGet=self.device.getMask))


class SmurfProcessor(pyrogue.Device):
    """
    SMuRF Processor device.

    This device accept a raw SMuRF Streaming data stream from
    the FW application, and process it by doing channel mapping,
    data unwrapping, filtering and downsampling in a monolithic
    C++ module.
    """
    def __init__(self, name, description, master, **kwargs):
        pyrogue.Device.__init__(self, name=name, description=description, **kwargs)

        self.master = master

        self.smurf_frame_stats = pysmurf.core.counters.FrameStatistics(name="FrameRxStats")
        self.add(self.smurf_frame_stats)

        self.smurf_processor = smurf.core.processors.SmurfProcessor()

        self.smurf_mapper = SmurfChannelMapper(name="ChannelMapper", device=self.smurf_processor)
        self.add(self.smurf_mapper)

        self.smurf_header2smurf = pysmurf.core.conventers.Header2Smurf(name="Header2Smurf")
        self.add(self.smurf_header2smurf)

        self.file_writer = pyrogue.utilities.fileio.StreamWriter(name='FileWriter')
        self.add(self.file_writer)

        pyrogue.streamConnect(self.master,             self.smurf_frame_stats)
        pyrogue.streamConnect(self.smurf_frame_stats,  self.smurf_processor)
        pyrogue.streamConnect(self.smurf_processor,    self.smurf_header2smurf)
        pyrogue.streamConnect(self.smurf_header2smurf, self.file_writer.getChannel(0))
