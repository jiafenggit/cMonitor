#!/bin/bash

function get_service_pid()
{
	service_name=$1
	reply=`pstree -ap  | grep -w $service_name | grep -v grep | grep -v $0| grep -v vi| grep -v { | awk -F ',' '{print $2}' | awk '{print $1}' `
	echo $reply
}


function get_service_cpu()
{
	service_name=$1
	service_pid=`get_service_pid $service_name`
	if [ "-$service_pid" == "-" ]
	then 
		echo -1'|'NULL
		exit 0
	fi
	service_cpu=`ps -p $service_pid -o pcpu | grep -v CPU | awk '{print $1}'`
	echo $2'|'3'|'$service_cpu
}
get_service_cpu 'mongod' 'mongod_cpu'