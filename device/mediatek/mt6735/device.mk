
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml

ifneq ($(strip $(CUSTOM_KERNEL_STEP_COUNTER)),)
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:system/etc/permissions/android.hardware.sensor.stepcounter.xml
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:system/etc/permissions/android.hardware.sensor.stepdetector.xml
endif

ifeq ($(strip $(MTK_IRTX_SUPPORT)),yes)
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.consumerir.xml:system/etc/permissions/android.hardware.consumerir.xml
PRODUCT_PACKAGES += consumerir.mt6735
PRODUCT_PACKAGES += consumerir.mt6735m
PRODUCT_PACKAGES += consumerir.mt6753
endif

#GPU  collection 
PRODUCT_PACKAGES += libI420colorconvert
PRODUCT_PACKAGES += libvcodec_utility
PRODUCT_PACKAGES += libvcodec_oal
PRODUCT_PACKAGES += libh264enc_sa.ca7
PRODUCT_PACKAGES += libh264enc_sb.ca7
PRODUCT_PACKAGES += libHEVCdec_sa.ca7.android
PRODUCT_PACKAGES += libmp4enc_sa.ca7
PRODUCT_PACKAGES += libmp4enc_xa.ca7
PRODUCT_PACKAGES += libvc1dec_sa.ca7
PRODUCT_PACKAGES += libvp8dec_sa.ca7
PRODUCT_PACKAGES += libvp8enc_sa.ca7
PRODUCT_PACKAGES += libvp9dec_sa.ca7
PRODUCT_PACKAGES += libvideoeditorplayer
PRODUCT_PACKAGES += libvideoeditor_osal
PRODUCT_PACKAGES += libvideoeditor_3gpwriter
PRODUCT_PACKAGES += libvideoeditor_mcs
PRODUCT_PACKAGES += libvideoeditor_core
PRODUCT_PACKAGES += libvideoeditor_stagefrightshells
PRODUCT_PACKAGES += libvideoeditor_videofilters
PRODUCT_PACKAGES += libvideoeditor_jni
PRODUCT_PACKAGES += audio.primary.default
PRODUCT_PACKAGES += audio.primary.mt6735
PRODUCT_PACKAGES += audio.primary.mt6735m
PRODUCT_PACKAGES += audio.primary.mt6753
PRODUCT_PACKAGES += audio_policy.stub
PRODUCT_PACKAGES += local_time.default
PRODUCT_PACKAGES += libaudiocustparam
PRODUCT_PACKAGES += libh264dec_xa.ca9
PRODUCT_PACKAGES += libh264dec_xb.ca9
PRODUCT_PACKAGES += libh264dec_customize
PRODUCT_PACKAGES += libmp4dec_sa.ca9
PRODUCT_PACKAGES += libmp4dec_sb.ca9
PRODUCT_PACKAGES += libmp4dec_customize
PRODUCT_PACKAGES += libvp8dec_xa.ca9
PRODUCT_PACKAGES += libmp4enc_xa.ca9
PRODUCT_PACKAGES += libmp4enc_xb.ca9
PRODUCT_PACKAGES += libh264enc_sa.ca9
PRODUCT_PACKAGES += libh264enc_sb.ca9
PRODUCT_PACKAGES += libvcodec_oal
PRODUCT_PACKAGES += libvc1dec_sa.ca9
PRODUCT_PACKAGES += init.factory.rc
PRODUCT_PACKAGES += libaudio.primary.default
PRODUCT_PACKAGES += audio_policy.default
PRODUCT_PACKAGES += audio_policy.mt6735
PRODUCT_PACKAGES += libaudio.a2dp.default
PRODUCT_PACKAGES += libMtkVideoTranscoder
PRODUCT_PACKAGES += libMtkOmxCore
PRODUCT_PACKAGES += libMtkOmxOsalUtils
PRODUCT_PACKAGES += libMtkOmxVdec
PRODUCT_PACKAGES += libMtkOmxVenc
PRODUCT_PACKAGES += libaudiodcrflt
PRODUCT_PACKAGES += libaudiosetting
PRODUCT_PACKAGES += librtp_jni
PRODUCT_PACKAGES += mfv_ut
PRODUCT_PACKAGES += libstagefrighthw
PRODUCT_PACKAGES += libstagefright_memutil
PRODUCT_PACKAGES += factory.ini
PRODUCT_PACKAGES += libmtdutil
PRODUCT_PACKAGES += libminiui
PRODUCT_PACKAGES += factory
PRODUCT_PACKAGES += drvbd
PRODUCT_PACKAGES += libaudio.usb.default
PRODUCT_PACKAGES += audio.usb.default
PRODUCT_PACKAGES += AccountAndSyncSettings
PRODUCT_PACKAGES += DeskClock
PRODUCT_PACKAGES += AlarmProvider
PRODUCT_PACKAGES += Bluetooth
PRODUCT_PACKAGES += Calculator
PRODUCT_PACKAGES += Calendar
PRODUCT_PACKAGES += CertInstaller
PRODUCT_PACKAGES += DrmProvider
PRODUCT_PACKAGES += Email
PRODUCT_PACKAGES += FusedLocation
PRODUCT_PACKAGES += TelephonyProvider
PRODUCT_PACKAGES += Exchange2
PRODUCT_PACKAGES += LatinIME
PRODUCT_PACKAGES += Music
PRODUCT_PACKAGES += MusicFX
PRODUCT_PACKAGES += Protips
PRODUCT_PACKAGES += QuickSearchBox
PRODUCT_PACKAGES += Settings
PRODUCT_PACKAGES += Sync
PRODUCT_PACKAGES += SystemUI
PRODUCT_PACKAGES += Updater
PRODUCT_PACKAGES += CalendarProvider
PRODUCT_PACKAGES += ccci_mdinit
PRODUCT_PACKAGES += ccci_fsd
PRODUCT_PACKAGES += ccci_rpcd
PRODUCT_PACKAGES += batterywarning
PRODUCT_PACKAGES += SyncProvider
PRODUCT_PACKAGES += Launcher3
PRODUCT_PACKAGES += disableapplist.txt
PRODUCT_PACKAGES += resmonwhitelist.txt
PRODUCT_PACKAGES += MTKThermalManager
PRODUCT_PACKAGES += libmtcloader
PRODUCT_PACKAGES += thermal_manager
PRODUCT_PACKAGES += thermald
PRODUCT_PACKAGES += thermal
PRODUCT_PACKAGES += CellConnService
PRODUCT_PACKAGES += MTKAndroidSuiteDaemon
PRODUCT_PACKAGES += libfmjni
PRODUCT_PACKAGES += libfmmt6616
PRODUCT_PACKAGES += libfmmt6626
PRODUCT_PACKAGES += libfmmt6620
PRODUCT_PACKAGES += libfmmt6628
PRODUCT_PACKAGES += libfmmt6627
PRODUCT_PACKAGES += libfmmt6630
PRODUCT_PACKAGES += libfmar1000
PRODUCT_PACKAGES += libfmcust
PRODUCT_PACKAGES += fm_cust.cfg
PRODUCT_PACKAGES += mt6620_fm_cust.cfg
PRODUCT_PACKAGES += mt6627_fm_cust.cfg
PRODUCT_PACKAGES += mt6628_fm_rom.bin
PRODUCT_PACKAGES += mt6628_fm_v1_patch.bin
PRODUCT_PACKAGES += mt6628_fm_v1_coeff.bin
PRODUCT_PACKAGES += mt6628_fm_v2_patch.bin
PRODUCT_PACKAGES += mt6628_fm_v2_coeff.bin
PRODUCT_PACKAGES += mt6628_fm_v3_patch.bin
PRODUCT_PACKAGES += mt6628_fm_v3_coeff.bin
PRODUCT_PACKAGES += mt6628_fm_v4_patch.bin
PRODUCT_PACKAGES += mt6628_fm_v4_coeff.bin
PRODUCT_PACKAGES += mt6628_fm_v5_patch.bin
PRODUCT_PACKAGES += mt6628_fm_v5_coeff.bin
PRODUCT_PACKAGES += mt6627_fm_v1_patch.bin
PRODUCT_PACKAGES += mt6627_fm_v1_coeff.bin
PRODUCT_PACKAGES += mt6627_fm_v2_patch.bin
PRODUCT_PACKAGES += mt6627_fm_v2_coeff.bin
PRODUCT_PACKAGES += mt6627_fm_v3_patch.bin
PRODUCT_PACKAGES += mt6627_fm_v3_coeff.bin
PRODUCT_PACKAGES += mt6627_fm_v4_patch.bin
PRODUCT_PACKAGES += mt6627_fm_v4_coeff.bin
PRODUCT_PACKAGES += mt6627_fm_v5_patch.bin
PRODUCT_PACKAGES += mt6627_fm_v5_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v1_patch.bin
PRODUCT_PACKAGES += mt6630_fm_v1_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v2_patch.bin
PRODUCT_PACKAGES += mt6630_fm_v2_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v3_patch.bin
PRODUCT_PACKAGES += mt6630_fm_v3_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v4_patch.bin
PRODUCT_PACKAGES += mt6630_fm_v4_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v5_patch.bin
PRODUCT_PACKAGES += mt6630_fm_v5_coeff.bin
PRODUCT_PACKAGES += mt6630_fm_v1_patch_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v1_coeff_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v2_patch_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v2_coeff_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v3_patch_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v3_coeff_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v4_patch_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v4_coeff_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v5_patch_tx.bin
PRODUCT_PACKAGES += mt6630_fm_v5_coeff_tx.bin
PRODUCT_PACKAGES += ami304d
PRODUCT_PACKAGES += akmd8963
PRODUCT_PACKAGES += akmd09911
PRODUCT_PACKAGES += akmd8975
PRODUCT_PACKAGES += AcdApiDaemon
PRODUCT_PACKAGES += calib.dat
PRODUCT_PACKAGES += param.dat
PRODUCT_PACKAGES += sensors.dat
PRODUCT_PACKAGES += geomagneticd
PRODUCT_PACKAGES += orientationd
PRODUCT_PACKAGES += memsicd
PRODUCT_PACKAGES += msensord
PRODUCT_PACKAGES += lsm303md
PRODUCT_PACKAGES += memsicd3416x
PRODUCT_PACKAGES += mmc3524xd
PRODUCT_PACKAGES += s62xd smartsensor
PRODUCT_PACKAGES += bmm050d
PRODUCT_PACKAGES += mc6420d
PRODUCT_PACKAGES += magd
PRODUCT_PACKAGES += sensors.mt6735
PRODUCT_PACKAGES += sensors.mt6735m
PRODUCT_PACKAGES += sensors.mt6753
PRODUCT_PACKAGES += libhwm
PRODUCT_PACKAGES += lights.default
PRODUCT_PACKAGES += meta_tst
PRODUCT_PACKAGES += GoogleOtaBinder
PRODUCT_PACKAGES += dhcp6c
PRODUCT_PACKAGES += dhcp6ctl
PRODUCT_PACKAGES += dhcp6c.conf
PRODUCT_PACKAGES += dhcp6cDNS.conf
PRODUCT_PACKAGES += dhcp6s
PRODUCT_PACKAGES += dhcp6s.conf
PRODUCT_PACKAGES += dhcp6c.script
PRODUCT_PACKAGES += dhcp6cctlkey
PRODUCT_PACKAGES += libblisrc
PRODUCT_PACKAGES += libifaddrs
PRODUCT_PACKAGES += libbluetoothdrv
PRODUCT_PACKAGES += libbluetooth_mtk
PRODUCT_PACKAGES += libbluetoothem_mtk
PRODUCT_PACKAGES += libbluetooth_relayer
PRODUCT_PACKAGES += libmeta_bluetooth
PRODUCT_PACKAGES += mobile_log_d
PRODUCT_PACKAGES += libmobilelog_jni
PRODUCT_PACKAGES += libaudio.r_submix.default
PRODUCT_PACKAGES += audio.r_submix.default
PRODUCT_PACKAGES += libaudio.usb.default
PRODUCT_PACKAGES += libnbaio
PRODUCT_PACKAGES += libaudioflinger
PRODUCT_PACKAGES += libmeta_audio
PRODUCT_PACKAGES += liba3m
PRODUCT_PACKAGES += libja3m
PRODUCT_PACKAGES += libmmprofile
PRODUCT_PACKAGES += libmmprofile_jni
PRODUCT_PACKAGES += libtvoutjni
PRODUCT_PACKAGES += libtvoutpattern
PRODUCT_PACKAGES += libmtkhdmi_jni
PRODUCT_PACKAGES += aee
PRODUCT_PACKAGES += aee_aed
PRODUCT_PACKAGES += aee_aed64
PRODUCT_PACKAGES += aee_core_forwarder
PRODUCT_PACKAGES += aee_dumpstate
PRODUCT_PACKAGES += aee_archive
PRODUCT_PACKAGES += rtt
PRODUCT_PACKAGES += libaed
PRODUCT_PACKAGES += libdirect-coredump
PRODUCT_PACKAGES += libmediatek_exceptionlog
PRODUCT_PACKAGES += libcam_platform
PRODUCT_PACKAGES += camera.default
PRODUCT_PACKAGES += camera.mt6735
PRODUCT_PACKAGES += camera.mt6753
PRODUCT_PACKAGES += camera.mt6735m
PRODUCT_PACKAGES += xlog
PRODUCT_PACKAGES += liblog
PRODUCT_PACKAGES += shutdown
PRODUCT_PACKAGES += WIFI_RAM_CODE_MT6630
PRODUCT_PACKAGES += muxreport
PRODUCT_PACKAGES += mtkrild
PRODUCT_PACKAGES += mtk-ril
PRODUCT_PACKAGES += librilmtk
PRODUCT_PACKAGES += libutilrilmtk
PRODUCT_PACKAGES += gsm0710muxd
PRODUCT_PACKAGES += mtkrildmd2
PRODUCT_PACKAGES += mtk-rilmd2
PRODUCT_PACKAGES += librilmtkmd2
PRODUCT_PACKAGES += gsm0710muxdmd2
PRODUCT_PACKAGES += md_minilog_util
PRODUCT_PACKAGES += wbxml
PRODUCT_PACKAGES += wappush
PRODUCT_PACKAGES += thememap.xml
PRODUCT_PACKAGES += libBLPP.so
PRODUCT_PACKAGES += rc.fac
PRODUCT_PACKAGES += mtkGD
PRODUCT_PACKAGES += pvrsrvctl
PRODUCT_PACKAGES += libEGL_mtk.so
PRODUCT_PACKAGES += libGLESv1_CM_mtk.so
PRODUCT_PACKAGES += libGLESv2_mtk.so
PRODUCT_PACKAGES += gralloc.mt6735.so
PRODUCT_PACKAGES += libusc.so
PRODUCT_PACKAGES += libglslcompiler.so
PRODUCT_PACKAGES += libIMGegl.so
PRODUCT_PACKAGES += libpvr2d.so
PRODUCT_PACKAGES += libsrv_um.so
PRODUCT_PACKAGES += libsrv_init.so
PRODUCT_PACKAGES += libPVRScopeServices.so
PRODUCT_PACKAGES += libpvrANDROID_WSEGL.so
PRODUCT_PACKAGES += libFraunhoferAAC
PRODUCT_PACKAGES += audiocmdservice_atci
PRODUCT_PACKAGES += libMtkOmxAudioEncBase
PRODUCT_PACKAGES += libMtkOmxAmrEnc
PRODUCT_PACKAGES += libMtkOmxAwbEnc
PRODUCT_PACKAGES += libMtkOmxAacEnc
PRODUCT_PACKAGES += libMtkOmxVorbisEnc
PRODUCT_PACKAGES += libMtkOmxAdpcmEnc
PRODUCT_PACKAGES += libMtkOmxMp3Dec
PRODUCT_PACKAGES += libMtkOmxGsmDec
PRODUCT_PACKAGES += libMtkOmxAacDec
PRODUCT_PACKAGES += libMtkOmxG711Dec
PRODUCT_PACKAGES += libMtkOmxVorbisDec
PRODUCT_PACKAGES += libMtkOmxAudioDecBase
PRODUCT_PACKAGES += libMtkOmxAdpcmDec
PRODUCT_PACKAGES += libMtkOmxRawDec
PRODUCT_PACKAGES += libMtkOmxAMRNBDec
PRODUCT_PACKAGES += libMtkOmxAMRWBDec
PRODUCT_PACKAGES += libvoicerecognition_jni
PRODUCT_PACKAGES += libvoicerecognition
PRODUCT_PACKAGES += libphonemotiondetector_jni
PRODUCT_PACKAGES += libphonemotiondetector
PRODUCT_PACKAGES += libmotionrecognition
PRODUCT_PACKAGES += libasf
PRODUCT_PACKAGES += libasfextractor
PRODUCT_PACKAGES += audio.primary.default
PRODUCT_PACKAGES += audio_policy.stub
PRODUCT_PACKAGES += audio_policy.default
PRODUCT_PACKAGES += libaudio.primary.default
PRODUCT_PACKAGES += libaudio.a2dp.default
PRODUCT_PACKAGES += libaudio-resampler
PRODUCT_PACKAGES += local_time.default
PRODUCT_PACKAGES += libaudiocustparam
PRODUCT_PACKAGES += libaudiodcrflt
PRODUCT_PACKAGES += libaudiosetting
PRODUCT_PACKAGES += librtp_jni
PRODUCT_PACKAGES += libmatv_cust
PRODUCT_PACKAGES += libmtkplayer
PRODUCT_PACKAGES += libatvctrlservice
PRODUCT_PACKAGES += matv
PRODUCT_PACKAGES += libMtkOmxApeDec
PRODUCT_PACKAGES += libMtkOmxFlacDec
PRODUCT_PACKAGES += ppp_dt
PRODUCT_PACKAGES += power.default
PRODUCT_PACKAGES += libdiagnose
PRODUCT_PACKAGES += netdiag
PRODUCT_PACKAGES += tcpdump
PRODUCT_PACKAGES += mnld
PRODUCT_PACKAGES += libmnlp
PRODUCT_PACKAGES += libmnlp_mt6628
PRODUCT_PACKAGES += libmnlp_mt6620
PRODUCT_PACKAGES += libmnlp_mt3332
PRODUCT_PACKAGES += libmnlp_mt6735
PRODUCT_PACKAGES += gps.default
PRODUCT_PACKAGES += libmnl.a
PRODUCT_PACKAGES += libsupl.a
PRODUCT_PACKAGES += libhotstill.a
PRODUCT_PACKAGES += libagent.a
PRODUCT_PACKAGES += libsonivox
PRODUCT_PACKAGES += iAmCdRom.iso
PRODUCT_PACKAGES += libmemorydumper
PRODUCT_PACKAGES += memorydumper
PRODUCT_PACKAGES += libvt_custom
PRODUCT_PACKAGES += libamrvt
PRODUCT_PACKAGES += libvtmal
PRODUCT_PACKAGES += libipsec_ims
PRODUCT_PACKAGES += racoon
PRODUCT_PACKAGES += libipsec
PRODUCT_PACKAGES += libpcap
PRODUCT_PACKAGES += mtpd
PRODUCT_PACKAGES += netcfg
PRODUCT_PACKAGES += pppd
PRODUCT_PACKAGES += pppd_via
PRODUCT_PACKAGES += pppd_dt
PRODUCT_PACKAGES += dhcpcd
PRODUCT_PACKAGES += dhcpcd.conf
PRODUCT_PACKAGES += dhcpcd-run-hooks
PRODUCT_PACKAGES += 20-dns.conf
PRODUCT_PACKAGES += 95-configured
PRODUCT_PACKAGES += radvd
PRODUCT_PACKAGES += radvd.conf
PRODUCT_PACKAGES += dnsmasq
PRODUCT_PACKAGES += netd
PRODUCT_PACKAGES += ndc
PRODUCT_PACKAGES += libiprouteutil
PRODUCT_PACKAGES += libnetlink
PRODUCT_PACKAGES += tc
PRODUCT_PACKAGES += libext2_profile
PRODUCT_PACKAGES += e2fsck
PRODUCT_PACKAGES += libext2_blkid
PRODUCT_PACKAGES += libext2_e2p
PRODUCT_PACKAGES += libext2_com_err
PRODUCT_PACKAGES += libext2fs
PRODUCT_PACKAGES += libext2_uuid
PRODUCT_PACKAGES += mke2fs
PRODUCT_PACKAGES += tune2fs
PRODUCT_PACKAGES += badblocks
PRODUCT_PACKAGES += resize2fs
PRODUCT_PACKAGES += libnvram
PRODUCT_PACKAGES += libnvram_daemon_callback
PRODUCT_PACKAGES += libfile_op
PRODUCT_PACKAGES += nvram_agent_binder
PRODUCT_PACKAGES += nvram_daemon
PRODUCT_PACKAGES += make_ext4fs
PRODUCT_PACKAGES += sdcard
PRODUCT_PACKAGES += libext
PRODUCT_PACKAGES += libext4
PRODUCT_PACKAGES += libext6
PRODUCT_PACKAGES += libxtables
PRODUCT_PACKAGES += libip4tc
PRODUCT_PACKAGES += libip6tc
PRODUCT_PACKAGES += ipod
PRODUCT_PACKAGES += libipod
PRODUCT_PACKAGES += fuelgauged
PRODUCT_PACKAGES += libfgauge
PRODUCT_PACKAGES += gatord
PRODUCT_PACKAGES += boot_logo_updater
PRODUCT_PACKAGES += boot_logo
PRODUCT_PACKAGES += bootanimation
PRODUCT_PACKAGES += kpoc_charger
PRODUCT_PACKAGES += libtvoutjni
PRODUCT_PACKAGES += libtvoutpattern
PRODUCT_PACKAGES += libmtkhdmi_jni
PRODUCT_PACKAGES += libhissage.so
PRODUCT_PACKAGES += libhpe.so
PRODUCT_PACKAGES += sdiotool
PRODUCT_PACKAGES += superumount
PRODUCT_PACKAGES += libsched
PRODUCT_PACKAGES += fsck_msdos_mtk
PRODUCT_PACKAGES += cmmbsp
PRODUCT_PACKAGES += libcmmb_jni
PRODUCT_PACKAGES += robotium
PRODUCT_PACKAGES += libc_malloc_debug_mtk
PRODUCT_PACKAGES += dpfd
PRODUCT_PACKAGES += libaal
PRODUCT_PACKAGES += aal
PRODUCT_PACKAGES += libaal_cust
PRODUCT_PACKAGES += SchedulePowerOnOff
PRODUCT_PACKAGES += BatteryWarning
PRODUCT_PACKAGES += pq
PRODUCT_PACKAGES += libpq_cust
PRODUCT_PACKAGES += libPQjni
PRODUCT_PACKAGES += libPQDCjni
PRODUCT_PACKAGES += MiraVision
PRODUCT_PACKAGES += libMiraVision_jni
PRODUCT_PACKAGES += hald
PRODUCT_PACKAGES += showmap
PRODUCT_PACKAGES += tiny_mkswap
PRODUCT_PACKAGES += tiny_swapon
PRODUCT_PACKAGES += tiny_swapoff
PRODUCT_PACKAGES += dmlog
PRODUCT_PACKAGES += mtk_msr.ko
PRODUCT_PACKAGES += resize_ext4
PRODUCT_PACKAGES += mtop
PRODUCT_PACKAGES += send_bug
PRODUCT_PACKAGES += met-cmd
PRODUCT_PACKAGES += libmet-tag
PRODUCT_PACKAGES += met_log_d
PRODUCT_PACKAGES += trace-cmd
PRODUCT_PACKAGES += libMtkOmxRawDec
PRODUCT_PACKAGES += libperfservice
PRODUCT_PACKAGES += libperfservice_test
PRODUCT_PACKAGES += libperfservicenative
PRODUCT_PACKAGES += perf_native_test
PRODUCT_PACKAGES += perfservapplist.txt
PRODUCT_PACKAGES += perfservtouchfilter.txt
PRODUCT_PACKAGES += perfservscntbl.txt
PRODUCT_PACKAGES += perfservboosttbl.txt
PRODUCT_PACKAGES += Videos
PRODUCT_PACKAGES += sn
PRODUCT_PACKAGES += lcdc_screen_cap
PRODUCT_PACKAGES += libJniAtvService
PRODUCT_PACKAGES += GoogleKoreanIME
PRODUCT_PACKAGES += memtrack.mt6735
PRODUCT_PACKAGES += memtrack.mt6735m
PRODUCT_PACKAGES += memtrack.mt6753

