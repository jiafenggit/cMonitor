#!/bin/bash

function get_disk_dev_size()
{
	dev_name=$1
	reply=`df | grep $dev_name | awk '{print $2}'`
	echo $reply
}

get_disk_dev_size sda1