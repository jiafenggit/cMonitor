#!/bin/bash

function get_net_dev_speeed()
{
	dev_name=$1
	reply=`echo cuitlab | sudo -S ethtool $dev_name | grep Speed | awk -F ':' '{print $2}'`
	echo 4'|'$reply
}

get_net_dev_speeed eth0