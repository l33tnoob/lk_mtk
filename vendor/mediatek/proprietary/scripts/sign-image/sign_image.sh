#!/bin/bash

#######################################
# Initialize variables
#######################################
D_CURR=`pwd`

#######################################
# Specify temporarily folder path
#######################################

function usage() {

	#######################################
	# Dump usage howto
	#######################################

	echo "sign image ..."
	echo "Please source ./build/envsetup.sh and ./mbldenv.sh first, and select correct project"
	echo "Command: ./sign_image.sh <BASE_PROJECT>"
}


#######################################
# Check arguments
#######################################
if [ "$1" == "" ]; then
	source ./build/envsetup.sh
	export MTK_BASE_PROJECT=$(get_build_var MTK_BASE_PROJECT)
	export OUT_DIR=$(get_build_var OUT_DIR)
	export PRODUCT_OUT=$(get_build_var PRODUCT_OUT)
else
	export MTK_BASE_PROJECT=$1
fi

echo -e "MTK_BASE_PROJECT=$MTK_BASE_PROJECT"

make -f ./vendor/mediatek/proprietary/scripts/sign-image/Android.mk

