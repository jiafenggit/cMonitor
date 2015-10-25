#!/bin/bash

function get_tcp_port_status()
{
	port=$1
	reply=`echo cuitlab | sudo -S netstat -ant| awk '{print $4}' | grep -w $port | wc -l`
	echo $2'|'$reply'|'$reply
}

get_tcp_port_status 8080 "8080_status"s