#
# HTC orporation Proprietary Rights Acknowledgment
#
# Copyright (C) 2012 HTC Corporation
#
# All Rights Reserved.
#
# The information contained in this work is the exclusive property of HTC Corporation
# ("HTC").  Only the user who is legally authorized by HTC ("Authorized User") has
# right to employ this work within the scope of this statement.  Nevertheless, the
# Authorized User shall not use this work for any purpose other than the purpose
# agreed by HTC.  Any and all addition or modification to this work shall be
# unconditionally granted back to HTC and such addition or modification shall be
# solely owned by HTC.  No right is granted under this statement, including but not
# limited to, distribution, reproduction, and transmission, except as otherwise
# provided in this statement.  Any other usage of this work shall be subject to the
# further written consent of HTC.

import errno
import os
import re
import zipfile
import glob
import shutil
import filecmp

import common

########################################################################################
#  Description:
#    This function is used to handle preload partition when /PRELOAD folder exist
#    in Target-files-package. The inputs are updater-script, source/target preload data
#    (all files under /PRELOAD), and ota package output.
#    1. This function should treat "patched" file as ADD because MASD package manager will
#    possiblely delete files under /preload, and updater won't find file to do patch.
#    2. ota tool will unmount all partition at tail of updater-script
########################################################################################
def PreloadHandler(script, source_data, target_data, output_zip):
  verbatim_preload_data = []
  for fn, tf in target_data.items():
    if fn in source_data.keys() and tf.sha1 == source_data[fn].sha1:
      continue
    else:
      # Add into add_list
      print "send", fn, "preload verbatim"
      tf.AddToZip(output_zip)
      verbatim_preload_data.append((fn, tf.size))

  script.Comment("Start to handle htc preload...")
  script.Mount("/preload", None)
  # Delete unnecessary files
  script.DeleteFiles(["/"+i for i in sorted(source_data)
                            if i not in target_data])
  # Add new or patched files
  for item in verbatim_preload_data:
    if not item[0].endswith("/"):
      script.UnpackPackageFile(item[0], "/"+item[0])

  script.SetPermissionsRecursive("/preload", 0, 0, 0771, 0644, None, None)

#####################################################################################
#  Description:
#    Sign the file, named by filename, with given key, sign_key.
#    Currently, this module is used for signing firmware.zip with hboot key.
#####################################################################################
def handleSignImageByBootloaderKey(filename, sign_key):
  newFilePath = str(filename) + ".new"
  command = "openssl dgst -binary -sha1 -sign " + str(sign_key) + " " + str(filename) + " > " + str(newFilePath)
  print command
  os.system(command)

  command = "cat " + str(filename) + " >> " + str(newFilePath)
  print command
  os.system(command)

  command = "mv " + str(newFilePath) + " " + str(filename)
  print command
  os.system(command)

#####################################################################################
#  Description:
#    Read CID list from android-info.txt and add edify script content in the beginning
#    of updater script to check if CID in device matches with OTA package's
#####################################################################################
def handleAssertCID(script, OPTIONS):
  # Extract android-info.txt from target files for CID parsing
  top_path = os.getcwd()
  path_android_info = os.path.join(top_path,"out/dist/android-info.txt")
  if os.path.exists(path_android_info):
    os.remove(path_android_info)

  shutil.copyfile(os.path.join(OPTIONS.target_tmp, "OTA/android-info.txt"), path_android_info)

  cid_flag = 0
  # [FIXME] Consider if it is necessary to check hboot version like this.
  # for filename in os.listdir(tmp_folder):
  #  #hboot version is a.bb.[0,1,2]ccc.nb
  #  res = re.match("^hboot.*[0-9]\.[0-9]+\.0[0-9]+.*",filename)
  #  if res != None:
  #    insertAssertCID(script, tmp_folder, False)
  #    cid_flag = 1
  #    break;

  cid_list = ["00000000","11111111","22222222","33333333","44444444","55555555","66666666","77777777","88888888","99999999"]
  f = open(path_android_info)

  for line in f:
    if line[0:7].lower() == 'cidnum:'.lower():
      cid = line.split(':')[1].replace(' ','').replace('\n','')
      print "Get CID from android-info.txt : " + cid
      cid_list.append(cid)

  f.close()
  script.AssertCid(cid_list);

#####################################################################################
#  Description:
#    Read MID list from android-info.txt and add edify script content in the beginning
#    of updater script to check if MID in device matches with OTA package's
#####################################################################################
def handleAssertMID(script, OPTIONS):
  # Extract android-info.txt from target files for MID parsing
  top_path = os.getcwd()
  path_android_info = os.path.join(top_path,"out/dist/android-info.txt")
  if os.path.exists(path_android_info):
    os.remove(path_android_info)

  shutil.copyfile(os.path.join(OPTIONS.target_tmp, "OTA/android-info.txt"), path_android_info)

  mid_list = []
  f = open(path_android_info)

  for line in f:
    if line[0:8].lower() == 'modelid:'.lower():
      #mid = line.replace('modelid: ','').replace('MODELID: ','').replace('\n','')
      mid = line.split(':')[1].replace(' ','').replace('\n','')
      print "Get MID from android-info.txt : " + mid
      mid_list.append(mid)

  f.close()
  script.AssertMid(mid_list);

