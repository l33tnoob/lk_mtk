#!/usr/bin/perl

use lib "vendor/mediatek/proprietary/scripts/sign-image";
use lib "device/mediatek/build/build/tools";
use pack_dep_gen;
PrintDependModule($0);

##########################################################
# Parse ProjectConfig.mk
##########################################################
my $TARGET_PRODUCT = $ENV{"TARGET_PRODUCT"};
my $PROJECT = substr $TARGET_PRODUCT, (index($TARGET_PRODUCT, 'full_') + 5);
$PROJECT =~ s/\_64//;

=do not parse ProjectConfig.mk in AOSP branch
my $ProjectConfig = "device/mediatek/$PROJECT/ProjectConfig.mk";

open (FILE_HANDLE, "<$ProjectConfig") or die "cannot open $ProjectConfig\n";
while (<FILE_HANDLE>)
{
  if (/^(\S+)\s*=\s*(\S+)/)
  {
    $ENV{$1} = $2;
  }
}
close FILE_HANDLE;
=cut

##########################################################
# Initialize Variables
##########################################################
my $MTK_SECURITY_SW_SUPPORT = $ENV{"MTK_SECURITY_SW_SUPPORT"};
my $modem_encode = $ENV{"MTK_SEC_MODEM_ENCODE"};
my $modem_auth = $ENV{"MTK_SEC_MODEM_AUTH"};
my $OUT_DIR = $ENV{"OUT"};
my $MTK_ROOT_CUSTOM_OUT = "$OUT_DIR/obj/CUSTGEN/custom";
my $SYSTEM_MODEM = "$OUT_DIR/system/etc/firmware";
my $sml_dir = "vendor/mediatek/proprietary/custom/$PROJECT/security/sml_auth";
my $cipher_tool = "vendor/mediatek/proprietary/scripts/ciphertool/CipherTool";
my $sign_tool = "vendor/mediatek/proprietary/scripts/sign-image/SignTool.sh";

##########################################################
# Check Parameter
##########################################################

=comment begin
print "\n\n";
print "********************************************\n";
print " CHECK PARAMETER \n";
print "********************************************\n";

if (${modem_auth} eq "yes")
{
	if (${modem_encode} eq "no")
	{
		die "Error! MTK_SEC_MODEM_AUTH is 'yes' but MTK_SEC_MODEM_ENCODE is 'no'\n";
	}
}

if (${MTK_SECURITY_SW_SUPPORT} ne "yes")
{
	$modem_encode = "no";
	$modem_auth = "no";
}
=cut
 
 
## always make MTK_SECURITY_SW_SUPPORT, modem_encode and modem_auth yes in AOSP project since ProjectConfig.mk will be no longer available.
${MTK_SECURITY_SW_SUPPORT} = "yes";
$modem_encode = "yes";
$modem_auth = "yes";
print "MTK_SEC_MODEM_AUTH,  MTK_SEC_MODEM_ENCODE, MTK_SECURITY_SW_SUPPORT will always be yes for AOSP project\n";

if (! -d "$MTK_ROOT_CUSTOM_OUT"){
    system("mkdir -p $MTK_ROOT_CUSTOM_OUT");
}

if (! -d "$SYSTEM_MODEM"){
    die "$SYSTEM_MODEM does not exist\n ";
}

print "parameter check pass (2 MDs)\n";
print "MTK_SEC_MODEM_AUTH    =  $modem_auth\n";
print "MTK_SEC_MODEM_ENCODE  =  $modem_encode\n";
print "MTK_SECURITY_SW_SUPPORT  =  $MTK_SECURITY_SW_SUPPORT\n";
print "MTK_ROOT_CUSTOM_OUT  =  $MTK_ROOT_CUSTOM_OUT\n";

##########################################################
# Process Modem Image
##########################################################

my $md_load = "$MTK_ROOT_CUSTOM_OUT/modem/modem.img";
my $b_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/modem.img.bak";
my $c_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/cipher_modem.img";
my $s_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/signed_modem.img";


#opendir(DIR, "$MTK_ROOT_CUSTOM_OUT/modem");
opendir(DIR, "$SYSTEM_MODEM");
@files = grep(/\.img/,readdir(DIR));
foreach my $file (@files)
{
	#$md_load = "$MTK_ROOT_CUSTOM_OUT/modem/$file";
	#$b_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/$file.bak";
	#$c_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/cipher_$file";
	#$s_md_load = "$MTK_ROOT_CUSTOM_OUT/modem/signed_$file";
	$md_load = "$SYSTEM_MODEM/$file";
	$b_md_load = "$MTK_ROOT_CUSTOM_OUT/$file.bak";
	$c_md_load = "$SYSTEM_MODEM/cipher_$file";
	$s_md_load = "$SYSTEM_MODEM/signed_$file";
	&process_modem_image;
}
closedir(DIR);

sub process_modem_image
{
	print "\n\n";
	print "********************************************\n";
	print " PROCESS MODEM IMAGE ($md_load)\n";
	print "********************************************\n";	
	
	if (-e "$b_md_load")
	{
		print "$md_load already processed ... \n";
	}
	else
	{
		if (-e "$md_load")
		{
			system("cp -f $md_load $b_md_load") == 0 or die "can't backup modem image";

			########################################		
			# Encrypt and Sign Modem Image
			########################################		
			if (${modem_auth} eq "yes")
			{
				if (${modem_encode} eq "yes")
				{
					PrintDependency("$sml_dir/SML_ENCODE_KEY.ini");
					PrintDependency("$sml_dir/SML_ENCODE_CFG.ini");
					PrintDependency($md_load);
					PrintDependency($cipher_tool);
					system("./$cipher_tool ENC $sml_dir/SML_ENCODE_KEY.ini $sml_dir/SML_ENCODE_CFG.ini $md_load $c_md_load") == 0 or die "Cipher Tool return error\n";
				
					if(-e "$c_md_load")
					{
						system("rm -f $md_load") == 0 or die "can't remove original modem binary\n";
						system("mv -f $c_md_load $md_load") == 0 or die "can't generate cipher modem binary\n";
					}
				}
				PrintDependency("$sml_dir/SML_AUTH_KEY.ini");
				PrintDependency("$sml_dir/SML_AUTH_CFG.ini");
				PrintDependency("$md_load");
				PrintDependency($sign_tool);
				system("./$sign_tool $sml_dir/SML_AUTH_KEY.ini $sml_dir/SML_AUTH_CFG.ini $md_load $s_md_load");
	
				if(-e "$s_md_load")
				{
					system("rm -f $md_load") == 0 or die "can't remove original modem binary\n";
					system("mv -f $s_md_load $md_load") == 0 or die "can't generate signed modem binary\n";
					system("rm -f $md_load.hdr $md_load.sig") == 0 or die "can't remove additional sig \n";
				}
			}
			else
			{
				print "doesn't execute Sign Tool and Cipher Tool ... \n";
			}
		}
		else
		{
			print "$md_load is not existed\n";			
		}
	}
}
