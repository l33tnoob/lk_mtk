#!/system/bin/sh

sleep 60
act=`cat /sys/power/pnpmgr/apps/activity_trigger`

if [ "$act" = "booting" ]; then
    echo "fota" > /sys/power/pnpmgr/apps/activity_trigger
fi

