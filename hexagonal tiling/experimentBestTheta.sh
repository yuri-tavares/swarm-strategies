#!/bin/bash
# The optional argument of this script is a file name with overrides for variables below.
# In this script, if the vector variable samples is defined, it is used for computed the increment for each value of the variable given. Otherwise, it is used the vector increment[i] for each index of variable i.

### Default values ###
# maximum number of retries if the log is empty
RETRYTIMES=1;
# Variables names for test.sh. Angle is measured in degrees
defaultVarNames=(s    d   y0  v    time)
# default values for each variable
defaultValues=(  3.0 1.0  100 1.0  70000000)
# number of repetitions for each variable value
nTests=1
# variables name to do more experiments
varsExp=( s )
# minimum and maximum values for each variable to do more experiments
minExp=( 1.165 )     
maxExp=( 1.195 )  
#increment of above values, since the value minExp until the variable is greater than maxExp 
increment=( 0.002 )   
# Name of the folder where the temporary configuration ini file named in variable configFile will be created
# and where the directories containg the logs will be created.
folderConf=.
# The name of the log for the experiments performed by this script is randomly generated until a not used is found.
experimentsLog="experimentsLog$RANDOM.txt"
while [ -f "$folderConf/$experimentsLog" ]; do experimentsLog="experimentsLog$RANDOM.txt"; done
# The name of the scenario file for stage is random too
scenarioFileName="automatic_$RANDOM"
while [ -f "$scenarioFileName.world" ]; do scenarioFileName="automatic_$RANDOM"; done

# The configuration file passed in first argument overrides the above values
if [ ! -z $1  ]; then
  . $1
fi

# Find a variable value and return it
# Argument 1: variable name being run
# Argument 2: the variable to find
findVariable(){
  local varName=$1
  local findMe=$2
  if [ "$varName" == "$findMe" ]; then
    ret=$varValue
  else
    for (( i=0; i<${#defaultVarNames[@]}; i++ )); do
      if  [ "${defaultVarNames[$i]}" == "$findMe" ]; then
        ret=${defaultValues[$i]}
        break
      fi
    done
  fi
  echo $ret
}

# Calculates the best theta for given parameters and appends it in configuration file
# Argument 1: name of variable.
# Argument 2: value of this variable.
# Argument 3: name of configuration file.
addLineWithBestTheta(){
  local varName=$1
  local varValue=$2
  local configName=$3
  local s=`findVariable $varName s`
  local d=`findVariable $varName d`
  local time=`findVariable $varName time`
  local v=`findVariable $varName v`
  local bestT=`python3 findBestTheta.py $s $d $((time/1000000)) $v`
  echo "theta="`echo "$bestT" | awk '{printf "%.30f", 180*$1/atan2(0, -1)}'` >> $configName
}

# Creates a config file from arguments.
# Argument 1: name of variable.
# Argument 2: value of this variable.
# Argument 3: name of configuration file.
# Argument 4: folder name for save the log file.
# Argument 5: log's filename.
makeConfigFile(){
  if [ $# -ne 5 ]; then
    echo "Wrong number of parameter for makeConfigFile." $# "are given."
    echo "Parameters were: " $@
    return 1
  fi
  local varName=$1
  local varValue=$2
  local configName=$3
  local folderLog=$4
  local logname=$5
  echo "log="$logname > $configName
  echo "folder="$folderLog >> $configName
  size=${#defaultVarNames[@]}
  for (( i=0; i<$size; i++ )); do 
    if [ ${defaultVarNames[$i]} != $varName ]; then
      echo ${defaultVarNames[$i]}"="${defaultValues[$i]} >> $configName 
    else
      echo $varName"="$varValue >> $configName
    fi
  done
  addLineWithBestTheta $varName $varValue $configName 
  echo "scenario="$scenarioFileName >> $configName
}

# Peform experiments from given paramenters.
# Argument 1: number of experiments for each variable value.
# Argument 2: name of the variable being experimented.
# Argument 3: minimum value of the variable.
# Argument 4: maximum value of the variable.
# Argument 5: increment in variable.
# 
doExperiments(){
  if [ $# -ne 5 ]; then
    echo "Wrong number of parameter for doExperiments. " $# "are given."
    echo "Parameters were: " $@
    return 1
  fi
  local nTests=$1
  local varName=$2
  local minValue=$3
  local maxValue=$4
  local varInc=$5
  varValue=$minValue

  echo "Log for $@" &>> $folderConf/$experimentsLog
  # The configuration file of test.sh is random too.
  configFile="configExperiment$RANDOM.ini"
  while [ -f "$folderConf/$configFile" ]; do configFile="configExperiment$RANDOM.ini"; done
  while [ `echo "$varValue $maxValue" | awk '{printf "%i", $1 <= $2}'` != 0  ];
  do
    logsFolder=$folderConf/$varName\_$varValue
    local i
    for (( i=0; i<$nTests; i++ )); 
    do
      logsName=log\_$i
      if [ ! -e $logsFolder/$logsName ]; then
        makeConfigFile $varName $varValue $folderConf/$configFile "$logsFolder" $logsName
        echo "******* test for "$varName" = "$varValue"  ********" &>> $folderConf/$experimentsLog
        if [ ! -d $logsFolder ]; then
          mkdir $logsFolder;
        fi
        bash test.sh $folderConf/$configFile  &>> $folderConf/$experimentsLog

        #after it is created, if the file was  empty, it runs again at most RETRYTIMES
        vezes=0;
        while [ $vezes -lt $RETRYTIMES ] &&  [ ! -s $logsFolder/$logsName  ]  ; do
          echo "*** Running again. Log file named "$logsFolder/$logsName" was empty. ***" &>> $folderConf/$experimentsLog
          bash test.sh $folderConf/$configFile &>> $folderConf/$experimentsLog
          vezes=$((vezes+1));
        done;
      fi;
    done
    varValue=`echo "$varValue $varInc" | awk '{printf "%.3f", $1 + $2}'`
  done
}

mkdir -p $folderConf
# initalise log for this script
cat /dev/null > $folderConf/$experimentsLog
sizeVars=${#varsExp[@]}
for (( i=0; i<$sizeVars; i++ )); do
  if [ ! -z $samples ]; then
    varInc=`echo "${maxExp[$i]} ${minExp[$i]} ${samples[$i]}" | awk '{printf "%.3f", ($1-$2)/($3 - 1) }'`
  else
    varInc=${increment[$i]}
  fi
  doExperiments $nTests ${varsExp[$i]} ${minExp[$i]} ${maxExp[$i]} $varInc 
done