ifndef MTK_TB_WIFI_3G_MODE
	PRODUCT_PACKAGES += Mms
else
	ifeq ($(strip $(MTK_TB_WIFI_3G_MODE)), 3GDATA_SMS)
		PRODUCT_PACKAGES += Mms
	endif
endif

PRODUCT_PACKAGES += libjni_koreanime.so
PRODUCT_PACKAGES += atcid
PRODUCT_PACKAGES += atci_service
PRODUCT_PACKAGES += libatciserv_jni
PRODUCT_PACKAGES += AtciService
PRODUCT_PACKAGES += wpa_supplicant
PRODUCT_PACKAGES += wpa_cli
PRODUCT_PACKAGES += wpa_supplicant.conf
PRODUCT_PACKAGES += wpa_supplicant_overlay.conf
PRODUCT_PACKAGES += p2p_supplicant_overlay.conf
PRODUCT_PACKAGES += hostapd
PRODUCT_PACKAGES += hostapd_cli
PRODUCT_PACKAGES += lib_driver_cmd_mt66xx.a
PRODUCT_PACKAGES += Dialer
PRODUCT_PACKAGES += libacdk

PRODUCT_PACKAGES += hwcomposer.mt6735
PRODUCT_PACKAGES += hwcomposer.mt6735m
PRODUCT_PACKAGES += hwcomposer.mt6753
PRODUCT_PACKAGES += md_ctrl

