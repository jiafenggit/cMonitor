#!/bin/bash

function get_io_dev_tps()
{
	dev_name=$1
	reply=`iostat -d -k | grep -w $dev_name | awk '{print $2}'`
	echo $reply
}

get_io_dev_tps sda