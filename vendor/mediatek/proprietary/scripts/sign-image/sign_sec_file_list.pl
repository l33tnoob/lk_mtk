#!/usr/bin/perl

use lib "alps/vendor/mediatek/proprietary/scripts/sign-image";
use lib "device/mediatek/build/build/tools";
use pack_dep_gen;
PrintDependModule($0);

##########################################################
# Initialize Variables
##########################################################
#my $TARGET_PRODUCT = $ENV{"TARGET_PRODUCT"};
#my $prj = substr $TARGET_PRODUCT, (index($TARGET_PRODUCT, 'full_') + 5);
my $cust_dir = "vendor/mediatek/proprietary/custom";
my $prj = $ARGV[0];
my $base_prj = $ARGV[1];
my $key_dir = "$cust_dir/$base_prj/security/image_auth";
my $cfg_dir = "$cust_dir/$base_prj/security/sec_file_list";
my $cipher_tool = "vendor/mediatek/proprietary/scripts/ciphertool/CipherTool";
my $sign_tool = "vendor/mediatek/proprietary/scripts/sign-image/SignTool.sh";
my $OUT_DIR = "out";


print "******************************************************\n";
print "*********************** SETTINGS *********************\n";
print "******************************************************\n";
print " TARGET PROJECT = $prj\n";
print " BASE PROJECT = $base_prj\n";

##########################################################
# Sign ANDROID Secure File List
##########################################################
print "\n\n*** Sign ANDROID Secure File List ***\n\n";

my $and_secfl = "$cust_dir/$base_prj/security/sec_file_list/ANDRO_SFL.ini";
my $s_andro_fl_dir = "$OUT_DIR/target/product/$prj/system/etc/firmware";
my $s_andro_fl = "$s_andro_fl_dir/S_ANDRO_SFL.ini";

if (! -d "$s_andro_fl_dir"){
	system("mkdir -p $s_andro_fl_dir");
}

if (-e "$and_secfl")
{
	if (-e "$s_andro_fl")
	{
		print "remove old file list (1) ... \n";
		system("rm -f $s_andro_fl");
	}
					
	PrintDependency("$key_dir/IMG_AUTH_KEY.ini");
	PrintDependency("$cfg_dir/SFL_CFG.ini");
	PrintDependency($and_secfl);
	system("./$sign_tool $key_dir/IMG_AUTH_KEY.ini $cfg_dir/SFL_CFG.ini $and_secfl $s_andro_fl");
	
	if (! -e "$s_andro_fl")
	{
		die "sign failed. please check";
	}
}
else
{
	print "file doesn't exist\n";	
}


##########################################################
# Sign SECRO Secure File List
##########################################################
print "\n\n*** Sign SECRO Secure File List ***\n\n";

my $secro_secfl = "$cust_dir/$base_prj/security/sec_file_list/SECRO_SFL.ini";
my $s_secro_out_dir = "$OUT_DIR/target/product/$prj/secro";
my $s_secro_fl_o1 = "$s_secro_out_dir/S_SECRO_SFL.ini";
my $s_secro_fl_o2 = "$s_secro_out_dir/S_SECRO_SFL.ini";

if (-e "$secro_secfl")
{				
	if (-e "$s_secro_fl_o1")
	{
		print "remove old file list (1) ... \n";
		system("rm -f $s_secro_fl_o1");
	}

	if (-e "$s_secro_fl_o2")
	{
		print "remove old file list (2) ... \n";
		system("rm -f $s_secro_fl_o2");
	}

	if (! -d $s_secro_out_dir)
	{
		print "secro secure file list out folder does not exist...\n";
		system("mkdir -p $s_secro_out_dir");
	}

	PrintDependency("$key_dir/IMG_AUTH_KEY.ini");
	PrintDependency("$cfg_dir/SFL_CFG.ini");
	PrintDependency($secro_secfl);
	system("./$sign_tool $key_dir/IMG_AUTH_KEY.ini $cfg_dir/SFL_CFG.ini $secro_secfl $s_secro_fl_o1");

	if (! -e "$s_secro_fl_o1")
	{
		die "sign failed. please check";
	}
	
	system("cp -f $s_secro_fl_o1 $s_secro_fl_o2");	
}
else
{
	print "file doesn't exist\n";
}

