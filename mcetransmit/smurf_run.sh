
f="./log/$(date +"%FT%H%M%S")_smurf_run.log"
amcc_dump_bsi --all 10.0.1.30/2 |& tee $f
#scripts/control-server/start_server.sh -a 10.0.2.102 -c eth-rssi-interleaved -l 0 -t /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/images/current.pyrogue.tar.gz -d /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/config/defaults.yml -e test_epics -f Int16 -b 524288
scripts/control-server/start_server.sh -a 10.0.2.102 -c pcie-rssi-interleaved -l 0 -t /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/images/current.pyrogue.tar.gz -d /usr/local/controls/Applications/smurf/cmb_Det/cryo-det/ultrascale+/firmware/targets/MicrowaveMuxBpEthGen2/config/defaults.yml -e test_epics -f Int16 -b 524288

