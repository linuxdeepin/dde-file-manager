// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 回归测试：mountcontrol 服务（bug 277301）。
// 服务以可执行文件构建，无库可链，此处 unity-include 被测服务实现源码。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QDir>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QUrl>

#include <libmount/libmount.h>
#include <pwd.h>

#include "services/mountcontrol/service_mountcontrol_global.h"
#include "services/mountcontrol/mounthelpers/cifsmounthelper.h"
#include "services/mountcontrol/mounthelpers/cifsmounthelper_p.h"

// unity-include 被测实现（服务无可链接目标）
#include "services/mountcontrol/mounthelpers/cifsmounthelper.cpp"
#include "services/mountcontrol/mounthelpers/commonmounthelper.cpp"
#include "services/common/polkit/policykithelper.cpp"

// 日志类别符号（真实服务在 mountcontroldbus.cpp 中注册）
SERVICEMOUNTCONTROL_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(SERVICEMOUNTCONTROL_NAMESPACE)
SERVICEMOUNTCONTROL_END_NAMESPACE

SERVICEMOUNTCONTROL_USE_NAMESPACE

namespace {
// libmount 桩的全局状态
bool g_fsFound { false };
QByteArray g_mpt;
QByteArray g_fstype;
QByteArray g_opts;
uint g_invokerUid { 1000 };
QByteArray g_pwdName { "ut-user" };
struct passwd g_pwd;

void resetMountTable(bool found, const QByteArray &mpt, const QByteArray &fstype, const QByteArray &opts)
{
    g_fsFound = found;
    g_mpt = mpt;
    g_fstype = fstype;
    g_opts = opts;
}
}   // namespace

class CifsMountHelperTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetMountTable(false, "", "", "");
        g_invokerUid = 1000;

        // 全部 libmount 入口打桩，构造受控的 mtab
        stub.set_lamda(&mnt_new_table, []() -> libmnt_table * {
            return reinterpret_cast<libmnt_table *>(0x1);
        });
        stub.set_lamda(&mnt_free_table, [](libmnt_table *) {});
        stub.set_lamda(&mnt_table_parse_mtab, [](libmnt_table *, const char *) -> int { return 0; });
        stub.set_lamda(&mnt_table_find_source,
                       [](libmnt_table *, const char *, int) -> libmnt_fs * {
            return g_fsFound ? reinterpret_cast<libmnt_fs *>(0x2) : nullptr;
        });
        stub.set_lamda(&mnt_table_find_target,
                       [](libmnt_table *, const char *, int) -> libmnt_fs * {
            return nullptr;   // 仅走 source 分支，保持确定性
        });
        stub.set_lamda(&mnt_fs_get_target, [](libmnt_fs *) -> const char * {
            return g_mpt.constData();
        });
        stub.set_lamda(&mnt_fs_get_fstype, [](libmnt_fs *) -> const char * {
            return g_fstype.constData();
        });
        stub.set_lamda(&mnt_fs_get_options, [](libmnt_fs *) -> const char * {
            return g_opts.constData();
        });

        // getpwuid 打桩，避免依赖真实系统用户
        stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * {
            g_pwd.pw_name = g_pwdName.data();
            g_pwd.pw_uid = g_invokerUid;
            g_pwd.pw_gid = g_invokerUid;
            return &g_pwd;
        });

        // invokerUid 打桩，隔离 DBus 调用方上下文
        stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) -> uint {
            return g_invokerUid;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// PMS:277301 CifsMountHelper::checkMount 挂载点正则误写为 ^/media/.*/smbmounts/，
// 而 cifs 挂载实际发生在 /run/media/$user/smbmounts（FHS 约定），
// 导致合法挂载被判为 kNotMountByDaemon，用户无法卸载自己挂载的共享目录；
// 修复为 ^(?:/media|/run/media)/.*，两种根路径都必须放行。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_RunMediaSmbmountsAccepted)
{
    resetMountTable(true,
                    "/run/media/ut-user/smbmounts/share",
                    "cifs",
                    "rw,relatime,uid=1000,gid=1000");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kOkay));
    EXPECT_EQ(mpt, "/run/media/ut-user/smbmounts/share");
}

// PMS:277301 兼容历史 /media 根路径的挂载点，修复后仍需放行。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_MediaSmbmountsAccepted)
{
    resetMountTable(true,
                    "/media/ut-user/smbmounts/share",
                    "cifs",
                    "rw,uid=1000");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kOkay));
}

// PMS:277301 非 /media、/run/media 根路径的挂载点必须拒绝（kNotMountByDaemon）。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_OtherRootRejected)
{
    resetMountTable(true, "/home/ut-user/smbmounts/share", "cifs", "uid=1000");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kNotMountByDaemon));
}

// PMS:277301 仅在挂载点根路径合法后校验文件系统类型：非 cifs 返回 kNotCifs。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_NonCifsRejected)
{
    resetMountTable(true, "/run/media/ut-user/smbmounts/share", "ext4", "uid=1000");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kNotCifs));
}

// PMS:277301 无 uid 挂载选项时不可卸载他人挂载（kNotOwner）。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_NoUidOptionRejected)
{
    resetMountTable(true,
                    "/run/media/ut-user/smbmounts/share",
                    "cifs",
                    "rw,relatime,vers=default");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kNotOwner));
}

// PMS:277301 uid 与调用者不一致（他人挂载）必须拒绝（kNotOwner）。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_UidMismatchRejected)
{
    resetMountTable(true,
                    "/run/media/ut-user/smbmounts/share",
                    "cifs",
                    "rw,uid=999");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kNotOwner));
}

