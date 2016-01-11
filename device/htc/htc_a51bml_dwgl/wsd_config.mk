#HTC_WSD_START
#below option needs to have different config for CT, India and also needs to modify kernel config at the same time
#currently, India SKU is not ready, we don't need modify kernel config, just configurate CUSTOM_MODEM
ifeq ($(PRIVATE_RCMS_SKU), HTCCN CHS CT)
  CUSTOM_MODEM = mt6753_a51b_CT_c2k_svlte_custom mt6753_a51b_CT_c2k_lwg_dsds_custom
else ifeq ($(PRIVATE_RCMS_SKU), hTC Asia India)
  CUSTOM_MODEM = a51bml_dwgl_India_c2k a51bml_dwgl_India_lwg
else ifneq ($(wildcard mfg/.patched), )
  #This is for MFG build
  CUSTOM_MODEM = mt6753_a51b_MFG_c2k_svlte_custom mt6753_a51b_MFG_c2k_lwg_dsds_custom
else
  CUSTOM_MODEM = mt6753_a51b_c2k_svlte_custom mt6753_a51b_c2k_lwg_dsds_custom
endif
#HTC_WSD_END
