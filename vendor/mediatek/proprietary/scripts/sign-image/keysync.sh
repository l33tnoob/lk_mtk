#!/bin/bash

#get_build_var is defined in build/envsetup.sh
#It is used to get variable value in makefile.

source ./build/envsetup.sh
BASE_PROJECT=$(get_build_var MTK_BASE_PROJECT)
PRODUCT_OUT=$(get_build_var PRODUCT_OUT)
COMPANY=mediatek
SIG_INI_PATH=${PRODUCT_OUT}/sig_ini
PUBK_HDR_PATH=${PRODUCT_OUT}/pubk_hdr
TEE_PUBK_DEFAULT_PATH=vendor/${COMPANY}/proprietary/trustzone/cfg/
TEE_PUBK_CUSTOM_PATH=bootable/bootloader/preloader/custom/${BASE_PROJECT}/
VERIFIED_BOOT_PUBK_HDR_PL_PATH=bootable/bootloader/preloader/custom/${BASE_PROJECT}/inc
VERIFIED_BOOT_PUBK_HDR_LK_PATH=bootable/bootloader/lk/target/${BASE_PROJECT}/inc

echo -e "BASE_PROJECT=$BASE_PROJECT"
echo -e "PRODUCT_OUT=$PRODUCT_OUT"

rm -rf ${SIG_INI_PATH}
mkdir -p ${SIG_INI_PATH}
rm -rf ${PUBK_HDR_PATH}
mkdir -p ${PUBK_HDR_PATH}

#######################
# copy INI files
#######################

#Preloader Key INI
if [ ! -f vendor/${COMPANY}/proprietary/custom/${BASE_PROJECT}/security/chip_config/s/key/CHIP_TEST_KEY.ini ]; then
    echo "Preloader Key INI does not exist."
    exit 1
fi
cp vendor/${COMPANY}/proprietary/custom/${BASE_PROJECT}/security/chip_config/s/key/CHIP_TEST_KEY.ini ${SIG_INI_PATH}/

#Verified Boot Key INI
if [ ! -f vendor/${COMPANY}/proprietary/custom/${BASE_PROJECT}/security/image_auth/VERIFIED_BOOT_IMG_AUTH_KEY.ini ]; then
    echo "Verified Boot Key INI does not exist."
    exit 1
fi
cp vendor/${COMPANY}/proprietary/custom/${BASE_PROJECT}/security/image_auth/VERIFIED_BOOT_IMG_AUTH_KEY.ini ${SIG_INI_PATH}/

