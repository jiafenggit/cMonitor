#!/bin/bash
function get_service_status()
{
	service_name=$1
	reply=`pstree -ap  | grep $service_name | grep -v grep | grep -v $0| grep -v vi| grep -v { | awk -F ',' '{print $2}' | awk '{print $1}' `
	if [ "-$reply" == "-" ]
	then 
		return 0
	else
		return 1
	fi
}
get_service_status 'mongod'
echo $?