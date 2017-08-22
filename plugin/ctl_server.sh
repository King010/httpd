#!/bin/bash 

#注该脚本在output目录下运行
ROOT_PATH=$(pwd)
LIB=$ROOT_PATH/lib/lib
BIN=$ROOT_PATH/httpd
CONF=$ROOT_PATH/conf/server.conf

id=''
proc=$(basename $0)

function usage()
{
	printf "Usage :%s [start(-s)] [ stop(-t) ] [ restart(-r)] [ status(-ss)]\n" "$proc"
}

function server_status()
{
	#pidof 查看服务
	server_bin=$(basename $BIN)
	id=$(pidof $server_bin)
	
	if [ ! -z "$id" ];then
		return 0
	else
		#id为空
		return 1 
	fi
}
function service_start()
{
	if ! server_status ;then
		ip=$(awk -F: '/^IP/{print $NF}' $CONF)
		port=$(awk -F: '/^PORT/{print $NF}' $CONF)
		$BIN $ip $port
		echo "start...done"
	else
		echo "server is already running : $id"
	fi
}
function service_stop()
{
	if  server_status ;then
		kill -9 $id
		echo "stop ....done"
	else
		echo "server is not running,please running first!"
	fi
}
function service_restart()
{
	if server_status ;then
		service_stop
	fi
	service_start
}
function service_status()
{
	if server_status;then
		echo "server is running: $id"
	else
		echo "server is not running"
	fi
}

#先导入库
if [ -z "$LD_LIBRARY_PATH" ];then
	export "LD_LIBRARY_PATH=$LIB"
fi
#判断是否用对，参数是否为0
if [ $# -eq 0 ];then
	usage
	exit 1
fi

case $1 in
	start | -s)
		service_start
		;;
	stop | -t)
		service_stop
		;;
	restart | -r)
		service_restart
		;;
	status | -ss)
		service_status	

esac
