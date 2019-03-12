#!/bin/bash

#defaults
SHELFMANAGER=10.0.1.4
CRATEID=3
SMURFSLOT=2
NOGUI=
# PCIe interface is default
INTERFACE=pcie-rssi-interleaved
INTERM="screen -h 81920 -d -m -S pyrogue "
EPICS_ROOT=test_epics
PARAMS=""

#ed07dde0
DEFAULTS_YML=/usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/config/defaults.yml
PYROGUE=/usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/images/current.pyrogue.tar.gz

while (( "$#" )); do
  case "$1" in
    -m|--shm)
      SHELFMANAGER=$2
      shift 2
      ;;
    -c|--crateid)
      CRATEID=$2
      shift 2
      ;;
    -i|--interm)
      INTERM=""
      shift 1
      ;;
    -s|--smurfslot)
      SMURFSLOT=$2
      shift 2
      ;;
    -r|--epics-root)
      EPICS_ROOT=$2
      shift 2
      ;;
    -y|--defaults)
      DEFAULTS_YML=$2
      shift 2
      ;;
    -t|--pyrogue)
      PYROGUE=$2
      shift 2
      ;;
    --nogui)
      NOGUI=-s
      shift 1
      ;;
    -e)
      INTERFACE=eth-rssi-interleaved
      shift 1
      ;;
    --) # end argument parsing
      shift
      break
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done

# set positional arguments in their proper place
eval set -- "$PARAMS"

# some derived inputs
SMURFIP=10.0.${CRATEID}.$((100+${SMURFSLOT}))
RSSI_LINK=$(($SMURFSLOT-2))

if screen -list | grep -q "pyrogue"
then
    screen -X -S pyrogue kill
fi

f="./log/$(date +"%FT%H%M%S")_smurf_run.log"
amcc_dump_bsi --all ${SHELFMANAGER}/${SMURFSLOT} |& tee $f

## ed07dde0
echo ${INTERM} /home/cryo/smurf2mce/current/mcetransmit/scripts/control-server/start_server.sh -a ${SMURFIP} -c ${INTERFACE} -l ${RSSI_LINK} -t ${PYROGUE} -d ${DEFAULTS_YML} -e ${EPICS_ROOT} -f Int16 -b 524288 ${NOGUI} 
## d4495370
#${INTERM} /home/cryo/smurf2mce/current/mcetransmit/scripts/control-server/start_server.sh -a ${SMURFIP} -c ${INTERFACE} -l ${RSSI_LINK} -t /home/cryo/lbhb_fw_test2/MicrowaveMuxBpEthGen2-0x00000001-20190215083416-mdewart-d4495370.pyrogue.tar.gz -d /home/cryo/lbhb_fw_test2/defaults_lbhb_fw_test2_d4495370.yml -e ${EPICS_ROOT} -f Int16 -b 524288 ${NOGUI} 
## 6752dc80
#${INTERM} /home/cryo/smurf2mce/current/mcetransmit/scripts/control-server/start_server.sh -a ${SMURFIP} -c ${INTERFACE} -l ${RSSI_LINK} -t /home/cryo/shawn/fw_tests/lbhb_fw_test3/MicrowaveMuxBpEthGen2-0x00000001-20190308094902-mdewart-6752dc80.pyrogue.tar.gz -d /home/cryo/shawn/fw_tests/lbhb_fw_test3/defaults_lbhb_fw_test3_6752dc80_mmcswap.yml -e ${EPICS_ROOT} -f Int16 -b 524288 ${NOGUI} 

## 3/12/19 attempting to run with 2x carriers.  Basically ignoring inputs.