PRODUCT_PACKAGES += permission_check
PRODUCT_PROPERTY_OVERRIDES += persist.md.perm.checked=to_upgrade

PRODUCT_PACKAGES += libGLES_android

PRODUCT_PACKAGES += guiext-server

#ifeq ($(strip $(MTK_VT3G324M_SUPPORT)),yes)
PRODUCT_PACKAGES += libmtk_vt_client
PRODUCT_PACKAGES += libmtk_vt_em
PRODUCT_PACKAGES += libmtk_vt_utils
PRODUCT_PACKAGES += libmtk_vt_service
PRODUCT_PACKAGES += libmtk_vt_swip
PRODUCT_PACKAGES += vtservice
#endif

ifeq ($(strip $(BUILD_MTK_LDVT)),yes)
  PRODUCT_PACKAGES += ts_uvvf
  PRODUCT_PACKAGES += ts_md32
endif

ifeq ($(strip $(MTK_TC1_FEATURE)),yes)
  PRODUCT_PACKAGES += libtc1part
  PRODUCT_PACKAGES += libtc1rft
endif

ifeq ($(strip $(MTK_APP_GUIDE)),yes)
  PRODUCT_PACKAGES += ApplicationGuide
endif

ifeq ($(strip $(MTK_FLV_PLAYBACK_SUPPORT)), yes)
  PRODUCT_PACKAGES += libflv
  PRODUCT_PACKAGES += libflvextractor
