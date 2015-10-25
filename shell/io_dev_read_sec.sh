#!/bin/bash

function get_io_dev_read_sec()
{
	dev_name=$1
	reply=`iostat -d -k | grep -w $dev_name | awk '{print $3}'`
	echo $2'|'3'|'$reply
}

get_io_dev_read_sec sda 'sda_read_sec'