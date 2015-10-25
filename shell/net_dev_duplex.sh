#!/bin/bash

function get_net_dev_deplex()
{
	dev_name=$1
	reply=`echo cuitlab | sudo -S ethtool $dev_name | grep Duplex | awk -F ':' '{print $2}'`
	echo $2'|'4'|'$reply
}

get_net_dev_deplex eth0 "eth0_deplex"