endif

ifeq ($(strip $(MTK_ETWS_SUPPORT)), yes)
  PRODUCT_PACKAGES += CellBroadcastReceiver
endif

ifneq ($(strip $(foreach value,$(DFO_NVRAM_SET),$(filter yes,$($(value))))),)
  PRODUCT_PACKAGES += featured
  PRODUCT_PACKAGES += libdfo
  PRODUCT_PACKAGES += libdfo_jni
endif

ifeq ($(strip $(MTK_CMAS_SUPPORT)), yes)
  PRODUCT_PACKAGES += CMASReceiver
  PRODUCT_PACKAGES += CmasEM
endif

ifeq ($(strip $(MTK_CDS_EM_SUPPORT)), yes)
  PRODUCT_PACKAGES += CDS_INFO
endif

ifeq ($(strip $(MTK_WLAN_SUPPORT)), yes)
  ifeq ($(strip $(MTK_PLATFORM)), MT8127)
    PRODUCT_PACKAGES += WIFI_RAM_CODE_8127
  else
    ifeq ($(strip $(MTK_PLATFORM)), MT6571)
      PRODUCT_PACKAGES += WIFI_RAM_CODE_6571
    else
      ifeq ($(strip $(MTK_PLATFORM)), MT6735)
        PRODUCT_PACKAGES += WIFI_RAM_CODE_6735
      else
        PRODUCT_PACKAGES += WIFI_RAM_CODE_SOC
      endif
    endif
  endif
endif
#
#
ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  PRODUCT_PACKAGES += nfcservice
endif


ifeq ($(strip $(GEMINI)),yes)
  ifeq ($(strip $(MTK_GEMINI_3SIM_SUPPORT)),yes)
    PRODUCT_PROPERTY_OVERRIDES += persist.gemini.sim_num=3
  else
    ifeq ($(strip $(MTK_GEMINI_4SIM_SUPPORT)),yes)
      PRODUCT_PROPERTY_OVERRIDES += persist.gemini.sim_num=4
    else
      PRODUCT_PROPERTY_OVERRIDES += persist.gemini.sim_num=2
    endif
  endif
else
  PRODUCT_PROPERTY_OVERRIDES += persist.gemini.sim_num=1
endif

ifeq ($(strip $(GEMINI)),yes)
  ifeq ($(strip $(MTK_DT_SUPPORT)),yes)
    PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsda
  else
    PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsds
  endif
else
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=ss
endif
#
ifeq ($(strip $(MTK_GEMINI_SMART_SIM_SWITCH)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ro.gemini.smart_sim_switch=true
else
  PRODUCT_PROPERTY_OVERRIDES += ro.gemini.smart_sim_switch=false
endif
#
ifeq ($(strip $(MTK_GEMINI_SMART_3G_SWITCH)),0)
  PRODUCT_PROPERTY_OVERRIDES += ro.gemini.smart_3g_switch=0
endif

ifeq ($(strip $(MTK_GEMINI_SMART_3G_SWITCH)),1)
  PRODUCT_PROPERTY_OVERRIDES += ro.gemini.smart_3g_switch=1
endif

ifeq ($(strip $(MTK_GEMINI_SMART_3G_SWITCH)),2)
  PRODUCT_PROPERTY_OVERRIDES += ro.gemini.smart_3g_switch=2
endif

ifeq ($(strip $(MTK_EMMC_SUPPORT)), yes)
  PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=EXT4
else
  ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)), yes)
    PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=UBIFS
  else
    PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=YAFFS
  endif
endif

ifeq ($(strip $(MTK_ENABLE_MD1)),yes)
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_1_2g_n.img),)
    PRODUCT_PACKAGES += modem_1_2g_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_1_2g_n.bin
    endif
  endif
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_1_wg_n.img),)
    PRODUCT_PACKAGES += modem_1_wg_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_1_wg_n.bin
    endif
  endif
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_1_tg_n.img),)
    PRODUCT_PACKAGES += modem_1_tg_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_1_tg_n.bin
    endif
  endif
endif

ifeq ($(strip $(MTK_ENABLE_MD2)),yes)
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_2_2g_n.img),)
    PRODUCT_PACKAGES += modem_2_2g_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_2_2g_n.bin
    endif
  endif
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_2_wg_n.img),)
    PRODUCT_PACKAGES += modem_2_wg_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_2_wg_n.bin
    endif
  endif
  ifneq ($(wildcard $(MTK_ROOT_CUSTOM_OUT))/modem/modem_2_tg_n.img),)
    PRODUCT_PACKAGES += modem_2_tg_n.img
    ifeq ($(MTK_MDLOGGER_SUPPORT),yes)
      PRODUCT_PACKAGES += catcher_filter_2_tg_n.bin
    endif
  endif
endif

ifeq ($(strip $(MTK_ISMS_SUPPORT)), yes)
  PRODUCT_PACKAGES += ISmsService
endif

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  PRODUCT_PACKAGES += nfcstackp
  PRODUCT_PACKAGES += DeviceTestApp
  PRODUCT_PACKAGES += libdta_mt6605_jni
  PRODUCT_PACKAGES += libmtknfc_dynamic_load_jni
  PRODUCT_PACKAGES += libnfc_mt6605_jni
  $(call inherit-product-if-exists, vendor/mediatek/proprietary/packages/apps/DeviceTestApp/DeviceTestApp.mk)
  $(call inherit-product-if-exists, vendor/mediatek/proprietary/external/mtknfc/mtknfc.mk)
