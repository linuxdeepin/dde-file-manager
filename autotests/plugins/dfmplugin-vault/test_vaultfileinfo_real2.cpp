// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "stubext.h"

#include "fileutils/vaultfileinfo.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"

#include <dfm-io/dfmio_utils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static QString gVaultBaseTemp;

QString fakeBuildFilePath(const char *segment, ...)
{
    QStringList segs;
    if (segment) {
        QString first(segment);
        if (!gVaultBaseTemp.isEmpty() && first == QDir::homePath() + QString("/.config/Vault")) {
            first = gVaultBaseTemp;
        }
        segs << first;
    }

    va_list args;
    va_start(args, segment);
    const char *arg = nullptr;
    while ((arg = va_arg(args, const char *)) != nullptr) {
        segs << QString(arg);
    }
    va_end(args);

    QString path;
    for (const QString &s : segs) {
        QString part = s;
        while (part.endsWith('/')) {
            part.chop(1);
        }
        if (part.isEmpty() || part == "/")
            continue;
        if (path.isEmpty()) {
            path = part;
        } else {
            path += QDir::separator() + part;
        }
    }
    if (segs.isEmpty())
        return QString();
    if (segs.first().startsWith("/"))
        path = QString("/") + path;
    return path;
}

class VaultFileInfoReal2 : public testing::Test
{
public:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid());
        gVaultBaseTemp = tempDir->path();

        // Note: buildFilePath is a C-style variadic function and cannot be stubbed
        // via stub_ext. Instead, VaultFileInfo operations will use real temp paths.

        using StateFunc = VaultState (FileEncryptHandle::*)(const QString &, bool) const;
        stub.set_lamda(static_cast<StateFunc>(&FileEncryptHandle::state),
                       [](FileEncryptHandle *, const QString &, bool) -> VaultState { return kEncrypted; });

        // Redirect vault URLs to the temp dir so InfoFactory can create a real
        // proxy FileInfo for the underlying local file.
        stub.set_lamda(&VaultHelper::vaultToLocalUrl, [](const QUrl &url) -> QUrl {
            return QUrl::fromLocalFile(gVaultBaseTemp + url.path());
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        gVaultBaseTemp.clear();
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(VaultFileInfoReal2, RefreshAndExtendAttributes)
{
    QTemporaryFile tmpFile(tempDir->path() + "/XXXXXX.txt");
    ASSERT_TRUE(tmpFile.open());
    tmpFile.close();

    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/" + QFileInfo(tmpFile.fileName()).fileName());

    VaultFileInfo info(url);
    info.refresh();
    EXPECT_FALSE(info.extendAttributes(ExtInfoType::kSizeFormat).isNull());
}

TEST_F(VaultFileInfoReal2, ExtraPropertiesAndViewTip)
{
    QTemporaryDir subDir(tempDir->path() + "/sub");
    ASSERT_TRUE(subDir.isValid());

    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/sub");

    VaultFileInfo info(url);
    EXPECT_NO_THROW(info.extraProperties());
    EXPECT_NO_THROW(info.viewOfTip(FileInfo::ViewType::kEmptyDir));
}
