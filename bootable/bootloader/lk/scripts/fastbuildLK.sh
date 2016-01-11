#!/bin/bash 
MTK_TARGET_PROJECT=$1
LK_PROJECT=${MTK_TARGET_PROJECT/full_/}
LK_PROJECT=${LK_PROJECT/-eng/}
if [ $# -eq 0 ]; then
#lunch $FULL_PROJECT
        echo "plz assign project name!!"
        echo "e.g.: fastbuildLK.sh full_k6795v1_64-eng"
        exit 1
else
        source build/envsetup.sh 
        source mbldenv.sh 
        lunch $MTK_TARGET_PROJECT
fi
mosesq make -j24 -C bootable/bootloader/lk MTK_TARGET_PROJECT=$MTK_TARGET_PROJECT BOOTLOADER_OUT=../../../out/target/product/$LK_PROJECT/obj/BOOTLOADER_OBJ/ ROOTDIR=bootable/bootloader/lk/ $LK_PROJECT 2>&1 | tee lk.log
cp out/target/product/$LK_PROJECT/obj/BOOTLOADER_OBJ/build-$LK_PROJECT/lk.bin out/target/product/$LK_PROJECT/