#TEE Key INI
if [ -f ${TEE_PUBK_CUSTOM_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini ]; then
    echo "TEE uses custom key"
    cp ${TEE_PUBK_CUSTOM_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini ${SIG_INI_PATH}/
else
    echo "TEE uses default key"
    cp ${TEE_PUBK_DEFAULT_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini ${SIG_INI_PATH}/
fi

#######################
# copy PUBK header files
#######################

#Verified Boot Public Key Header in Preloader
if [ ! -f ${VERIFIED_BOOT_PUBK_HDR_PL_PATH}/cust_sec_ctrl.h ]; then
    echo "Preloader verified boot public key header does not exist"
    exit 1
fi
cp ${VERIFIED_BOOT_PUBK_HDR_PL_PATH}/cust_sec_ctrl.h ${PUBK_HDR_PATH}/

#Verified Boot Public Key in LK 
if [ ! -f ${VERIFIED_BOOT_PUBK_HDR_LK_PATH}/oemkey.h ]; then
    echo "LK verified boot public key header does not exist"
    exit 1
fi
cp ${VERIFIED_BOOT_PUBK_HDR_LK_PATH}/oemkey.h ${PUBK_HDR_PATH}/

###################################
# check whether autosync is enabled 
###################################
if grep -q "AUTOSYNC=yes" ${SIG_INI_PATH}/CHIP_TEST_KEY.ini; then
    echo "AUTOSYNC=yes"
    UPDATE_FILES="yes"
else
    echo "AUTOSYNC=no"
    UPDATE_FILES="no"
fi

#Check whether keys are synchronized, if not, sync them.
#Due to ATBC rule, we don't change customization files here.

# note that rsa_e is always 65537(0x10001), so we don't process it here.
echo "start synchronizing key configurations"

#sync rsa_d
grep private_key_d ${SIG_INI_PATH}/CHIP_TEST_KEY.ini > ${SIG_INI_PATH}/rsa_d
cp ${SIG_INI_PATH}/rsa_d ${SIG_INI_PATH}/verified_rsa_d

sed -i 's/ //g' ${SIG_INI_PATH}/verified_rsa_d
sed -i 's/\"//g' ${SIG_INI_PATH}/verified_rsa_d
sed -i 's/private_key_d/AUTH_PARAM_D/' ${SIG_INI_PATH}/verified_rsa_d
sed -i 's/=/=0x/' ${SIG_INI_PATH}/verified_rsa_d
sed -i 's/=/ = /' ${SIG_INI_PATH}/verified_rsa_d

sed -i '/AUTH_PARAM_D/d' ${SIG_INI_PATH}/VERIFIED_BOOT_IMG_AUTH_KEY.ini
cat ${SIG_INI_PATH}/verified_rsa_d >> ${SIG_INI_PATH}/VERIFIED_BOOT_IMG_AUTH_KEY.ini

sed -i '/AUTH_PARAM_D/d' ${SIG_INI_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini
cat ${SIG_INI_PATH}/verified_rsa_d >> ${SIG_INI_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini

#sync rsa_n
grep private_key_n ${SIG_INI_PATH}/CHIP_TEST_KEY.ini > ${SIG_INI_PATH}/rsa_n
cp ${SIG_INI_PATH}/rsa_n ${SIG_INI_PATH}/verified_rsa_n

sed -i 's/ //g' ${SIG_INI_PATH}/verified_rsa_n
sed -i 's/\"//g' ${SIG_INI_PATH}/verified_rsa_n
sed -i 's/private_key_n/AUTH_PARAM_N/' ${SIG_INI_PATH}/verified_rsa_n
sed -i 's/=/=0x/' ${SIG_INI_PATH}/verified_rsa_n
sed -i 's/=/ = /' ${SIG_INI_PATH}/verified_rsa_n

sed -i '/AUTH_PARAM_N/d' ${SIG_INI_PATH}/VERIFIED_BOOT_IMG_AUTH_KEY.ini
cat ${SIG_INI_PATH}/verified_rsa_n >> ${SIG_INI_PATH}/VERIFIED_BOOT_IMG_AUTH_KEY.ini

sed -i '/AUTH_PARAM_N/d' ${SIG_INI_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini
cat ${SIG_INI_PATH}/verified_rsa_n >> ${SIG_INI_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini

#sync to public key headers

echo "start synchronizing public key headers"
#process public key file
cp ${SIG_INI_PATH}/rsa_n ${PUBK_HDR_PATH}/pubk
sed -i 's/private_key_n//' ${PUBK_HDR_PATH}/pubk
sed -i 's/ //g' ${PUBK_HDR_PATH}/pubk
sed -i 's/\"//g' ${PUBK_HDR_PATH}/pubk
sed -i 's/=//g' ${PUBK_HDR_PATH}/pubk
sed -i 's/../, 0x&/g' ${PUBK_HDR_PATH}/pubk
sed -i 's/, //' ${PUBK_HDR_PATH}/pubk

#process cust_sec_ctrl.h
#set OEM_PUBK tag and MTEE_IMG_VFY_PUBK tag for key injection location mark
sed -i 's/\#define OEM_PUBK /\#define OEM_PUBK_TAG\'$'\n\#define OEM_PUBK /g' ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i 's/\#define MTEE_IMG_VFY_PUBK /\#define MTEE_IMG_VFY_PUBK_TAG\'$'\n\#define MTEE_IMG_VFY_PUBK /g' ${PUBK_HDR_PATH}/cust_sec_ctrl.h

#remove original key
sed -i ':x; /\\$/ { N; s/\\\n//; tx }' ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i '/\#define OEM_PUBK /d' ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i '/\#define MTEE_IMG_VFY_PUBK /d' ${PUBK_HDR_PATH}/cust_sec_ctrl.h

#inject preloader key
sed -i "/\#define OEM_PUBK_TAG/r ${PUBK_HDR_PATH}/pubk" ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i "/\#define MTEE_IMG_VFY_PUBK_TAG/r ${PUBK_HDR_PATH}/pubk" ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i '/\#define OEM_PUBK_TAG/{N;s/\#define OEM_PUBK_TAG[\r\n]/\#define OEM_PUBK /}' ${PUBK_HDR_PATH}/cust_sec_ctrl.h
sed -i '/\#define MTEE_IMG_VFY_PUBK_TAG/{N;s/\#define MTEE_IMG_VFY_PUBK_TAG[\r\n]/\#define MTEE_IMG_VFY_PUBK /}' ${PUBK_HDR_PATH}/cust_sec_ctrl.h

#process oemkey.h
#set OEM_PUBK tag for key injection location mark
sed -i 's/\#define OEM_PUBK /\#define OEM_PUBK_TAG\'$'\n\#define OEM_PUBK /g' ${PUBK_HDR_PATH}/oemkey.h
sed -i ':x; /\\$/ { N; s/\\\n//; tx }' ${PUBK_HDR_PATH}/oemkey.h
sed -i '/\#define OEM_PUBK /d' ${PUBK_HDR_PATH}/oemkey.h
sed -i "/\#define OEM_PUBK_TAG/r ${PUBK_HDR_PATH}/pubk" ${PUBK_HDR_PATH}/oemkey.h
sed -i '/\#define OEM_PUBK_TAG/{N;s/\#define OEM_PUBK_TAG[\r\n]/\#define OEM_PUBK /}' ${PUBK_HDR_PATH}/oemkey.h

if [ ${UPDATE_FILES} == "yes" ]; then
    echo "trying to update files"
    #copy processed files back to codebase
    cp ${SIG_INI_PATH}/VERIFIED_BOOT_IMG_AUTH_KEY.ini vendor/${COMPANY}/proprietary/custom/${BASE_PROJECT}/security/image_auth/VERIFIED_BOOT_IMG_AUTH_KEY.ini 
    echo "update TEE custom key"
    cp ${SIG_INI_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini ${TEE_PUBK_CUSTOM_PATH}/TRUSTZONE_IMG_PROTECT_CFG.ini
    cp ${PUBK_HDR_PATH}/cust_sec_ctrl.h ${VERIFIED_BOOT_PUBK_HDR_PL_PATH}/cust_sec_ctrl.h
    cp ${PUBK_HDR_PATH}/oemkey.h ${VERIFIED_BOOT_PUBK_HDR_LK_PATH}/oemkey.h
else
    echo "please update files manually"
fi

echo "done"

