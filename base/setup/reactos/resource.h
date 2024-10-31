#pragma once

/* Bitmaps */
#define IDB_WATERMARK       100
#define IDB_HEADER          101

/* Icons */
#define IDI_MAIN            3000
#define IDI_ROSICON         3001
#define IDI_WINICON         3002
#define IDI_DISKDRIVE       3003
#define IDI_PARTITION       3004


#define IDC_STATIC          -1

/* Dialogs */
#define IDD_STARTPAGE        2000
#define IDC_STARTTITLE       2001
#define IDC_WARNTEXT1        2002   // TEMPTEMP: TODO: Remove once we reach 0.5/Beta :)
#define IDC_WARNTEXT2        2003   // TEMPTEMP: TODO: Remove once we reach 0.5/Beta :)
#define IDC_WARNTEXT3        2004   // TEMPTEMP: TODO: Remove once we reach 0.5/Beta :)

#define IDD_TYPEPAGE         2010
#define IDC_INSTALL          2011
#define IDC_INSTALLTEXT      2012
#define IDC_UPDATE           2013
#define IDC_UPDATETEXT       2014

#define IDD_UPDATEREPAIRPAGE    2020
#define IDC_NTOSLIST            2021
#define IDC_SKIPUPGRADE         2022

#define IDD_DEVICEPAGE       2030
#define IDC_COMPUTER         2031
#define IDC_DISPLAY          2032
#define IDC_KEYBOARD         2033
#define IDC_KEYLAYOUT        2034

#define IDD_DRIVEPAGE        2040
#define IDC_PARTITION        2041
#define IDC_INITDISK         2042
#define IDC_PARTCREATE       2043
#define IDC_PARTFORMAT       2044
#define IDC_PARTDELETE       2045
#define IDC_DEVICEDRIVER     2046
#define IDC_PARTMOREOPTS     2047

#define IDD_SUMMARYPAGE      2050
#define IDC_INSTALLTYPE      2051
#define IDC_INSTALLSOURCE    2052
#define IDC_ARCHITECTURE     2053
// #define IDC_COMPUTER         2031
// #define IDC_DISPLAY          2032
// #define IDC_KEYBOARD         2033
#define IDC_DESTDRIVE        2054
// #define IDC_PATH             2071
#define IDC_CONFIRM_INSTALL  2055

#define IDD_PROCESSPAGE      2060
#define IDC_ACTIVITY         2061
#define IDC_ITEM             2062
#define IDC_PROCESSPROGRESS  2063

#define IDD_RESTARTPAGE      2070
#define IDC_FINISHTITLE      2071
#define IDC_RESTART_PROGRESS 2072

#define IDD_ADVINSTOPTS      2080
#define IDC_PATH             2081
#define IDC_INSTFREELDR      2082

#define IDD_PARTITION        2090
#define IDC_EDIT_PARTSIZE    2091
#define IDC_UPDOWN_PARTSIZE  2092
#define IDC_UNIT             2093
#define IDC_CHECK_MBREXTPART 2094
#define IDC_FS_STATIC        2095
#define IDC_FSTYPE           2096
#define IDC_CHECK_QUICKFMT   2097

#define IDD_FORMAT 2100


/* Strings */
#define IDS_CAPTION          5000
#define IDS_TYPETITLE        5001
#define IDS_TYPESUBTITLE     5002
#define IDS_DEVICETITLE      5003
#define IDS_DEVICESUBTITLE   5004
#define IDS_DRIVETITLE       5005
#define IDS_DRIVESUBTITLE    5006
#define IDS_SUMMARYTITLE     5007
#define IDS_SUMMARYSUBTITLE  5008
#define IDS_PROCESSTITLE     5009
#define IDS_PROCESSSUBTITLE  5010
#define IDS_RESTARTTITLE     5011
#define IDS_RESTARTSUBTITLE  5012
#define IDS_ABORTSETUP       5020
#define IDS_ABORTSETUP2      5021
#define IDS_NO_TXTSETUP_SIF  5022
#define IDS_INSTALLBTN       5030
#define IDS_VOLUME_NOFORMAT  5040

#define IDS_INSTALLATION_NAME   5100
#define IDS_INSTALLATION_PATH   5101
#define IDS_INSTALLATION_VENDOR 5102

