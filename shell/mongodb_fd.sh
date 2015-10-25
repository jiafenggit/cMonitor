#!/bin/bash

function get_service_pid()
{
	service_name=$1
	reply=`pstree -ap  | grep -w $service_name | grep -v grep | grep -v $0| grep -v vi| grep -v { | awk -F ',' '{print $2}' | awk '{print $1}' `
	echo $reply
}


function get_service_fd()
{
	service_name=$1
	service_pid=`get_service_pid $service_name`
	if [ "-$service_pid" == "-" ]
	then 
		echo -1'|'NULL
		exit 0
	fi
	service_fd=`echo cuitlab | sudo -S ls /proc/$service_pid/fd | wc -l | awk -F ':' '{print $NF}'`
	echo $2'|'2'|'$service_fd
}
get_service_fd 'mongod' 'mongod_fd'