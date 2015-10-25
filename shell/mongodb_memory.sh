#!/bin/bash

function get_service_pid()
{
	service_name=$1
	reply=`pstree -ap  | grep $service_name | grep -v grep | grep -v $0| grep -v vi| grep -v { | awk -F ',' '{print $2}' | awk '{print $1}' `
	echo $reply
}


function get_service_mem()
{
	service_name=$1
	service_pid=`get_service_pid $service_name`
	if [ "-$service_pid" == "-" ]
	then 
		echo -1'|'NULL
		exit 0
	fi
	service_mem=`ps -p $service_pid -o pmem | grep -v MEM | awk '{print $1}'`
	echo $2'|'2'|'$service_mem
}
get_service_mem 'mongod' 'mongod_mem'