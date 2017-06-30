#!/bin/bash

#run from proxy1 server(same as capacity estimator)
cd ~/WebQ/bin
source ~/WebQ/bin/ips.sh

# get command line arguments {{{
if [ $# -ne 2 ];
then
    echo "input username ./redeployCon.sh <username> <ip>"
    exit
else
    username=$1
    ip=$2
    echo "username : ${username}   ip : ${ip}"
fi
# }}}

# {{{ put the number of tokengens

echo "log file = `pwd` $log_file"
# }}}

# The following is done in the following code
# stop server, make the proxy1 code, and copy it in /usr/lib/cgi-bin
# then start the server

# killall.sh will stop all tokenGens and TokenChecks
bash ~/WebQ/bin/killOne.sh $username $ip                 #kill all components

# cleaning up all the log files in con#{{{
for machine in $ip
do
    printf " %d\n%s%43s" $? $marker "Cleaning up the log files at $machine" | tee -a $log_file
    ssh root@$machine "cat /dev/null > /home/${username}/WebQ/Controller/src/proxy1.log"
    ssh root@$machine "cat /dev/null > /usr/lib/cgi-bin/proxy1.log"
#     ssh root@$machine "cat /dev/null > /home/${uesrname}/WebQ/CapacityEstimator/javapersecond.log"
#     ssh root@$machine "cat /dev/null > /home/${username}/WebQ/CapacityEstimator/javadebug.log"
done
#}}}

# rebuild and copy controller(proxy1) to cgi-bin folder#{{{
for machine in $ip
do
    printf " %d\n%s%43s" $? $marker "remade proxy1 -> php @ $machine" | tee -a $log_file
    sshpass -p "webq" ssh root@$machine \
        "cd /home/${username}/WebQ/Controller/src; ./make_script.sh" >> $log_file
done
#}}}

#{{{ start apache and hence the proxy and hit url
for machine in $ip
do
    printf " %d\n%s%43s" $? $marker "start the apache server at $machine" | tee -a $log_file
    ssh root@$machine "service apache2 start" >> $log_file
done
# we need a separate for loop here so that we can hit the machines
# in parellel at the same time
for machine in $ip
do
    # #hit the URL once
    printf " %d\n%s%43s" $? $marker "Hitting the URL once `grep $machine /etc/hosts`" | tee -a $log_file
    lynx -dump http://$machine:8000/proxy1\?limit\=100 >> $log_file &
done
#}}}

echo "################# REDEPLOYMENT ATTEMPT FINISHED ##################";