#!/bin/sh
DEV_ID=$1
DEV_TYPE=$2
WFO_ENABLE=$3

if [ $DEV_TYPE -eq 0 ]; then
     ##Atheros  11ac
     cd /rboot/wfo_atheros_11AC
     if [ "$WFO_ENABLE" == "1" ]; then
       ./wfo_start.sh  
     elif [ "$WFO_ENABLE" == "14" ]; then
       /rboot/rboot ar988x_pe0.bin /rboot/ipsec/ipsec_pe1.bin
       #/rboot/rboot ar988x_pe0.bin

       sleep 2
       echo "enabling 11AC WFO..."
       echo 1 > /proc/driver/cs752x/wfo/wifi_offload_enable
       sh ath_11ac_ap.sh $DEV_ID 2
     else
       ./ath_11ac_ap.sh $DEV_ID
        
     fi     
     exit 0
fi

if [ $DEV_TYPE -eq 1 ]; then
     ##Atheros  11n
     cd /rboot/wfo_atheros_11AC
     if [ "$WFO_ENABLE" == "1" ]; then
       	/rboot/rboot ar988x_pe0.bin ar9580_pe1.bin 
	sleep 2
	echo "enabling 11n WFO..."
	echo 1 > /proc/driver/cs752x/wfo/wifi_offload_enable 
	./ath_11n_ap.sh $DEV_ID 1
     else
       ./ath_11n_ap.sh $DEV_ID
     fi     
     exit 0
fi
