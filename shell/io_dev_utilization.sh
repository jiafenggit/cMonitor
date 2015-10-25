#!/bin/bash

function get_io_dev_utilization()
{
	dev_name=$1
	reply=`iostat -x -d | grep -w $dev_name | awk '{print $NF}'`
	echo $2'|'3'|'$reply
}

get_io_dev_utilization sda 'sda_utilization'