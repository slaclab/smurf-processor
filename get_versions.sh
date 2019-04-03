CWD=`pwd`

#pysmurf version
cdpysmurf
echo "pysmurf : (branch,githash)=("`git branch | grep \* | cut -d ' ' -f2`","`git rev-parse --short HEAD`")"

#transmitter
cdtransmit
echo "smurf2mce : (branch,githash)=("`git branch | grep \* | cut -d ' ' -f2`","`git rev-parse --short HEAD`")"

#smurftestapps
cdtestapps
echo "smurftestapps : (branch,githash)=("`git branch | grep \* | cut -d ' ' -f2`","`git rev-parse --short HEAD`")"

cd $CWD