endif

ifeq ($(strip $(MTK_MTKLOGGER_SUPPORT)), yes)
  PRODUCT_PACKAGES += MTKLogger
endif

ifeq ($(strip $(MTK_SPECIFIC_SM_CAUSE)), yes)
  PRODUCT_PROPERTY_OVERRIDES += ril.specific.sm_cause=1
else
  PRODUCT_PROPERTY_OVERRIDES += ril.specific.sm_cause=0
endif

ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
  PRODUCT_PACKAGES += SDKGallery
else
  PRODUCT_PACKAGES += Gallery2
endif

ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
  PRODUCT_PACKAGES += Provision
endif

ifeq ($(strip $(HAVE_CMMB_FEATURE)), yes)
  PRODUCT_PACKAGES += CMMBPlayer
endif

ifeq ($(strip $(MTK_DATA_TRANSFER_APP)), yes)
  PRODUCT_PACKAGES += DataTransfer
endif

ifeq ($(strip $(MTK_MDM_APP)),yes)
  PRODUCT_PACKAGES += MediatekDM
endif

ifeq ($(strip $(MTK_VT3G324M_SUPPORT)),yes)
  PRODUCT_PACKAGES += libmtk_vt_client
  PRODUCT_PACKAGES += libmtk_vt_em
  PRODUCT_PACKAGES += libmtk_vt_utils
  PRODUCT_PACKAGES += libmtk_vt_service
  PRODUCT_PACKAGES += libmtk_vt_swip
  PRODUCT_PACKAGES += vtservice
endif

ifeq ($(strip $(MTK_OOBE_APP)),yes)
  PRODUCT_PACKAGES += OOBE
endif

ifdef MTK_WEATHER_PROVIDER_APP
  ifneq ($(strip $(MTK_WEATHER_PROVIDER_APP)), no)
    PRODUCT_PACKAGES += MtkWeatherProvider
  endif
endif

ifeq ($(strip $(MTK_ENABLE_VIDEO_EDITOR)),yes)
  PRODUCT_PACKAGES += VideoEditor
endif

ifeq ($(strip $(MTK_CALENDAR_IMPORTER_APP)), yes)
  PRODUCT_PACKAGES += CalendarImporter
endif

ifeq ($(strip $(MTK_LOG2SERVER_APP)), yes)
  PRODUCT_PACKAGES += Log2Server
  PRODUCT_PACKAGES += Excftpcommonlib
  PRODUCT_PACKAGES += Excactivationlib
  PRODUCT_PACKAGES += Excadditionnallib
  PRODUCT_PACKAGES += Excmaillib
endif

ifeq ($(strip $(MTK_CAMERA_APP)), yes)
  PRODUCT_PACKAGES += CameraOpen
else
  PRODUCT_PACKAGES += Camera
endif

ifeq ($(strip $(MTK_VIDEO_FAVORITES_WIDGET_APP)), yes)
  ifneq ($(strip $(MTK_TABLET_PLATFORM)), yes)
    PRODUCT_PACKAGES += VideoFavorites
    PRODUCT_PACKAGES += libjtranscode
  endif
endif

ifeq ($(strip $(MTK_VIDEOWIDGET_APP)),yes)
  PRODUCT_PACKAGES += MtkVideoWidget
endif

ifeq ($(strip $(MTK_BSP_PACKAGE)),yes)
  PRODUCT_PACKAGES += Stk
else
  PRODUCT_PACKAGES += Stk1
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += EngineerMode
  PRODUCT_PACKAGES += EngineerModeSim
  PRODUCT_PACKAGES += libem_bt_jni
  PRODUCT_PACKAGES += libem_platform32_dummy
  PRODUCT_PACKAGES += libem_support_jni
  PRODUCT_PACKAGES += libem_gpio_jni
  PRODUCT_PACKAGES += libem_modem_jni
  PRODUCT_PACKAGES += libem_usb_jni
  PRODUCT_PACKAGES += libem_wifi_jni
  PRODUCT_PACKAGES += libem_sensor_jni
  PRODUCT_PACKAGES += libem_lte_jni
  ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
    PRODUCT_PACKAGES += libem_nfc_jni
  endif
  ifneq ($(strip $(MTK_GMO_RAM_OPTIMIZE)), yes)
      PRODUCT_PACKAGES += em_svr
  endif
endif

ifeq ($(strip $(MTK_RCSE_SUPPORT)), yes)
  PRODUCT_PACKAGES += Rcse
  PRODUCT_PACKAGES += Provisioning
endif

ifeq ($(strip $(MTK_GPS_SUPPORT)), yes)
  PRODUCT_PACKAGES += YGPS
  PRODUCT_PACKAGES += BGW
  PRODUCT_PROPERTY_OVERRIDES += bgw.current3gband=0
endif

ifeq ($(strip $(MTK_GPS_SUPPORT)), yes)
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT6620)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=1
  endif
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT6628)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=1
  endif
  ifeq ($(strip $(MTK_GPS_CHIP)), MTK_GPS_MT3332)
    PRODUCT_PROPERTY_OVERRIDES += gps.solution.combo.chip=0
  endif
endif

ifeq ($(strip $(MTK_NAND_UBIFS_SUPPORT)),yes)
  PRODUCT_PACKAGES += mkfs_ubifs
  PRODUCT_PACKAGES += ubinize
  PRODUCT_PACKAGES += mtdinfo
  PRODUCT_PACKAGES += ubiupdatevol
  PRODUCT_PACKAGES += ubirmvol
  PRODUCT_PACKAGES += ubimkvol
  PRODUCT_PACKAGES += ubidetach
  PRODUCT_PACKAGES += ubiattach
  PRODUCT_PACKAGES += ubinfo
  PRODUCT_PACKAGES += ubiformat
endif

ifeq ($(strip $(MTK_EXTERNAL_MODEM_SLOT)),2)
  PRODUCT_PROPERTY_OVERRIDES += ril.external.md=2
else
  ifeq ($(strip $(MTK_EXTERNAL_MODEM_SLOT)),1)
    PRODUCT_PROPERTY_OVERRIDES += ril.external.md=1
  else
    PRODUCT_PROPERTY_OVERRIDES += ril.external.md=0
  endif
endif

ifeq ($(strip $(MTK_LIVEWALLPAPER_APP)), yes)
  PRODUCT_PACKAGES += LiveWallpapers
  PRODUCT_PACKAGES += LiveWallpapersPicker
  PRODUCT_PACKAGES += MagicSmokeWallpapers
  PRODUCT_PACKAGES += VisualizationWallpapers
  PRODUCT_PACKAGES += Galaxy4
  PRODUCT_PACKAGES += HoloSpiralWallpaper
  PRODUCT_PACKAGES += NoiseField
  PRODUCT_PACKAGES += PhaseBeam
endif

ifeq ($(strip $(MTK_SNS_SUPPORT)), yes)
  PRODUCT_PACKAGES += SNSCommon
  PRODUCT_PACKAGES += SnsContentProvider
  PRODUCT_PACKAGES += SnsWidget
  PRODUCT_PACKAGES += SnsWidget24
  PRODUCT_PACKAGES += SocialStream
  ifeq ($(strip $(MTK_SNS_KAIXIN_APP)), yes)
    PRODUCT_PACKAGES += KaiXinAccountService
  endif
  ifeq ($(strip $(MTK_SNS_RENREN_APP)), yes)
    PRODUCT_PACKAGES += RenRenAccountService
  endif
  ifeq ($(strip $(MTK_SNS_FACEBOOK_APP)), yes)
    PRODUCT_PACKAGES += FacebookAccountService
  endif
  ifeq ($(strip $(MTK_SNS_FLICKR_APP)), yes)
    PRODUCT_PACKAGES += FlickrAccountService
  endif
  ifeq ($(strip $(MTK_SNS_TWITTER_APP)), yes)
    PRODUCT_PACKAGES += TwitterAccountService
  endif
  ifeq ($(strip $(MTK_SNS_SINAWEIBO_APP)), yes)
    PRODUCT_PACKAGES += WeiboAccountService
  endif
