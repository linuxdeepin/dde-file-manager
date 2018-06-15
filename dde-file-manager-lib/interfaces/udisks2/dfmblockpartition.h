/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DFMBLOCKPARTITION_H
#define DFMBLOCKPARTITION_H

#include <dfmblockdevice.h>

DFM_BEGIN_NAMESPACE

class DFMBlockPartitionPrivate;
class DFMBlockPartition : public DFMBlockDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMBlockPartition)

    Q_PROPERTY(qulonglong flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(bool isContained READ isContained NOTIFY isContainedChanged)
    Q_PROPERTY(bool isContainer READ isContainer NOTIFY isContainerChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(uint number READ number NOTIFY numberChanged)
    Q_PROPERTY(qulonglong offset READ offset NOTIFY offsetChanged)
    Q_PROPERTY(qulonglong size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QString table READ table)
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)
    // MBR Partition Type
    Q_PROPERTY(Type eType READ eType NOTIFY eTypeChanged)
    // GPT Partition Type
    Q_PROPERTY(GUIDType guidType READ guidType NOTIFY guidTypeChanged)
    Q_PROPERTY(QString UUID READ UUID NOTIFY UUIDChanged)

public:
    enum Type {
        Empty = 0x0,
        FAT12Type = 0x1,
        XENIX_root = 0x2,
        XENIX_usr = 0x3,
        FAT16_Less_32M = 0x4,
        Extended = 0x5,
        FAT16Type = 0x6,
        HPFS_NTFS = 0x7,
        AIX = 0x8,
        AIX_bootable = 0x9,
        OS2_Boot_Manager = 0xa,
        Win95_FAT32 = 0xb,
        Win95_FAT32_LBA = 0xc,
        Win95_FAT16_LBA = 0xe,
        Win95_Extended_LBA = 0xf,
        OPUS = 0x10,
        Hidden_FAT12 = 0x11,
        Compaq_diagnostics = 0x12,
        Hidden_FAT16_Less_32M = 0x14,
        Hidden_FAT16 = 0x16,
        Hidden_HPFS_or_NTFS = 0x17,
        AST_SmartSleep = 0x18,
        Hidden_Win95_FAT32 = 0x1b,
        Hidden_Win95_FAT32_LBA = 0x1c,
        Hidden_Win95_FAT16_LBA = 0x1e,
        NEC_DOS = 0x24,
        Plan9 = 0x39,
        PartitionMagic_recovery = 0x3c,
        Venix_80286 = 0x40,
        PPC_PReP_Boot = 0x41,
        SFS = 0x42,
        QNX4_dot_x = 0x4d,
        QNX4_dot_x_2nd_part = 0x4e,
        QNX4_dot_x_3rd_part = 0x4f,
        OnTrack_DM = 0x50,
        OnTrack_DM6_Aux1 = 0x51,
        CP_M = 0x52,
        OnTrack_DM6_Aux3 = 0x53,
        OnTrackDM6 = 0x54,
        EZ_Drive = 0x55,
        Golden_Bow = 0x56,
        Priam_Edisk = 0x5c,
        SpeedStor = 0x61,
        GNU_HURD_or_SysV = 0x63,
        Novell_Netware_286 = 0x64,
        Novell_Netware_386 = 0x65,
        DiskSecure_Multi_Boot = 0x70,
        PC_IX = 0x75,
        Old_Minix = 0x80,
        Minix_old_Linux = 0x81,
        Linux_swap = 0x82,
        Linux = 0x83,
        OS2_hidden_C_drive = 0x84,
        Linux_extended = 0x85,
        NTFS_volume_set_1 = 0x86,
        NTFS_volume_set_2 = 0x87,
        Linux_LVM = 0x8e,
        Amoeba = 0x93,
        Amoeba_BBT = 0x94,
        BSD_OS = 0x9f,
        IBM_Thinkpad_hibernation = 0xa0,
        FreeBSD = 0xa5,
        OpenBSD = 0xa6,
        NeXTSTEP = 0xa7,
        NetBSD = 0xa8,
        BSDI_fs = 0xa9,
        BSDI_swap = 0xb7,
        Boot_Wizard_hidden = 0xb8,
        DRDOS_sec_FAT12 = 0xbb,
        DRDOS_sec_FAT16_Less_32M = 0xc1,
        DRDOS_sec_FAT16 = 0xc4,
        Syrinx = 0xc6,
        Non_FS_data = 0xc7,
        CP_M_CTOS_dot_dot_dot = 0xda,
        Dell_Utility = 0xdb,
        BootIt = 0xde,
        DOS_access = 0xdf,
        DOS_R_O = 0xe1,
        SpeedStor_1 = 0xe3,
        BeOS_fs = 0xe4,
        EFI_GPT = 0xeb,
        EFI_FAT12_16_32 = 0xee,
        Linux_PA_RISC_boot = 0xef,
        SpeedStor_2 = 0xf0,
        SeppdStor_3 = 0xf4,
        DOS_secondary = 0xf2,
        Linux_raid_autodetect = 0xfd,
        LANstep = 0xfe,
        BBT = 0xff,
        Unknow
    };

    Q_ENUM(Type)

    enum GUIDType {
        InvalidUUID = 0,
        GUIDTypeBegin = 1,
        // None
        Unused_None = GUIDTypeBegin,
        MBR_PS_None = 2,
        EFI_SP_None = 3,
        BIOS_BP_None = 4,
        iFFS_None = 5,
        Sony_BP_None = 6,
        Lenove_BP_None = 7,
        // Windows
        MSR_Win = 8,
        BasicData_Win = 9,
        LDM_Win = 10,
        LDM_DP_Win = 11,
        WRE_Win = 12,
        IBM_GPFS_Win = 13,
        SSP_Win = 14,
        // HP-UX
        DP_HPUX = 15,
        SP_HPUX = 16,
        // Linux
        LFD_Linux = 17,
        RAID_P_Linux = 18,
        RP_x86_Linux = 19,
        RP_x86_64_Linux = 20,
        RP_32bit_ARM_Linux = 21,
        RP_64bit_ARM_Linux = 22,
        SP_Linux = 23,
        LVM_P_Linux = 24,
        Home_P_Linux = 25,
        Srv_P_Linux = 26,
        Plain_DC_P_Linux = 27,
        LUKS_P_Linux = 28,
        Reserved_Linux = 29,
        // FreeBSD
        BP_FreeBSD = 30,
        DP_FreeBSD = 31,
        SP_FreeBSD = 32,
        UFS_P_FreeBSD = 33,
        VVM_P_FreeBSD = 34,
        ZFS_P_FreeBSD = 35,
        // macOS Darwin
        HFS_PLUS_P_Mac = 36,
        UFS_Mac = 37,
        ZFS_Mac = 38,
        RAID_P_Mac = 39,
        RAID_P_Offline_Mac = 40,
        BP_Mac = 41,
        Label_Mac = 42,
        TV_RP_Mac = 43,
        CS_P_Mac = 44,
        SoftRAID_Status_Mac = 45,
        SoftRAID_Scratch_Mac = 46,
        SoftRAID_Volume_Mac = 47,
        SoftRAID_Cache_Mac = 48,
        // Solaris illumos
        BP_Solaris = 49,
        RP_Solaris = 50,
        SP_Solaris = 51,
        Backup_P_Solaris = 52,
        Usr_P_Solaris = ZFS_Mac,
        Var_P_Solaris = 53,
        Home_P_Solaris = 54,
        AS_Solaris = 55,
        Reserved_Solaris = 56,
        // NetBSD
        SP_NetBSD = 57,
        FFS_P_NetBSD = 58,
        LFS_P_NetBSD = 59,
        RAID_P_NetBSD = 60,
        CP_NetBSD = 61,
        EP_NetBSD = 62,
        // ChromeOS
        Kernel_ChromeOS = 63,
        Rootfs_ChromeOS = 64,
        FU_ChromeOS = 65,
        // Haiku
        BFS_Haiku = 66,
        // MidnightBSD
        BP_MidnightBSD = 67,
        DP_MidnightBSD = 68,
        SP_MidnightBSD = 69,
        UFS_P_MidnightBSD = 70,
        VVM_P_MidnightBSD = 71,
        ZFS_P_MidnightBSD = 72,
        // Ceph
        Journal_Ceph = 73,
        DC_EJ_Ceph = 74,
        OSD_Ceph = 75,
        DC_OSD_Ceph = 76,
        DIC_Ceph = 77,
        DC_DIC_Ceph = 78,
        // OpenBSD
        DP_OpenBSD = 79,
        // QNX
        PAFS_QNX = 80,
        // Plan9
        Partition_Plan9 = 81,
        // VMware ESX
        Vmkcore_VMware = 82,
        VMFS_VMware = 83,
        Reserved_VMware = 84,
        // Android-IA
        Bootloader_Android = 85,
        Bottloader2_Android = 86,
        Boot_Android = 87,
        Recovery_Android = 88,
        Misc_Android = 89,
        Metadata_Android = 90,
        System_Android = 91,
        Cache_Android = 92,
        Data_Android = 93,
        Persistent_Android = 94,
        Factory_Android = 95,
        Fastboot_Android = 96,
        OEM_Android = 97,
        // Open Network Install Environment (ONIE)
        Boot_ONIE = 98,
        Config_ONIE = 99,
        // PowerPC
        Boot_PowerPC = 100,
        // freedesktop.org OSes (Linux, etc.)
        SBLC_OSes = 101,
        // Atari TOS
        BD_P_Atari = 102,
        GUIDTypeEnd,
        UnknowUUID
    };

    Q_ENUM(GUIDType)

    qulonglong flags() const;
    bool isContained() const;
    bool isContainer() const;
    QString name() const;
    uint number() const;
    qulonglong offset() const;
    qulonglong size() const;
    QString table() const;
    QString type() const;
    Type eType() const;
    GUIDType guidType() const;
    QString UUID() const;

    static QString typeDescription(Type type);
    static QString guidTypeDescription(GUIDType type);

public Q_SLOTS: // METHODS
    void deletePartition(const QVariantMap &options);
    void resize(qulonglong size, const QVariantMap &options);
    void setFlags(qulonglong flags, const QVariantMap &options);
    void setName(const QString &name, const QVariantMap &options);
    void setType(const QString &type, const QVariantMap &options);
    void setType(Type type, const QVariantMap &options);

Q_SIGNALS:
    void flagsChanged(qulonglong flags);
    void isContainedChanged(bool isContained);
    void isContainerChanged(bool isContainer);
    void nameChanged(const QString &name);
    void numberChanged(uint number);
    void offsetChanged(qulonglong offset);
    void sizeChanged(qulonglong size);
    void typeChanged(const QString &type);
    void eTypeChanged();
    void UUIDChanged(const QString &UUID);
    void guidTypeChanged();

private:
    explicit DFMBlockPartition(const QString &path, QObject *parent = nullptr);

    friend class DFMDiskManager;
};

DFM_END_NAMESPACE

#endif // DFMBLOCKPARTITION_H
