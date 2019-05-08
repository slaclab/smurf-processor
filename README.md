# SMuRF Processor

## Description

This repository contains the SMuRF data processor. This processor receive each data frame from coming from the SMuRF FPGA, and average the data by a configurable number of samples.

Once the average is done, SMuRF packet is assembled. For information about the SMuRF packet structure see [README.SmurfPacket.md](README.SmurfPacket.md).

Each SMuRF packet is placed in a circular buffer. The buffer contains objects of class `SmurfPacket` which gives an interface to SMuRF packet data.

The virtual method `SmurfProcessor::transmit` is called when new packets are available in the buffer. A user can create a custom class, using `SmurfProcessor` as a base class, and overwrite the `transmit` method to perform application specific processing tasks. The `transmit` method receives a (smart) pointer to a SMuRF packet object, in read only mode.

Additionally, this processor writes each SMuRF packet to disk once new packets are available.