endif


ifeq ($(strip $(MTK_SYSTEM_UPDATE_SUPPORT)), yes)
  PRODUCT_PACKAGES += SystemUpdate
  PRODUCT_PACKAGES += SystemUpdateAssistant
endif

ifeq ($(strip $(MTK_DATADIALOG_APP)), yes)
  PRODUCT_PACKAGES += DataDialog
endif

ifeq ($(strip $(MTK_DATA_TRANSFER_APP)), yes)
  PRODUCT_PACKAGES += DataTransfer
endif

ifeq ($(strip $(MTK_FM_SUPPORT)), yes)
  PRODUCT_PACKAGES += FmRadio
endif

ifeq ($(strip $(MTK_CAM_LOMO_SUPPORT)), yes)
  PRODUCT_PROPERTY_OVERRIDES += ro.mtk_cam_lomo_support=1
  PRODUCT_PACKAGES += libjni_lomoeffect
endif

ifeq (MT6620_FM,$(strip $(MTK_FM_CHIP)))
  PRODUCT_PROPERTY_OVERRIDES += fmradio.driver.chip=1
endif

ifeq (MT6626_FM,$(strip $(MTK_FM_CHIP)))
  PRODUCT_PROPERTY_OVERRIDES += fmradio.driver.chip=2
endif

ifeq (MT6628_FM,$(strip $(MTK_FM_CHIP)))
  PRODUCT_PROPERTY_OVERRIDES += fmradio.driver.chip=3
endif

ifeq ($(strip $(MTK_BT_SUPPORT)), yes)
  PRODUCT_PROPERTY_OVERRIDES += \
        ro.btstack=blueangel

  PRODUCT_PACKAGES += auto_pair_blacklist.conf
  PRODUCT_PACKAGES += btconfig.xml
  PRODUCT_PACKAGES += MtkBt
  PRODUCT_PACKAGES += libbtcusttable
  PRODUCT_PACKAGES += libbtcust
  PRODUCT_PACKAGES += libmtkbtextadp
  PRODUCT_PACKAGES += libextpbap
  PRODUCT_PACKAGES += libextavrcp
  PRODUCT_PACKAGES += libextopp
  PRODUCT_PACKAGES += libextsys
  PRODUCT_PACKAGES += libextftp
  PRODUCT_PACKAGES += libmtkbtextadpa2dp
  PRODUCT_PACKAGES += libmtka2dp
  PRODUCT_PACKAGES += libextbip
  PRODUCT_PACKAGES += libextbpp
  PRODUCT_PACKAGES += libexthid
  PRODUCT_PACKAGES += libextsimap
  PRODUCT_PACKAGES += libextjsr82
  PRODUCT_PACKAGES += libbtsession
  PRODUCT_PACKAGES += libmtkbtextpan
  PRODUCT_PACKAGES += libextmap
  PRODUCT_PACKAGES += libmtkbtextspp
  PRODUCT_PACKAGES += libexttestmode
  PRODUCT_PACKAGES += libpppbtdun
  PRODUCT_PACKAGES += libextopp_jni
  PRODUCT_PACKAGES += libexthid_jni
  PRODUCT_PACKAGES += libextpan_jni
  PRODUCT_PACKAGES += libextftp_jni
  PRODUCT_PACKAGES += libextbpp_jni
  PRODUCT_PACKAGES += libextbip_jni
  PRODUCT_PACKAGES += libextpbap_jni
  PRODUCT_PACKAGES += libextavrcp_jni
  PRODUCT_PACKAGES += libextsimap_jni
  PRODUCT_PACKAGES += libextdun_jni
  PRODUCT_PACKAGES += libextmap_jni
  PRODUCT_PACKAGES += libextsys_jni
  PRODUCT_PACKAGES += libextadvanced_jni
  PRODUCT_PACKAGES += btlogmask
  PRODUCT_PACKAGES += btconfig
  PRODUCT_PACKAGES += libbtpcm
  PRODUCT_PACKAGES += libbtsniff
  PRODUCT_PACKAGES += mtkbt
  PRODUCT_PACKAGES += bluetooth.blueangel
  PRODUCT_PACKAGES += audio.a2dp.blueangel
  PRODUCT_PACKAGES += pppd_btdun
endif

ifeq ($(strip $(MTK_DT_SUPPORT)),yes)
  ifneq ($(strip $(MTK_C2K_SUPPORT)),yes)
    ifeq ($(strip $(MTK_MDLOGGER_SUPPORT)),yes)
      PRODUCT_PACKAGES += ExtModemLog
      PRODUCT_PACKAGES += libextmdlogger_ctrl_jni
      PRODUCT_PACKAGES += libextmdlogger_ctrl
      PRODUCT_PACKAGES += extmdlogger
    endif
  endif
endif

ifeq ($(strip $(MTK_ACWFDIALOG_APP)), yes)
  PRODUCT_PACKAGES += AcwfDialog
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += EngineerMode
  PRODUCT_PACKAGES += MobileLog
endif

ifeq ($(strip $(HAVE_MATV_FEATURE)),yes)
  PRODUCT_PACKAGES += MtvPlayer
  PRODUCT_PACKAGES += MATVEM
  PRODUCT_PACKAGES += com.mediatek.atv.adapter
endif

ifneq ($(strip $(MTK_LCM_PHYSICAL_ROTATION)),)
  PRODUCT_PROPERTY_OVERRIDES += ro.sf.hwrotation=$(MTK_LCM_PHYSICAL_ROTATION)
endif

ifeq ($(strip $(MTK_SHARE_MODEM_CURRENT)),2)
  PRODUCT_PROPERTY_OVERRIDES += ril.current.share_modem=2
else
  ifeq ($(strip $(MTK_SHARE_MODEM_CURRENT)),1)
    PRODUCT_PROPERTY_OVERRIDES += ril.current.share_modem=1
  else
    PRODUCT_PROPERTY_OVERRIDES += ril.current.share_modem=0
  endif
endif

ifeq ($(strip $(MTK_FM_TX_SUPPORT)), yes)
  PRODUCT_PACKAGES += FMTransmitter
endif

ifeq ($(strip $(MTK_SOUNDRECORDER_APP)),yes)
  PRODUCT_PACKAGES += SoundRecorder
endif

ifeq ($(strip $(MTK_LOCKSCREEN_TYPE)),2)
  PRODUCT_PACKAGES += MtkWallPaper
endif

ifneq ($(strip $(MTK_LOCKSCREEN_TYPE)),)
  PRODUCT_PROPERTY_OVERRIDES += curlockscreen=$(MTK_LOCKSCREEN_TYPE)
endif

ifeq ($(strip $(MTK_OMA_DOWNLOAD_SUPPORT)),yes)
  PRODUCT_PACKAGES += Browser
  PRODUCT_PACKAGES += DownloadProvider
endif

ifeq ($(strip $(MTK_OMACP_SUPPORT)),yes)
  PRODUCT_PACKAGES += Omacp
endif

ifeq ($(strip $(MTK_VIDEO_THUMBNAIL_PLAY_SUPPORT)),yes)
  PRODUCT_PACKAGES += libjtranscode
endif

ifeq ($(strip $(MTK_WIFI_P2P_SUPPORT)),yes)
  PRODUCT_PACKAGES += WifiContactSync
  PRODUCT_PACKAGES += WifiP2PWizardy
  PRODUCT_PACKAGES += FileSharingServer
  PRODUCT_PACKAGES += FileSharingClient
  PRODUCT_PACKAGES += UPnPAV
  PRODUCT_PACKAGES += WifiWsdsrv
  PRODUCT_PACKAGES += bonjourExplorer
endif

ifeq ($(strip $(CUSTOM_KERNEL_TOUCHPANEL)),generic)
  PRODUCT_PACKAGES += Calibrator
endif

ifeq ($(strip $(MTK_FILEMANAGER_APP)), yes)
  PRODUCT_PACKAGES += FileManager
endif

ifeq ($(strip $(MTK_ENGINEERMODE_APP)), yes)
  PRODUCT_PACKAGES += ActivityNetwork
endif

ifneq ($(findstring OP03, $(strip $(OPTR_SPEC_SEG_DEF))),)
  PRODUCT_PACKAGES += SimCardAuthenticationService
