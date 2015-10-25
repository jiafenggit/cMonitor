#!/bin/bash

function get_port_connections()
{
	port=$1
	reply=`echo cuitlab | sudo -S netstat -an| grep ESTABLISHED | awk '{print $4}' | grep -w $port | wc -l`
	echo $reply
}

get_port_connections 8080