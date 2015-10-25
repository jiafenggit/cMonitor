#!/bin/bash

function get_io_dev_write_sec()
{
	dev_name=$1
	reply=`iostat -d -k | grep -w $dev_name | awk '{print $4}'`
	echo $reply
}

get_io_dev_write_sec sda