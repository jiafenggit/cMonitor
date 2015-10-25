#!/bin/bash

function get_io_dev_read_sec()
{
	dev_name=$1
	reply=`iostat -d -k | grep -w $dev_name | awk '{print $3}'`
	echo $reply
}

get_io_dev_read_sec sda