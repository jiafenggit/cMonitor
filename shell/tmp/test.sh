#!/bin/bash

function get_disk_dev_inodes()
{
	dev_name=$1
	reply=`df -Ti | grep $dev_name | awk '{print $3}'`
	echo 2'|'$reply
}

get_disk_dev_inodes sda1