#define IDS_PARTITION_NAME   5110
#define IDS_PARTITION_TYPE   5111
#define IDS_PARTITION_SIZE   5112
#define IDS_PARTITION_STATUS 5113

// WARNING: These IDs *MUST* stay in increasing order!
#define IDS_BOOTLOADER_NOINST       5120
#define IDS_BOOTLOADER_REMOVABLE    5121
#define IDS_BOOTLOADER_SYSTEM       5122    // For non-MBR disks
#define IDS_BOOTLOADER_MBRVBR       5123    // For MBR disks only
#define IDS_BOOTLOADER_VBRONLY      5124    // ""


/* Progress Strings */
#define IDS_FORMATTING_PROGRESS1    5200
#define IDS_FORMATTING_PROGRESS2    5201
#define IDS_CHECKING_PROGRESS1      5202
#define IDS_CHECKING_PROGRESS2      5203
#define IDS_COPYING                 5204
#define IDS_MOVING                  5205
#define IDS_RENAMING                5206
#define IDS_DELETING                5207
#define IDS_CONFIG_SYSTEM_PARTITION 5210
#define IDS_PREPARE_PARTITIONS      5211
#define IDS_PREPARE_FILES           5212
#define IDS_COPYING_FILES           5213
#define IDS_CREATE_REGISTRY         5214
#define IDS_UPDATE_REGISTRY         5215
// #define IDS_INSTALL_FINALIZE        5216
#define IDS_INSTALL_BOOTLOADER      5217

#define IDS_REG_DONE                    5220
#define IDS_REG_REGHIVEUPDATE           5221
#define IDS_REG_IMPORTFILE              5222
#define IDS_REG_DISPLAYSETTINGSUPDATE   5223
#define IDS_REG_LOCALESETTINGSUPDATE    5224
#define IDS_REG_ADDKBLAYOUTS            5225
#define IDS_REG_KEYBOARDSETTINGSUPDATE  5226
#define IDS_REG_CODEPAGEINFOUPDATE      5227
#define IDS_REG_UNKNOWN                 5228


/* Error Strings */
#define IDS_ERROR_INVALID_INSTALLDIR_CHAR_TITLE 5300
#define IDS_ERROR_INVALID_INSTALLDIR_CHAR       5301

// ERROR_DIRECTORY_NAME
#define IDS_ERROR_DIRECTORY_NAME_TITLE          5302
#define IDS_ERROR_DIRECTORY_NAME                5303

// ERROR_CREATE_PARTITION // TODO?
#define IDS_ERROR_CREATE_PARTITION_TITLE        5304
#define IDS_ERROR_CREATE_PARTITION              5305

#define IDS_WARN_DELETE_PARTITION_TITLE         5306
#define IDS_WARN_DELETE_PARTITION               5307
#define IDS_WARN_DELETE_MBR_EXTENDED_PARTITION  5308

// ERROR_WRITE_PTABLE
#define IDS_ERROR_WRITE_PTABLE          5309

// ERROR_SYSTEM_PARTITION_NOT_FOUND
// ERROR_SYSTEM_PARTITION // TODO
#define IDS_ERROR_SYSTEM_PARTITION      5310

// ERROR_FORMAT_UNRECOGNIZED_VOLUME // TODO
#define IDS_ERROR_FORMAT_UNRECOGNIZED_VOLUME    5311

// ERROR_COULD_NOT_FORMAT // TODO?
#define IDS_ERROR_COULD_NOT_FORMAT      5312

// ERROR_FORMATTING_PARTITION
#define IDS_ERROR_FORMATTING_PARTITION  5313

// ERROR_COULD_NOT_CHECK // TODO?
#define IDS_ERROR_COULD_NOT_CHECK       5314

// ERROR_CHECKING_PARTITION // TODO
#define IDS_ERROR_CHECKING_PARTITION    5315

// ERROR_WRITE_BOOT
#define IDS_ERROR_WRITE_BOOT        5316
// ERROR_INSTALL_BOOTCODE
#define IDS_ERROR_INSTALL_BOOTCODE  5317
#define IDS_ERROR_INSTALL_BOOTCODE_REMOVABLE    5318
// STATUS_NOT_SUPPORTED for bootloader
#define IDS_ERROR_BOOTLDR_ARCH_UNSUPPORTED      5319
// ERROR_BOOTLDR_FAILED // TODO
#define IDS_ERROR_BOOTLDR_FAILED    5320
