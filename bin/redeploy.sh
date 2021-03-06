#!/bin/bash

#run from proxy1 server(same as capacity estimator)
cd ~/WebQ/bin
source ~/WebQ/bin/ips.sh

# get command line arguments {{{
if [ -z "$1" ];
then
    echo "input username ./redeploy.sh <username> <no_of_tokengen>"
    exit
else
    username=$1
    echo "username : ${username}"
fi
# }}}

# {{{ put the number of tokengens
if [ -z "$2" ];
then
    gens="${tokengen1}"
    con="${controller2} ${controller1}"
else
    con="${controller2} ${controller1}"
    case $2 in
        1)
            gens="${tokengen1}"
            ;;
        2)
            gens="${tokengen2} ${tokengen1}"
            ;;
        3)
            gens="${tokengen3} ${tokengen2} ${tokengen1}"
            ;;
        4)
            gens="${tokengen4} ${tokengen3} ${tokengen2} ${tokengen1}"
            ;;
        *)
            gens="${tokengen4} ${tokengen3} ${tokengen2} ${tokengen1}"
            ;;
    esac
fi
mc="${controller2} ${controller1} $gens"
echo "gens = $gens"
echo "mc = $mc"
echo "log file = `pwd` $log_file"
export gens
export mc
# }}}

# The following is done in the following code
# stop server, make the proxy1 code, and copy it in /usr/lib/cgi-bin
# then start the server

# killall.sh will stop all tokenGens and TokenChecks
bash ~/WebQ/bin/killall.sh $username                  #kill all components

# cleaning up all the log files in con#{{{
for machine in $con
do
    printf " %d\n%s%43s" $? $marker "Cleaning up the log files at $machine" | tee -a $log_file
    ssh root@$machine "cat /dev/null > /home/${username}/WebQ/Controller/src/proxy1.log"
    ssh root@$machine "cat /dev/null > /usr/lib/cgi-bin/proxy1.log"
    ssh root@$machine "echo \"0\" > /usr/lib/cgi-bin/IdStore"
#     ssh root@$machine "cat /dev/null > /home/${uesrname}/WebQ/CapacityEstimator/javapersecond.log"
#     ssh root@$machine "cat /dev/null > /home/${username}/WebQ/CapacityEstimator/javadebug.log"
done
#}}}

# cleaning up all the log files#{{{
for machine in $gens
do
    printf " %d\n%s%43s" $? $marker "Cleaning up the log files at $machine" | tee -a $log_file
    ssh root@$machine "cat /dev/null > /home/${username}/WebQ/TokenGen/src/proxy1.log"
    ssh root@$machine "cat /dev/null > /usr/lib/cgi-bin/proxy1.log"
    ssh root@$machine "echo \"0\" > /usr/lib/cgi-bin/IdStore"
#     ssh root@$machine "cat /dev/null > /home/${uesrname}/WebQ/CapacityEstimator/javapersecond.log"
#     ssh root@$machine "cat /dev/null > /home/${username}/WebQ/CapacityEstimator/javadebug.log"
done
#}}}

# rebuild and copy controller(proxy1) to cgi-bin folder#{{{
for machine in $con
do
    printf " %d\n%s%43s" $? $marker "remade proxy1 -> php @ $machine" | tee -a $log_file
    sshpass -p "webq" ssh root@$machine \
        "cd /home/${username}/WebQ/Controller/src; ./make_script.sh" >> $log_file
done
#}}}

# rebuild and copy tokengen(proxy1) to cgi-bin folder#{{{
for machine in $gens
do
    printf " %d\n%s%43s" $? $marker "remade proxy1 -> php @ $machine" | tee -a $log_file
    sshpass -p "webq" ssh root@$machine \
        "cd /home/${username}/WebQ/TokenGen/src; ./make_script.sh" >> $log_file
done
#}}}
 
# {{{  now wait for some time
# as apache need some time to recover !! :P
# if "SOME" thrid parameter is passed do not wait
if [ -z "$3" ];
then
    # empty statement | do nothing
    :
else
    # unset
    printf " %d\n%s%43s" $? $marker "wait for some time for apache to recover" | tee -a $log_file
    ttw=40  #time to wait
    printf "\n"
    for i in `seq $ttw -1 1`
    do
        printf "%3d" $i
        sleep 1
        if (( i%10 == 0)) ; then
            printf "\n"
        fi
    done
    printf "\n"
fi
# }}}

#{{{ start apache and hence the proxy and hit url
for machine in $mc
do
    printf " %d\n%s%43s" $? $marker "start the apache server at $machine" | tee -a $log_file
    ssh root@$machine "service apache2 start" >> $log_file
done
# we need a separate for loop here so that we can hit the machines
# in parellel at the same time
for machine in $mc
do
    # #hit the URL once
    printf " %d\n%s%43s" $? $marker "Hitting the URL once `grep $machine /etc/hosts`" | tee -a $log_file
    lynx -dump http://$machine:8000/proxy1\?limit\=100 >> $log_file &
done
#}}}

#start java code {{{
printf " %d\n%s%43s" $? $marker "Starting the java code" | tee -a $log_file
ssh root@${capacityEstimator} "cd /home/${username}/WebQ/CapacityEstimator;bash makefile.sh"
ssh root@${capacityEstimator} "cd /home/${username}/WebQ/CapacityEstimator;bash run.sh;"
printf " %d\n%s%43s" $? $marker "foo.out" | tee -a $log_file
ssh root@${capacityEstimator} "cat /home/${username}/WebQ/CapacityEstimator/foo.out;"| tee -a $log_file
printf " %d\n%s%43s" $? $marker "foo.err" | tee -a $log_file
ssh root@${capacityEstimator} "cat /home/${username}/WebQ/CapacityEstimator/foo.err;"| tee -a $log_file

#}}}

# {{{ start lighttpd
sleep 4
printf " %d\n%s%46s\n" $? $marker "Starting the lighttpd server" | tee -a $log_file
ssh root@${tokencheck} "bash /home/${username}/WebQ/TokenCheck/run.sh ${username}"

echo "################# REDEPLOYMENT ATTEMPT FINISHED ##################";

echo "$tokengen1:8000/proxy1?limit=100"
echo "$tokengen2:8000/proxy1?limit=100"

# }}}
