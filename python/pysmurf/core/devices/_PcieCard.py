#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : SMuRF PCIe Card
#-----------------------------------------------------------------------------
# File       : _PcieCard.py
# Created    : 2019-09-30
#-----------------------------------------------------------------------------
# Description:
#    SMuRF PCIe card device.
#-----------------------------------------------------------------------------
# This file is part of the smurf software platform. It is subject to
# the license terms in the LICENSE.txt file found in the top-level directory
# of this distribution and at:
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
# No part of the smurf software platform, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------

from pathlib import Path
import pyrogue

class PcieCard():
    """
    Class to setup the PCIe RSSI card.

    This class takes care of setting up PCIe card according to the communication
    type used.

    If the PCIe card is present in the system:
    - All the RSSI connection links which point to the target IP address will
      be closed.
    - If PCIe comunication type is used, the RSSI connection is open in the
      specific link. Also, when the the server is closed, the RSSI connection
      is closed.

    If the PCIe card is not present:
    - If PCIe comunication type is used, the program is terminated.
    - If ETH communication type is used, then this class does not do anything.

    This class must be used in a 'with' block in order to ensure that the
    RSSI connection is close correctly during exit even in the case of an
    exepction condition.
    """

    def __init__(self, comm_type, link, ip_addr, dev):

        print("Setting up the RSSI PCIe card...")

        # Get system status:

        # Check if the PCIe card is present in the system
        if Path(dev).exists():
            self.pcie_present = True
        else:
            self.pcie_present = False

        # Check if we use the PCIe for communication
        if 'pcie-' in comm_type:
            self.use_pcie = True
        else:
            self.use_pcie = False

        # Look for configuration errors:

        # Check if we are trying to use PCIe communication without the Pcie
        # card present in the system
        if self.use_pcie and not self.pcie_present:
            exit_message("  ERROR: PCIe device {} does not exist.".format(dev))

        # When the PCIe is in used verify the link number is valid
        if self.use_pcie:
            if link == None:
                exit_message("  ERROR: Must specify an RSSI link number")

            if link in range(0, 6):
                self.link = link
            else:
                exit_message("  ERROR: Invalid RSSI link number. Must be between 0 and 5")

        # Should need to check that the IP address is defined when PCIe is present
        # and not in used, but that is enforce in the main function. We need to
        # know the IP address so we can look for all RSSI links that point to it
        # and close their connections.

        # Not more configuration errors at this point

        # Prepare the PCIe when present
        if self.pcie_present:

            # Build the pyrogue device for the PCIe board
            import rogue.hardware.axi
            import SmurfKcu1500RssiOffload as fpga
            self.pcie = pyrogue.Root(name='pcie',description='')
            memMap = rogue.hardware.axi.AxiMemMap(dev)
            self.pcie.add(fpga.Core(memBase=memMap))
            self.pcie.start(pollEn='False',initRead='True')

            # Verify if the PCIe card is configured with a MAC and IP address.
            # If not, load default values before it can be used.
            valid_local_mac_addr = True
            local_mac_addr = self.pcie.Core.EthLane[0].EthConfig.LocalMac.get()
            if local_mac_addr == "00:00:00:00:00:00":
                valid_local_mac_addr = False
                self.pcie.Core.EthLane[0].EthConfig.LocalMac.set("08:00:56:00:45:50")
                local_mac_addr = self.pcie.Core.EthLane[0].EthConfig.LocalMac.get()

            valid_local_ip_addr = True
            local_ip_addr = self.pcie.Core.EthLane[0].EthConfig.LocalIp.get()
            if local_ip_addr == "0.0.0.0":
                valid_local_ip_addr = False
                self.pcie.Core.EthLane[0].EthConfig.LocalIp.set("10.0.3.99")
                local_ip_addr = self.pcie.Core.EthLane[0].EthConfig.LocalIp.get()


            # If the IP was not defined, read the one from the register space.
            # Note: this could be the case only the PCIe is in used.
            if not ip_addr:
                ip_addr = self.pcie.Core.EthLane[0].UdpClient[self.link].ClientRemoteIp.get()

                # Check if the IP address read from the PCIe card is valid
                try:
                    socket.inet_pton(socket.AF_INET, ip_addr)
                except socket.error:
                    exit_message("ERROR: IP Address read from the PCIe card: {} is invalid.".format(ip_addr))

            # Update the IP address.
            # Note: when the PCIe card is not in used, the IP will be defined
            # by the user.
            self.ip_addr = ip_addr

        # Print system configuration and status
        print("  - PCIe present in the system             : {}".format(
            "Yes" if self.pcie_present else "No"))
        print("  - PCIe based communicartion selected     : {}".format(
            "Yes" if self.use_pcie else "No"))

        # Show IP address and link when the PCIe is in use
        if self.use_pcie:
            print("  - Valid MAC address                      : {}".format(
                "Yes" if valid_local_mac_addr else "No. A default address was loaded"))
            print("  - Valid IP address                       : {}".format(
                "Yes" if valid_local_ip_addr else "No. A default address was loaded"))
            print("  - Local MAC address:                     : {}".format(local_mac_addr))
            print("  - Local IP address:                      : {}".format(local_ip_addr))
            print("  - Using IP address                       : {}".format(self.ip_addr))
            print("  - Using RSSI link number                 : {}".format(self.link))

        # Print the FW version information when the PCIe is present
        if self.pcie_present:
            self.print_version()

        # When the PCIe card is not present we don't do anything

    def __enter__(self):
        # Close all RSSI links that point to the target IP address
        self.close_all_rssi()

        # Open the RSSI link
        self.open_rssi()

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        # Close the RSSI link before exit
        self.close_rssi()

        # Stop the device
        if self.pcie_present:
            self.pcie.stop()

    def open_rssi(self):
        """
        Open the RSSI connection in the specified link
        """

        # Check if the PCIe is present and in used
        if self.pcie_present and self.use_pcie:
            print("  * Opening RSSI link...")
            self.__configure(open=True, link=self.link)
            print("  Done!")
            print("")

    def close_rssi(self):
        """
        Close the RSSI connection in the specified link
        """

        # Check if the PCIe is present and in used
        if self.pcie_present and self.use_pcie:
            print("  * Closing RSSI link...")
            self.__configure(open=False, link=self.link)
            print("  Done!")
            print("")

    def close_all_rssi(self):
        """
        Close all links with the target IP address
        """

        # Check if the PCIe is present
        if self.pcie_present:
            print("  * Looking for RSSI links pointing to {}...".format(self.ip_addr))
            # Look for links with the target IP address, and close their RSSI connection
            for i in range(6):
                if self.ip_addr == self.pcie.Core.EthLane[0].UdpClient[i].ClientRemoteIp.get():
                    print("    RSSI Link {} points to it. Disabling it...".format(i))
                    self.__configure(open=False, link=i)
                    print("")
            print("  Done!")
            print("")

    def print_version(self):
        """
        Print the FW version information
        """

        # Print inforamtion if the PCIe is present
        if self.pcie_present:
            # Call readAll so that the LinkVariables get updated correctly.
            self.pcie.ReadAll.call()
            print("  ==============================================================")
            print("                         PCIe information")
            print("  ==============================================================")
            print("    FW Version      : 0x{:08X}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.FpgaVersion.get()))
            print("    FW GitHash      : 0x{:040X}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.GitHash.get()))
            print("    FW image name   : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.ImageName.get()))
            print("    FW build env    : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.BuildEnv.get()))
            print("    FW build server : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.BuildServer.get()))
            print("    FW build date   : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.BuildDate.get()))
            print("    FW builder      : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.Builder.get()))
            print("    Up time         : {}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.UpTime.get()))
            print("    Xilinx DNA ID   : 0x{:032X}".format(
                self.pcie.Core.AxiPcieCore.AxiVersion.DeviceDna.get()))
            print("  ==============================================================")
            print("")

    def __configure(self, open, link):

        # Read the bypass RSSI mask
        mask = self.pcie.Core.EthLane[0].EthConfig.BypRssi.get()
        dis  = mask
        dis |= (1<<link)
        self.pcie.Core.EthLane[0].EthConfig.BypRssi.set(dis)
        time.sleep(1)

        if open:
            print("    Opening PCIe RSSI link {}".format(link))

            # Clear the RSSI bypass bit
            mask &= ~(1<<link)

            # Setup udp client IP address and port number
            self.pcie.Core.EthLane[0].UdpClient[link].ClientRemoteIp.set(self.ip_addr)
            self.pcie.Core.EthLane[0].UdpClient[link].ClientRemotePort.set(8198)
        else:
            print("    Closing PCIe RSSI link {}".format(link))

            # Set the RSSI bypass bit
            mask |= (1<<link)

            # Setup udp client port number
            self.pcie.Core.EthLane[0].UdpClient[link].ClientRemotePort.set(8192)

        # Set the bypass RSSI mask
        self.pcie.Core.EthLane[0].EthConfig.BypRssi.set(mask)

        # Set the Open and close connection registers
        self.pcie.Core.EthLane[0].RssiClient[link].CloseConn.set(int(not open))
        self.pcie.Core.EthLane[0].RssiClient[link].OpenConn.set(int(open))
        self.pcie.Core.EthLane[0].RssiClient[link].HeaderChksumEn.set(1)

        # Printt register status after setting them
        print("      PCIe register status:")
        print("      EthConfig.BypRssi = 0x{:02X}".format(
            self.pcie.Core.EthLane[0].EthConfig.BypRssi.get()))
        print("      UdpClient[{}].ClientRemoteIp = {}".format(link,
            self.pcie.Core.EthLane[0].UdpClient[link].ClientRemoteIp.get()))
        print("      UdpClient[{}].ClientRemotePort = {}".format(link,
            self.pcie.Core.EthLane[0].UdpClient[link].ClientRemotePort.get()))
        print("      RssiClient[{}].CloseConn = {}".format(link,
            self.pcie.Core.EthLane[0].RssiClient[link].CloseConn.get()))
        print("      RssiClient[{}].OpenConn = {}".format(link,
            self.pcie.Core.EthLane[0].RssiClient[link].OpenConn.get()))
