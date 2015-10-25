#!/bin/bash

function get_io_dev_await()
{
	dev_name=$1
	reply=`iostat -d -k  -x | grep -w $dev_name | awk '{print $10}'`
	echo 3'|'$reply
}

get_io_dev_await sda