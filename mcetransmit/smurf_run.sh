#!/bin/bash

#defaults
SHELFMANAGER=10.0.1.4
CRATEID=3
SMURFSLOT=5
NOGUI=

PARAMS=""

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
    -s|--smurfslot)
      SMURFSLOT=$2
      shift 2
      ;;
    --nogui)
      NOGUI=-s
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

if screen -list | grep -q "pyrogue"
then
    screen -X -S pyrogue kill
fi

f="./log/$(date +"%FT%H%M%S")_smurf_run.log"
amcc_dump_bsi --all ${SHELFMANAGER}/${SMURFSLOT} |& tee $f

screen -h 81920 -d -m -S pyrogue /home/cryo/smurf2mce/current/mcetransmit/scripts/control-server/start_server.sh -a ${SMURFIP} -c pcie-rssi-interleaved -l 0 -t /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/images/current.pyrogue.tar.gz -d /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/config/defaults.yml -e test_epics -f Int16 -b 524288 ${NOGUI} 