endif

ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  PRODUCT_PACKAGES += NxpSecureElement
endif

ifeq ($(strip $(MTK_NFC_OMAAC_SUPPORT)),yes)
  PRODUCT_PACKAGES += SmartcardService
  PRODUCT_PACKAGES += org.simalliance.openmobileapi
  PRODUCT_PACKAGES += org.simalliance.openmobileapi.xml
  PRODUCT_PACKAGES += libassd
endif

ifeq ($(strip $(MTK_APKINSTALLER_APP)), yes)
  PRODUCT_PACKAGES += APKInstaller
endif

ifeq ($(strip $(MTK_SMSREG_APP)), yes)
  PRODUCT_PACKAGES += SmsReg
endif

ifeq ($(strip $(GEMINI)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ro.mediatek.gemini_support=true
else
  PRODUCT_PROPERTY_OVERRIDES += ro.mediatek.gemini_support=false
endif

ifeq ($(MTK_BACKUPANDRESTORE_APP),yes)
  PRODUCT_PACKAGES += BackupAndRestore
endif

ifeq ($(strip $(MTK_NOTEBOOK_SUPPORT)),yes)
  PRODUCT_PACKAGES += NoteBook
endif

ifeq ($(strip $(MTK_BWC_SUPPORT)), yes)
  PRODUCT_PACKAGES += libbwc
endif

ifeq ($(strip $(MTK_GPU_SUPPORT)), yes)
  ifeq ($(MTK_PLATFORM),$(filter $(MTK_PLATFORM),MT6735))
    PRODUCT_PACKAGES += gralloc.mt6735
    PRODUCT_PACKAGES += gralloc.mt6735m
    PRODUCT_PACKAGES += gralloc.mt6753
    PRODUCT_PACKAGES += libGLES_mali
    PRODUCT_PACKAGES += libgpu_aux
  endif
endif


ifeq ($(strip $(MTK_DT_SUPPORT)),yes)
  PRODUCT_PACKAGES += ip-up
  PRODUCT_PACKAGES += ip-down
  PRODUCT_PACKAGES += ppp_options
  PRODUCT_PACKAGES += chap-secrets
  PRODUCT_PACKAGES += init.gprs-pppd
endif

ifdef OPTR_SPEC_SEG_DEF
  ifneq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
    OPTR := $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SPEC := $(word 2,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SEG  := $(word 3,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    $(call inherit-product-if-exists, mediatek/operator/$(OPTR)/$(SPEC)/$(SEG)/optr_apk_config.mk)

    # Todo:
    # obsolete this section's configuration for operator project resource overlay
    # once all operator related overlay resource moved to custom folder
    PRODUCT_PACKAGE_OVERLAYS += mediatek/operator/$(OPTR)/$(SPEC)/$(SEG)/OverLayResource
    # End

    PRODUCT_PROPERTY_OVERRIDES += ro.operator.optr=$(OPTR)
    PRODUCT_PROPERTY_OVERRIDES += ro.operator.spec=$(SPEC)
    PRODUCT_PROPERTY_OVERRIDES += ro.operator.seg=$(SEG)
  endif
endif

#Move definition to build\core\main.mk
#ifneq ($(TARGET_BUILD_VARIANT),user)
#  ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=0
#else
#  ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=1
#endif

ifeq ($(strip $(GEMINI)), yes)
  ifeq ($(OPTR_SPEC_SEG_DEF),NONE)
    PRODUCT_PACKAGES += StkSelection
  endif
  ifeq (OP01,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    PRODUCT_PACKAGES += StkSelection
  endif
  ifndef OPTR_SPEC_SEG_DEF
    PRODUCT_PACKAGES += StkSelection
  endif
endif

ifeq (yes,$(strip $(MTK_FD_SUPPORT)))
  # Only support the format: n.m (n:1 or 1+ digits, m:Only 1 digit) or n (n:integer)
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.fd.counter=15
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.fd.off.counter=5
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.fd.r8.counter=15
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.fd.off.r8.counter=5
endif


#################################################
#Widevine DRM part
ifeq ($(strip $(MTK_WVDRM_SUPPORT)),yes)
  #both L1 and L3 library
  PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
  PRODUCT_PACKAGES += com.google.widevine.software.drm.xml
  PRODUCT_PACKAGES += com.google.widevine.software.drm
  PRODUCT_PACKAGES += libdrmmtkutil
  PRODUCT_PACKAGES += libdrmwvmplugin
  PRODUCT_PACKAGES += libwvm
  PRODUCT_PACKAGES += libdrmdecrypt
  ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
    PRODUCT_PACKAGES += libWVStreamControlAPI_L1
    PRODUCT_PACKAGES += libwvdrm_L1
    PRODUCT_PACKAGES += lib_uree_mtk_crypto
  else
    PRODUCT_PACKAGES += libWVStreamControlAPI_L3
    PRODUCT_PACKAGES += libwvdrm_L3
  endif
else
  PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=false
endif

###Modular drm
ifeq ($(strip $(MTK_WVDRM_SUPPORT)),yes)
  #Mock modular drm plugin for cts
  PRODUCT_PACKAGES += libmockdrmcryptoplugin
  #both L1 and L3 library
  PRODUCT_PACKAGES += libwvdrmengine
  ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
    PRODUCT_PACKAGES += lib_uree_mtk_modular_drm
    PRODUCT_PACKAGES += liboemcrypto
  endif
endif

################################################

#################################################
#OMA DRM part, MTK_DRM_APP should be set to false
ifeq ($(strip $(MTK_DRM_APP)),yes)
  PRODUCT_PACKAGES += libdrmmtkutil
  #Media framework reads this property
  PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
  ifeq ($(strip $(MTK_OMADRM_SUPPORT)), yes)
    PRODUCT_PACKAGES += libdrmmtkplugin
    PRODUCT_PACKAGES += drm_chmod
    PRODUCT_PACKAGES += libdcfdecoderjni
  endif
  ifeq ($(strip $(MTK_MOBILE_MANAGEMENT)), yes)
    PRODUCT_PACKAGES += libdrmctaplugin
    PRODUCT_PACKAGES += DataProtection
  endif
endif
################################################

ifeq (yes,$(strip $(MTK_FM_SUPPORT)))
  PRODUCT_PROPERTY_OVERRIDES += fmradio.driver.enable=1
else
  PRODUCT_PROPERTY_OVERRIDES += fmradio.driver.enable=0
endif




ifeq ($(strip $(MTK_NFC_SUPPORT)), yes)
  ifeq ($(strip $(MTK_NFC_APP_SUPPORT)), yes)
    PRODUCT_PACKAGES += NFCTagMaster
    PRODUCT_PACKAGES += NFCSysOper
  endif
endif

ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
  PRODUCT_PACKAGES += DTSTruMedia
  PRODUCT_PACKAGES += libsrsprocessing
endif

ifeq ($(strip $(MTK_DOLBY_DAP_SUPPORT)), yes)
  PRODUCT_PACKAGES += libdseffect \
                      Ds \
                      DsUI \
                      dolby_ds
endif

ifeq ($(strip $(MTK_AUDIO_DDPLUS_SUPPORT)), yes)
    PRODUCT_PACKAGES += libstagefright_soft_ddpdec
endif

ifeq ($(strip $(MTK_WEATHER_WIDGET_APP)), yes)
  PRODUCT_PACKAGES += MtkWeatherWidget
endif

ifeq ($(strip $(MTK_FIRST_MD)),1)
  PRODUCT_PROPERTY_OVERRIDES += ril.first.md=1
endif

ifeq ($(strip $(MTK_FIRST_MD)),2)
  PRODUCT_PROPERTY_OVERRIDES += ril.first.md=2
endif

ifeq ($(strip $(MTK_FLIGHT_MODE_POWER_OFF_MD)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ril.flightmode.poweroffMD=1
else
  PRODUCT_PROPERTY_OVERRIDES += ril.flightmode.poweroffMD=0
endif

ifeq ($(strip $(MTK_FIRST_MD)),1)
  PRODUCT_PROPERTY_OVERRIDES += ril.first.md=1
endif

ifeq ($(strip $(MTK_FIRST_MD)),2)
  PRODUCT_PROPERTY_OVERRIDES += ril.first.md=2
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),0)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=0
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),1)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=1
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),2)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=2
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),3)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=3
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),4)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=4
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),5)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=5
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),6)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=6
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),7)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=7
endif

