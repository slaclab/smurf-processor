#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PySMuRF Processor
#-----------------------------------------------------------------------------
# File       : __init__.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF Processor device.
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
import pysmurf
import pysmurf.core.counters
import pysmurf.core.mappers
import pysmurf.core.unwrappers
import pysmurf.core.filters
import pysmurf.core.downsamplers
import pysmurf.core.conventers
import pysmurf.core.transmitters

class SmurfProcessor(pyrogue.Device):
    """
    SMuRF Processor device.

    This device accept a raw SMuRF Streaming data stream from
    the FW application, and process it applying th following block:
    - Channel mapping,
    - Data unwrap,
    - Data filter,
    - Data downsampler
    - SMuRF header inserter

    After this, the data goes both to a Rogue data writer and to a
    transmitter block.
    """
    def __init__(self, name, description, master, **kwargs):
        pyrogue.Device.__init__(self, name=name, description=description, **kwargs)

        self.master = master

        self.smurf_frame_stats = pysmurf.core.counters.FrameStatistics(name="FrameRxStats")
        self.add(self.smurf_frame_stats)

        self.smurf_mapper = pysmurf.core.mappers.SmurfChannelMapper(name="ChannelMapper")
        self.add(self.smurf_mapper)

        self.smurf_unwrapper = pysmurf.core.unwrappers.Unwrapper(name="Unwrapper")
        self.add(self.smurf_unwrapper)

        self.smurf_filter = pysmurf.core.filters.GeneralAnalogFilter(name="Filter")
        self.add(self.smurf_filter)

        self.smurf_downsampler = pysmurf.core.downsamplers.Downsampler(name="Downsampler")
        self.add(self.smurf_downsampler)

        self.smurf_header2smurf = pysmurf.core.conventers.Header2Smurf(name="Header2Smurf")
        self.add(self.smurf_header2smurf)

        self.test_data_writer = pyrogue.utilities.fileio.StreamWriter(name='FileWriter')
        self.add(self.test_data_writer)

        pyrogue.streamConnect(self.master,             self.smurf_frame_stats)
        pyrogue.streamConnect(self.smurf_frame_stats,  self.smurf_mapper)
        pyrogue.streamConnect(self.smurf_mapper,       self.smurf_unwrapper)
        pyrogue.streamConnect(self.smurf_unwrapper,    self.smurf_filter)
        pyrogue.streamConnect(self.smurf_filter,       self.smurf_downsampler)
        pyrogue.streamConnect(self.smurf_downsampler,  self.smurf_header2smurf)
        pyrogue.streamConnect(self.smurf_header2smurf, self.test_data_writer.getChannel(0))