#####################################################################################
#  Description:
#    An interface which handle all HTC post process features
#####################################################################################
def handlePostProcess(script, OPTIONS, output_zip):
  # Generate firmware.zip
  handlePackFirmware(script, OPTIONS, output_zip)

  # Create fotaBoot and copy it to /cache/recovery/last_fotaBoot if requested
  handleFotaBoot(script, OPTIONS, output_zip)

#####################################################################################
#  Description:
#    1. Parse all pre-built images under FIRMWARE/.
#    2. Generate boot.img and recovery.img based on information in BOOT/ and RECOVERY/.
#    3. Include all images in item 1 and 2 in a firmware.zip.
#    4. Sign firmware.zip with hboot key if hboot key is given.
#####################################################################################
firmware_list_bootloader = [ \
# preloader
        "preloader.bin", \
# lk
        "lk_verified.bin", \
        "lk.bin", \
# tee
        "tee1.img", \
        "tee2.img", \
# hosd
        "hosd_verified.img", \
        "hosd.img", \
# secro
        "secro_verified.img", \
        "secro.img", \
# logo
        "logo_verified.bin", \
        "logo.bin", \
# gpt
        "gpt_main_16g.img", \
        "gpt_main_32g.img", \
        "gpt_main_64g.img", \
        ]

firmware_list_peripheral = [ \
# sensor
        "sensor_hub.img", \
# tp
        "tp_MXM11876.img", \
#a50aml_a31aml_a51aml tp
	"tp_HMX852XE.img", \
	"tp_HMX852XE_MAIN.img", \
	"tp_HMX852XE_SECOND.img", \
        ]

firmware_list_signed = [ \
# boot
        "boot_verified.img", \
        "boot.img", \
# recovery
        "recovery_verified.img", \
        "recovery.img", \
        ]

firmware_list = []
firmware_list.extend(firmware_list_bootloader)
firmware_list.extend(firmware_list_peripheral)
firmware_list.extend(firmware_list_signed)

