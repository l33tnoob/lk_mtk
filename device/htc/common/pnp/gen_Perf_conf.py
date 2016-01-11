import os
import sys
import re
import shutil

TMP_CONF = "device/htc/common/pnp/temp-list.cfg"
CMN_CONF = "device/htc/common/pnp/common-list.cfg"

def replace_conf(src_file, tgt_file, key_file):
	try:
		f = open(src_file, 'r')
		contents = f.readlines()
		f.close()

		if src_file == tgt_file:
			replace_string = False
		else:
			replace_string = True

		f = open(key_file, 'r')

		for line in f.readlines():
			line = line.strip()
			if (not line or line.startswith("#")):
				continue

			substring = line.split("=")
			if (replace_string and substring[1] == "n"):
				newstring = "# " + substring[0].strip() + " is not set\n"
			else:
				newstring = line + "\n"

			isfound = False
			for item in contents:
				entries = re.split('=| ', item)
				if substring[0] in entries:
					loc = contents.index(item)
					contents[loc] = newstring
					isfound = True
					break

			if isfound == False:
				contents.append(newstring)

		f.close()

		f = open(tgt_file, 'w')
		f.write("".join(contents))
		f.close()

		return True

	except Exception, e:
		print "-----replace_conf() Exception Start -----"
		print str(e)
		print "-----replace_conf() Exception End -----"
		return False

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "gen_Perf_conf.py $(kernel-config-prefix-name) [$(white-list path)] [$(kern-perf_defconfig output path)]"
		exit(1)

	if os.path.isfile(CMN_CONF) == False:
		print "Common config file is not existed!"
		exit(1)

	shutil.copy2(CMN_CONF, TMP_CONF)

	argc = len(sys.argv)
	if argc > 2:
		cfg_file = str(sys.argv[2])
		if (cfg_file.find("white-list") != -1 and os.path.isfile(cfg_file) == True):
			replace_conf(TMP_CONF, TMP_CONF, cfg_file)

	src_file = "kernel-3.10/arch/arm/configs/" + str(sys.argv[1]) + "_defconfig"
	tgt_file = "kernel-3.10/arch/arm/configs/" + str(sys.argv[1]) + "-perf_defconfig"
	src_file64 = "kernel-3.10/arch/arm64/configs/" + str(sys.argv[1]) + "_defconfig"
	tgt_file64 = "kernel-3.10/arch/arm64/configs/" + str(sys.argv[1]) + "-perf_defconfig"

	if os.path.isfile(src_file) == False:
		replace_conf(src_file64, tgt_file64, TMP_CONF)
	else:
		replace_conf(src_file, tgt_file, TMP_CONF)

	os.remove(TMP_CONF)

	out_file = str(sys.argv[argc - 1])
	if out_file.find("kern-perf_defconfig") != -1:
		shutil.copy2(tgt_file, str(sys.argv[argc - 1]))

