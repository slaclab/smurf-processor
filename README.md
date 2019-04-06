# SMuRF Processor

## Description

This repository contains the SMuRF data processor. This processor receive each data frame from coming from the SMuRF FPGA, and average the data by a configurable number of samples.

Once the average is done, SMuRF packet is assembled. For information about the SMuRF packet structure see [README.SmurfPacket.md](README.SmurfPacket.md).

Each SMuRF packet is placed in a circular buffer. The virtual method `SmurfProcessor::transmit` is called when new packets are available in the buffer. A user can create a custom class, using `SmurfProcessor` as a base class, and overwrite the `transmit` method to perform application specific processing tasks.

On the other hand, each SMuRF packet is also written to disk once it is ready.