def handlePackFirmware(script, OPTIONS, output_zip):
  print "Function handlePackFirmware"
  top_path = os.getcwd()
  path_dist = os.path.join(top_path, "out/dist/")
  path_firmware_tmp = os.path.join(top_path, "out/dist/firmware_tmp/")
  path_firmware_manual = os.path.join(top_path, "out/dist/firmware/")
  path_android_info = os.path.join(path_firmware_tmp, "android-info.txt")
  path_android_info_tmp = os.path.join(path_firmware_tmp, "android-info_tmp.txt")
  path_firmware_zip = os.path.join(path_dist, "firmware.zip")

  # Make sure everything is clean
  if os.path.exists(path_firmware_tmp):
    shutil.rmtree(path_firmware_tmp)

  # Special handle for add images in firmware manually
  if os.path.isdir(path_firmware_manual):
    try:
      print path_firmware_manual + " is exist, copy file to " + path_firmware_tmp
      shutil.copytree(path_firmware_manual, path_firmware_tmp)
    except OSError as e:
      print e
      return None
  else:
    try:
      os.mkdir(path_firmware_tmp)
    except OSError as e:
      print e
      return None

  if os.path.exists(path_android_info):
    try:
      os.remove(path_android_info)
    except OSError as e:
      print e
      return None
  if os.path.exists(path_android_info_tmp):
    try:
      os.remove(path_android_info_tmp)
    except OSError as e:
      print e
      return None
  if os.path.exists(path_firmware_zip):
    try:
      os.remove(path_firmware_zip)
    except OSError as e:
      print e
      return None

  # Copy others image from target rom to out/dist/firmware_tmp/
  # Build boot.img and recovery.img to out/dist/firmware_tmp/
  if OPTIONS.incremental_source is None:
    # Full OTA
    print "handlePackFirmware for full package"

    # Get images from the folder FIRMWARE
    for img_name in firmware_list:
      if os.path.exists(os.path.join(path_firmware_tmp, img_name)) is True:
        print img_name + " is copied manually, ignore..."
        continue

      if os.path.isfile(os.path.join(OPTIONS.input_tmp, "FIRMWARE/"+img_name)) is True:
        shutil.copyfile(os.path.join(OPTIONS.input_tmp, "FIRMWARE/"+img_name), os.path.join(path_firmware_tmp, img_name))

  else:
    # Diff OTA
    print "handlePackFirmware for diff package"

    # Get images from the folder FIRMWARE
    for img_name in firmware_list:
      if os.path.exists(os.path.join(path_firmware_tmp, img_name)) is True:
        print img_name + " is copied manually, ignore..."
        continue

      source_img = os.path.join(OPTIONS.source_tmp, "FIRMWARE/"+img_name)
      target_img = os.path.join(OPTIONS.target_tmp, "FIRMWARE/"+img_name)

      if not os.path.isfile(target_img):
        continue
      elif not os.path.isfile(source_img) or not filecmp.cmp(source_img, target_img,False):
        shutil.copyfile(target_img, os.path.join(path_firmware_tmp, img_name))

  # If no image files in tmp_folder, we do not need to pack firmware.zip
  img_flag = False
  for filename in os.listdir(path_firmware_tmp):
    res = re.match("^.*\.img", filename) or re.match("^.*\.bin", filename)
    if res != None:
      img_flag = True

  if img_flag is False:
    print "No image files in firmware.zip! Skip repacking it..."
    return None

  print "Copy android-info.txt to firmware_tmp/"
  shutil.copyfile(os.path.join(os.path.join(OPTIONS.target_tmp, "OTA"), "android-info.txt"), path_android_info)
  # Prevent android_info.txt using DelCache and DelUserData flags
  if os.path.exists(path_android_info) is not True:
    raise ValueError('"%s" does not exist!' % (android_info_path,))
  fr = open(path_android_info,"r")
  fw = open(path_android_info_tmp,"w")
  # Use whitelist(Download mode feature)
  for readline in fr:
    if readline.lower().startswith('modelid:'.lower()) or readline.lower().startswith('cidnum:'.lower()) or \
       readline.lower().startswith('mainver:'.lower()) or readline.lower().startswith('hbootpreupdate:'.lower()) or \
       readline.lower().startswith('eraseonflash:'.lower()):
      fw.write(readline)
    else:
      print "Discard invalid android_info flag: " + readline
  fr.close()
  fw.close()
  shutil.move(path_android_info_tmp,path_android_info)

  # Repack by linux zip tool
  cmd = "cd " + path_firmware_tmp + "; zip firmware.zip " + " ./*"
  print cmd
  os.system(cmd)
  # Move to correct path
  cmd = "mv " + path_firmware_tmp + "/firmware.zip " + path_firmware_zip
  print cmd
  os.system(cmd)

  # Sign firmware.zip with given key(hboot key)
  if OPTIONS.firmware_zip_key is not None:
      handleSignImageByBootloaderKey(path_firmware_zip, OPTIONS.firmware_zip_key)

  print "Pack Firmware ZIP done!"

  output_zip.write(path_firmware_zip, os.path.basename(path_firmware_zip), zipfile.ZIP_DEFLATED)
  script.WriteFirmwareImage("zip", "firmware.zip")

#########################################################################################
#  Description:
#    Create a file, named fotaBoot, fill in it with some content, and then copy it to
#    /cache/recovery/last_fotaBoot by OTA update in order to trigger customization reload
#########################################################################################
path_fotaBoot = "build/tools/releasetools/fotaBoot"
path_ccinfo = "SYSTEM/build.prop"
customization_reload = "fota.load.customization=true"
def handleFotaBoot(script, OPTIONS, output_zip):
  print "Create a fotaBoot file for customization reload"
  if os.path.exists(path_fotaBoot) is True:
    fp = open(path_fotaBoot, "w")
    if OPTIONS.incremental_source is not None:
      #Diff OTA,
      if OPTIONS.cust_force_reload is True:
        print "Force reload customization for Diff OTA!\n"
        fp.write(customization_reload)
      else:
        def get_ccinfo_build_id(dir_path):
          filepath=os.path.join(dir_path, path_ccinfo)
          if os.path.exists(filepath):
            file=open(filepath,"r")
            for line in file.read().split("\n"):
              res = re.match("^ro.aa.customizationid=\s*(?P<build_id>.+$)",line)
              if res != None:
                return res.group('build_id')
          return None
        source_ccinfo=get_ccinfo_build_id(OPTIONS.source_tmp)
        target_ccinfo=get_ccinfo_build_id(OPTIONS.target_tmp)
        if source_ccinfo != target_ccinfo:
          print "ccinfo is different!\n"
          fp.write(customization_reload)
        else:
          print "ccinfo is the same!\n"
    else:
      #Full OTA,
      fp.write(customization_reload)

    fp.close()
    output_zip.write(path_fotaBoot, os.path.basename(path_fotaBoot), zipfile.ZIP_DEFLATED)
    script.Print("Copying fotaBoot to /cache/recovery/last_fotaBoot for customize reload...")
    # Mount /data first
    script.AppendExtra("package_extract_file(\"fotaBoot\", \"/cache/recovery/last_fotaBoot\");")