ifeq ($(strip $(MTK_TELEPHONY_MODE)),8)
  PRODUCT_PROPERTY_OVERRIDES += ril.telephony.mode=8
endif

ifeq ($(strip $(MTK_AGPS_APP)), yes)
  PRODUCT_PACKAGES += LocationEM \
                      LocationEM2 \
                      mtk_agpsd \
                      libssladp \
                      cacerts_supl \
                      AutoDialer
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/frameworks/base/agps/etc/agps_profiles_conf2.xml:system/etc/agps_profiles_conf2.xml
endif

PRODUCT_PACKAGES += wifi2agps

ifeq ($(strip $(MTK_WFD_SINK_SUPPORT)), yes)
  PRODUCT_PACKAGES += MtkFloatMenu
endif

PRODUCT_PACKAGES += libsec
PRODUCT_PACKAGES += sbchk

PRODUCT_BRAND := alps
PRODUCT_MANUFACTURER := alps

PRODUCT_COPY_FILES += vendor/mediatek/proprietary/frameworks/base/telephony/etc/apns-conf.xml:system/etc/apns-conf.xml
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/frameworks/base/telephony/etc/spn-conf.xml:system/etc/spn-conf.xml

# for USB Accessory Library/permission
# Mark for early porting in JB
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml
PRODUCT_PACKAGES += com.android.future.usb.accessory

# System property for MediaTek ANR pre-dump.
PRODUCT_PROPERTY_OVERRIDES += dalvik.vm.mtk-stack-trace-file=/data/anr/mtk_traces.txt

ifeq ($(strip $(MTK_WLAN_SUPPORT)),yes)
  PRODUCT_PROPERTY_OVERRIDES += mediatek.wlan.chip=$(MTK_WLAN_CHIP)
  PRODUCT_PROPERTY_OVERRIDES += mediatek.wlan.module.postfix="_"$(shell echo $(strip $(MTK_WLAN_CHIP)) | tr A-Z a-z)
endif

ifeq ($(strip $(MTK_RILD_READ_IMSI)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ril.read.imsi=1
endif

ifeq ($(strip $(MTK_RADIOOFF_POWER_OFF_MD)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ril.radiooff.poweroffMD=1
else
  PRODUCT_PROPERTY_OVERRIDES += ril.radiooff.poweroffMD=0
endif

ifeq ($(strip $(MTK_FACTORY_RESET_PROTECTION_SUPPORT)),yes)
  PRODUCT_PROPERTY_OVERRIDES += ro.frp.pst=/dev/block/platform/mtk-msdc.0/by-name/frp
endif

PRODUCT_COPY_FILES += device/mediatek/mt6735/init.mt6735.rc:root/init.mt6735.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/factory_init.rc:root/factory_init.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.modem.rc:root/init.modem.rc

ifeq ($(strip $(MTK_TC7_FEATURE)), yes)
# RamDump
PRODUCT_COPY_FILES += device/mediatek/mt6735/frdump_init.rc:root/frdump_init.rc
PRODUCT_PACKAGES += frdump
endif

ifeq ($(strip $(MTK_C2K_SUPPORT)),yes)
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.c2k.rc:root/init.c2k.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/meta_init.c2k.rc:root/meta_init.c2k.rc
endif

ifeq ($(strip $(MTK_SHARED_SDCARD)),yes)
ifeq ($(strip $(MTK_2SDCARD_SWAP)),yes)
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.ssd_nomuser.rc:root/init.ssd_nomuser.rc
else
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.ssd.rc:root/init.ssd.rc
endif
else
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.no_ssd.rc:root/init.no_ssd.rc
endif

ifeq ($(strip $(MTK_FAT_ON_NAND)),yes)
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.fon.rc:root/init.fon.rc
endif

ifeq ($(HTC_UT_BUILD), true)
	PRODUCT_PROPERTY_OVERRIDES += ro.forceenablexlog=true
	PRODUCT_COPY_FILES += device/mediatek/mt6735/init.xlog_ut.rc:root/init.xlog.rc
else
	PRODUCT_COPY_FILES += device/mediatek/mt6735/init.xlog.rc:root/init.xlog.rc
endif

PRODUCT_COPY_FILES += device/mediatek/mt6735/init.xlog.rc:root/init.xlog.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/meta_init.modem.rc:root/meta_init.modem.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/meta_init.rc:root/meta_init.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.mt6735.usb.rc:root/init.mt6735.usb.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/init.recovery.mt6735.rc:root/init.recovery.mt6735.rc
PRODUCT_COPY_FILES += device/mediatek/mt6735/egl.cfg:system/lib/egl/egl.cfg
ifeq ($(strip $(MTK_EMMC_SUPPORT)), yes)
  ifeq ($(strip $(MTK_CIP_SUPPORT)), yes)
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab.mt6735.wwop:root/fstab.mt6735
  else
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab.mt6735:root/fstab.mt6735
  endif
else
PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab.mt6735.nand:root/fstab.mt6735
endif

#PRODUCT_COPY_FILES += $(LOCAL_PATH)/thermal.conf:system/etc/.tp/thermal.conf
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/thermal.off.conf:system/etc/.tp/thermal.off.conf
PRODUCT_COPY_FILES += $(LOCAL_PATH)/partition_permission.sh:system/etc/partition_permission.sh
PRODUCT_COPY_FILES += $(LOCAL_PATH)/ht120.mtc:system/etc/.tp/.ht120.mtc
PRODUCT_COPY_FILES += $(LOCAL_PATH)/throttle.sh:system/etc/throttle.sh
PRODUCT_COPY_FILES += $(LOCAL_PATH)/enableswap.sh:root/enableswap.sh

PRODUCT_COPY_FILES += device/mediatek/mt6735/media_codecs.xml:system/etc/media_codecs.xml
PRODUCT_COPY_FILES += device/mediatek/mt6735/media_codecs.xml:system/etc/permissions/media_codecs.xml
PRODUCT_COPY_FILES += device/mediatek/mt6735/mtk_omx_core.cfg:system/etc/mtk_omx_core.cfg

# HTC_AUD_START
$(warning :Checking ENABLE_MIRRORLINK_RECORD: $(ENABLE_MIRRORLINK_RECORD).)
ifeq ($(strip $(ENABLE_MIRRORLINK_RECORD)),yes)
    PRODUCT_PACKAGES += audio.mirrorlink_rec.default
endif
# HTC_AUD_END

#Audio config
PRODUCT_COPY_FILES += device/mediatek/mt6735/audio_device.xml:system/etc/audio_device.xml

ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)), yes)
  PRODUCT_PACKAGES += keystore.mt6735
  PRODUCT_PACKAGES += keystore.mt6735m
  PRODUCT_PACKAGES += keystore.mt6753
  PRODUCT_COPY_FILES += device/mediatek/mt6735/init.trustonic.rc:root/init.trustonic.rc
endif

#inherit common platform
$(call inherit-product, device/mediatek/common/device.mk)
$(call inherit-product-if-exists, device/htc/common/device.mk)

#inherit prebuilt
$(call inherit-product-if-exists, vendor/mediatek/mt6735/device-vendor.mk)

# for 3Gdongle
ifeq ($(strip $(MTK_EXTERNAL_DONGLE_SUPPORT)), yes)
    PRODUCT_PACKAGES += dongled \
                        chat \
                        rild_dongle \
                        libril_dongle \
                        Dongle \
                        TeledongleDemo \
                        tedongle-telephony

    $(call inherit-product-if-exists, vendor/mediatek/proprietary/hardware/dongle-ril/vendorSoFile64/vendorSoFile.mk)
    $(call inherit-product-if-exists, vendor/mediatek/proprietary/hardware/dongle-ril/dongled/dongled.mk)
    $(call inherit-product-if-exists, vendor/mediatek/proprietary/hardware/dongle-ril/vendor001-ril/vendor001-ril.mk)
    $(call inherit-product-if-exists, vendor/mediatek/proprietary/hardware/dongle-ril/vendor002-ril/vendor002-ril.mk)
    $(call inherit-product-if-exists, frameworks/opt/tedongle/tedonglePermission.mk)
endif

