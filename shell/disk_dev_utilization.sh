#!/bin/bash

function get_disk_dev_utilization()
{
	dev_name=$1
	reply=`df | grep $dev_name | awk '{uti_str = $5;  uti_len = length(uti_str);print substr(uti_str, 1, uti_len - 1)}' `
	echo $2'|'3'|'$reply
}
get_disk_dev_utilization sda1 'sda1_utilization'