// PMS:277301 mtab 中找不到挂载项时返回 kNotExist。
TEST_F(CifsMountHelperTest, BUG277301_CheckMount_NotExist)
{
    resetMountTable(false, "", "", "");

    CifsMountHelper helper(nullptr);
    QString mpt;
    const int status = static_cast<int>(helper.checkMount("//server/share", mpt));
    EXPECT_EQ(status, static_cast<int>(CifsMountHelper::kNotExist));
}

// PMS:277301 mkdirMountRootPath 原实现用 ::mkdir 只能创建一级目录，
// /media/$user/smbmounts 需要多级创建（QDir::mkpath），否则挂载根目录创建失败。
TEST_F(CifsMountHelperTest, BUG277301_MkdirMountRootPath_CreatesMultiLevelPath)
{
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString root = temp.path() + "/level1/level2/smbmounts";

    stub.set_lamda(&CifsMountHelper::mountRoot, [&temp](CifsMountHelper *) -> QString {
        return temp.path() + "/level1/level2/smbmounts";
    });

    CifsMountHelper helper(nullptr);
    EXPECT_TRUE(helper.mkdirMountRootPath());
    EXPECT_TRUE(QDir(root).exists());
}

// PMS:277301 挂载根目录已存在时应直接成功。
TEST_F(CifsMountHelperTest, BUG277301_MkdirMountRootPath_ExistingPathSucceeds)
{
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString root = temp.path() + "/smbmounts";
    ASSERT_TRUE(QDir().mkpath(root));

    stub.set_lamda(&CifsMountHelper::mountRoot, [&temp](CifsMountHelper *) -> QString {
        return temp.path() + "/smbmounts";
    });

    CifsMountHelper helper(nullptr);
    EXPECT_TRUE(helper.mkdirMountRootPath());
    EXPECT_TRUE(QDir(root).exists());
}

// PMS:277301 unmount 在调用方非挂载所有者（kNotOwner）时必须失败，
// 返回 kNotOwnerOfMount 错误码且不执行真实 umount。
TEST_F(CifsMountHelperTest, BUG277301_Unmount_NonOwnerRejected)
{
    stub.set_lamda(&CifsMountHelper::checkMount,
                   [](CifsMountHelper *, const QString &, QString &mpt) -> CifsMountHelper::MountStatus {
        mpt = "/run/media/ut-user/smbmounts/share";
        return CifsMountHelper::kNotOwner;
    });

    CifsMountHelper helper(nullptr);
    const QVariantMap ret = helper.unmount("smb://server/share", {});
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kNotOwnerOfMount);
}

// PMS:277301 unmount 对未挂载路径返回 kMountNotExist 错误码。
TEST_F(CifsMountHelperTest, BUG277301_Unmount_NotExistRejected)
{
    stub.set_lamda(&CifsMountHelper::checkMount,
                   [](CifsMountHelper *, const QString &, QString &) -> CifsMountHelper::MountStatus {
        return CifsMountHelper::kNotExist;
    });

    CifsMountHelper helper(nullptr);
    const QVariantMap ret = helper.unmount("smb://server/share", {});
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kMountNotExist);
}

// PMS:277301 convertArgs 需按约定生成 cifs 挂载参数：
// 凭据、uid/gid 归属、iocharset/actimeo 默认值、vers 回退 default、超时映射。
TEST_F(CifsMountHelperTest, BUG277301_ConvertArgs_BuildsExpectedOptions)
{
    stub.set_lamda(&CifsMountHelper::overrideOptions, [](CifsMountHelper *) -> QVariantMap {
        return {};
    });

    CifsMountHelper helper(nullptr);
    const QVariantMap opts {
        { MountOptionsField::kUser, "u1" },
        { MountOptionsField::kPasswd, "p1" },
        { MountOptionsField::kDomain, "dom1" },
        { MountOptionsField::kPort, 445 },
        { MountOptionsField::kIp, "1.2.3.4" },
        { MountOptionsField::kTimeout, 30 }
    };
    const std::string args = helper.convertArgs(opts);

    EXPECT_NE(args.find("user=u1"), std::string::npos);
    EXPECT_NE(args.find("pass=p1"), std::string::npos);
    EXPECT_NE(args.find("dom=dom1"), std::string::npos);
    EXPECT_NE(args.find("port=445"), std::string::npos);
    EXPECT_NE(args.find("ip=1.2.3.4"), std::string::npos);
    EXPECT_NE(args.find("uid=1000"), std::string::npos);
    EXPECT_NE(args.find("gid=1000"), std::string::npos);
    EXPECT_NE(args.find("iocharset=utf8"), std::string::npos);
    EXPECT_NE(args.find("actimeo=5"), std::string::npos);
    EXPECT_NE(args.find("vers=default"), std::string::npos);
    // 无 tryWaitReconn 时 handletimeout = timeout * 1000 ms
    EXPECT_NE(args.find("handletimeout=30000"), std::string::npos);
    EXPECT_EQ(args.find("wait_reconnect_timeout"), std::string::npos);
}

// PMS:277301 无凭据时仍需携带空 user=（保持参数顺序与兼容性）。
TEST_F(CifsMountHelperTest, BUG277301_ConvertArgs_EmptyCredentialsKeepUserField)
{
    stub.set_lamda(&CifsMountHelper::overrideOptions, [](CifsMountHelper *) -> QVariantMap {
        return {};
    });

    CifsMountHelper helper(nullptr);
    const std::string args = helper.convertArgs({});

    EXPECT_NE(args.find("user="), std::string::npos);
    EXPECT_EQ(args.find("pass="), std::string::npos);
}
