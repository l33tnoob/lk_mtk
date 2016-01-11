#!/system/bin/sh
# This file implements a user-space handler for compressing
# core files and limiting the number of core files.
BASE=/data/core
MAX_NUM_SYSTEM_CORE_FILES=3
MAX_NUM_APP_CORE_FILES=3
LIMIT_TIME=600 # 10 minutes

pid=$1          # process id
uid=$2          # user id
ts=$3           # time stamp
rlimit_core=$4  # core file size soft resource limit

# The following section is used to determine whether it needs to
# generate core file based on its rlimit value of core.
#{
if [ -z "$rlimit_core" -o "0" = "$rlimit_core" ]; then # 0 means disable coredump
      exit 1
fi
#}

/system/bin/mkdir $BASE #create core directory
chmod $BASE 0766

# classify the proc
fpostfix="app"
if [ $rlimit_core -eq 4294967294 ]; then # 0xfffffffe is system_server
    fpostfix="system"
fi

fname=$ts.$pid.$uid-$fpostfix.core.gz # output file name
OUTPUT=$BASE/$fname # absolute file name
LOG=$BASE/$ts.$pid.$uid.tmp.log # Log path
/system/bin/rm $BASE/*.tmp.log

# create the empty log
/system/bin/touch $LOG
# Get proc name from /proc/$pid/cmdline
#{
pname=""
fcmdline=/proc/$pid/cmdline
isExist=(`/system/bin/ls /proc/$pid`)
if [ -n $isExist ]; then
  read pname < $fcmdline
else
  echo "no /proc/$pid/" >> $LOG
fi
#}

# The first line of LOG MUST BE proc name
echo "$pname" >> $LOG
echo "Generating core files of: $pname ..." >> $LOG

# Check last dump time
#{
if [ "$fpostfix" != "system" ]; then
    loglist=(`/system/bin/ls $BASE/*core.gz.log`)
    if [ -n $loglist -a ${#loglist[@]} -gt 0 ]; then
        lastIndex=$((${#loglist[@]} - 1))
        lastlog=${loglist[$lastIndex]}
        echo "Check lastlog $lastlog" >> $LOG
        logfile=${lastlog#$BASE/*}
        lastTime=${logfile%%.*}

        if [ $(($ts - $lastTime)) -lt $LIMIT_TIME ]; then 
            echo "Skip! It had done core dump within $LIMIT_TIME seconds ago." >> $LOG
            echo "$ts $pid $uid $rlimit_core $pname (skip dump)" >> $BASE/core_history
            exit 1
        fi
    fi
fi
#}

# file_control: remove older files to keep the file number of current
#     process type less then MAX_NUM_CORE_FILES.
#{
if [ "$fpostfix" = "app" ]; then
    loglist=(`/system/bin/ls $BASE/*-app.core.gz`) # filter app core logs
    if [ -n $loglist ]; then
        for logfile in $loglist
        do
            read _pname < $logfile.log
            if [ "$_pname" = "$pname" ]; then
                echo "delete core files of the same APP" >> $LOG
                echo "delete $logfile.log" >> $LOG
                echo "delete $logfile" >> $LOG
                /system/bin/rm $logfile.log
                /system/bin/rm $logfile
                break
            fi
        done
    fi
fi

# Rename the log file
#{
/system/bin/mv $LOG $BASE/$fname.log
LOG=$BASE/$fname.log
#}

# Check whether exceed the max file num
maxFiles=$MAX_NUM_APP_CORE_FILES

filelist=(`/system/bin/ls $BASE/*-$fpostfix.core.gz`)
if [ -n $filelist ]; then
    num_to_delete=$((${#filelist[@]} - $maxFiles + 1)) # reserving a quota for later
    echo "FileControl num_to_delete $num_to_delete" >> $LOG
    if [ $num_to_delete -gt 0 ]; then
        index=0
        while [ $index -lt $num_to_delete ];
        do
            delete_fname=${filelist[$index]}
            if [ -n "$delete_fname" ]; then
                echo "FileControl - $index) delete '$delete_fname'" >> $LOG
                /system/bin/rm $delete_fname
                /system/bin/rm $delete_fname.log
            fi
            index=$(($index + 1))
        done
    fi
fi
#}

# Delete the redundant core.dump.maps.PID.TID file
cd $BASE
filelist=(`/system/bin/ls *.core.gz`)
if [ -n $filelist ]; then
    for flog in "${filelist[@]}"
    do
        #echo "$flog"
        file_tmp=${flog#*.}
        file_pid=${file_tmp%%.*}
        echo "pid $file_pid"
        target=(`/system/bin/ls core.dump.maps.$file_pid.*`)
        if [ -n $target ];
        then
           /system/bin/mv $target "s.$target" # prefix s. marked
           echo "rename s.$target"
        fi
    done
fi
/system/bin/rm core.dump.maps.*
filelist=(`/system/bin/ls s.core.dump.maps.*`)
if [ -n $filelist ]; then
    for flog in "${filelist[@]}"
    do
        /system/bin/mv $flog ${flog#*.} # recover the prefix mark
        echo "rename ${flog#*.}"
    done
fi
cd /
#}

echo "Generating core files..." >> $LOG
echo "---- $ts ----" >> $LOG
echo "pname = $pname" >>$LOG
echo "pid = $pid" >> $LOG
echo "uid = $uid" >> $LOG
echo "rlimit core = $rlimit_core" >> $LOG
/system/bin/gzip -1 > $OUTPUT
echo "Done..." >> $LOG
echo "$ts $pid $uid $rlimit_core $pname" >> $BASE/core_history

