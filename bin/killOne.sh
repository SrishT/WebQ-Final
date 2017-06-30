#!/bin/bash


if [ $# -ne 2 ];
then
    echo "input username ./killOne.sh <username> <ip>"
    exit
fi

cd ~/WebQ/bin
source ips.sh
printf "`date`" | tee -a $log_file

#{{{ kill all components
printf " %d\n%s%43s\n" $? $marker "Killing all components" | tee -a $log_file

mc="$2"

echo "in killone, mc considered is: $mc"


for machine in $mc
do
    # redirect stderr to stdout so that we can log it 
    # ssh root@$machine "killall java" 2>&1 | tee -a $log_file > /dev/null
    ssh root@$machine "killall MATLAB" 2>&1 | tee -a $log_file > /dev/null
    ssh root@$machine "killall apache2" 2>&1 | tee -a $log_file > /dev/null
done
# ssh root@${tokencheck[$1]} "killall lighttpd" 2>&1  | tee -a $log_file > /dev/null
# }}}