// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QCoreApplication>

#include "stubext.h"
#include "trashfileinfo.h"
#include "utils/trashcorehelper.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-io/dfileinfo.h>

using namespace dfmplugin_trashcore;
DFMBASE_USE_NAMESPACE

namespace {
// Creates a real trashed entry via gio(1) and returns its trash URL.
// Returns an invalid URL when the environment does not allow trashing.
QUrl createTrashedFile(const QByteArray &payload)
{
    // gio(1) refuses to trash files on internal mounts (e.g. /tmp); use $HOME.
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString path = dir + "/ut-dfmplugin-trashcore-" + QString::number(QCoreApplication::applicationPid())
                         + "-" + QString::number(QRandomGenerator::global()->generate());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return QUrl();
    file.write(payload);
    file.flush();
    file.close();

    if (QProcess::execute("gio", { "trash", path }) != 0)
        QFile::remove(path);

    QProcess proc;
    proc.start("gio", { "list", "trash:///" });
    if (!proc.waitForFinished(5000))
        return QUrl();
    const QString base = QFileInfo(path).fileName();
    const QStringList names = QString::fromUtf8(proc.readAllStandardOutput())
                                  .split('\n', Qt::SkipEmptyParts);
    for (const QString &n : names) {
        if (n.trimmed() == base)
            return QUrl("trash:///" + base);
    }
    return QUrl();
}

void removeTrashedFile(const QUrl &trashUrl)
{
    const QString base = trashUrl.fileName();
    const QString filesRoot = DFMBASE_NAMESPACE::StandardPaths::location(
            DFMBASE_NAMESPACE::StandardPaths::kTrashLocalFilesPath);
    QFile::remove(filesRoot + "/" + base);
    QFile::remove(filesRoot + "/../info/" + base + ".trashinfo");
}
}

class TrashFileInfoRealTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // TrashFileInfo resolves a proxy FileInfo for the real location; make
        // the factory able to create file infos for local urls.
        DFMBASE_NAMESPACE::UrlRoute::regScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile, "/");
        DFMBASE_NAMESPACE::InfoFactory::regClass<DFMBASE_NAMESPACE::SyncFileInfo>(
                DFMBASE_NAMESPACE::Global::Scheme::kFile);
    }

    void TearDown() override
    {
        stub.clear();
        if (trashedUrl.isValid())
            removeTrashedFile(trashedUrl);
    }

    stub_ext::StubExt stub;
    QUrl trashedUrl;
};

TEST_F(TrashFileInfoRealTest, RealTrashedFile_InitTarget)
{
    trashedUrl = createTrashedFile("unit test payload for trash core\n");
    if (!trashedUrl.isValid())
        GTEST_SKIP() << "gio trash not available in this environment";

    TrashFileInfo info(trashedUrl);

    // initTarget() resolves the real location behind the trash entry.
    const QUrl target = info.urlOf(DFMBASE_NAMESPACE::FileInfo::FileUrlInfoType::kRedirectedFileUrl);
    EXPECT_TRUE(target.isValid());
    EXPECT_TRUE(target.isLocalFile());
    EXPECT_TRUE(QFile::exists(target.toLocalFile()));

    // The original path is the pre-deletion location; the file no longer
    // lives there after trashing.
    const QUrl original = info.urlOf(DFMBASE_NAMESPACE::FileInfo::FileUrlInfoType::kOriginalUrl);
    EXPECT_FALSE(original.path().isEmpty());

    EXPECT_EQ(info.urlOf(DFMBASE_NAMESPACE::FileInfo::FileUrlInfoType::kUrl), trashedUrl);
}

TEST_F(TrashFileInfoRealTest, RealTrashedFile_Attributes)
{
    trashedUrl = createTrashedFile("unit test payload for attributes\n");
    if (!trashedUrl.isValid())
        GTEST_SKIP() << "gio trash not available in this environment";

    TrashFileInfo info(trashedUrl);
    const auto kUrl = DFMBASE_NAMESPACE::FileInfo::FileUrlInfoType::kUrl;

    EXPECT_TRUE(info.exists());
    EXPECT_FALSE(info.nameOf(DFMBASE_NAMESPACE::FileInfo::FileNameInfoType::kFileName).isEmpty());
    EXPECT_FALSE(info.nameOf(DFMBASE_NAMESPACE::FileInfo::FileNameInfoType::kFileCopyName).isEmpty());
    EXPECT_FALSE(info.nameOf(DFMBASE_NAMESPACE::FileInfo::FileNameInfoType::kMimeTypeName).isEmpty());
    EXPECT_FALSE(info.displayOf(DFMBASE_NAMESPACE::FileInfo::DisplayInfoType::kFileDisplayName).isEmpty());

    // Size comes from the trashed file itself.
    EXPECT_GT(info.size(), 0);

    // Trash entries cannot be modified in place.
    const QFile::Permissions ps = info.permissions();
    EXPECT_FALSE(ps & QFileDevice::WriteOwner);

    EXPECT_TRUE(info.canAttributes(DFMBASE_NAMESPACE::FileInfo::FileCanType::kCanDelete));
    EXPECT_FALSE(info.canAttributes(DFMBASE_NAMESPACE::FileInfo::FileCanType::kCanHidden));
    EXPECT_TRUE(info.canAttributes(DFMBASE_NAMESPACE::FileInfo::FileCanType::kCanRedirectionFileUrl));
    EXPECT_FALSE(info.canAttributes(DFMBASE_NAMESPACE::FileInfo::FileCanType::kCanDrop));

    EXPECT_TRUE(info.isAttributes(DFMBASE_NAMESPACE::FileInfo::FileIsType::kIsReadable));
    EXPECT_FALSE(info.isAttributes(DFMBASE_NAMESPACE::FileInfo::FileIsType::kIsHidden));

    // Time attributes: at least deletion time must be valid for a real entry.
    const QDateTime deletion = info.timeOf(DFMBASE_NAMESPACE::FileInfo::FileTimeType::kDeletionTime).toDateTime();
    EXPECT_TRUE(deletion.isValid());
    EXPECT_TRUE(info.timeOf(DFMBASE_NAMESPACE::FileInfo::FileTimeType::kLastRead).toDateTime().isValid());
    EXPECT_TRUE(info.timeOf(DFMBASE_NAMESPACE::FileInfo::FileTimeType::kLastModified).toDateTime().isValid());

    // A plain file has no children.
    EXPECT_EQ(info.countChildFile(), -1);

    Q_UNUSED(kUrl)
}

TEST_F(TrashFileInfoRealTest, RealTrashedFile_CustomData)
{
    trashedUrl = createTrashedFile("unit test payload for custom data\n");
    if (!trashedUrl.isValid())
        GTEST_SKIP() << "gio trash not available in this environment";

    TrashFileInfo info(trashedUrl);

    const QVariant orig = info.customData(dfmbase::Global::ItemRoles::kItemFileOriginalPath);
    EXPECT_FALSE(orig.toString().isEmpty());
    EXPECT_FALSE(QFile::exists(orig.toString()));

    const QVariant deletion = info.customData(dfmbase::Global::ItemRoles::kItemFileDeletionDate);
    EXPECT_FALSE(deletion.toString().isEmpty());

    const QVariant refresh = info.customData(dfmbase::Global::ItemRoles::kItemFileRefreshIcon);
    EXPECT_TRUE(refresh.isNull() || refresh.isValid());
}

TEST_F(TrashFileInfoRealTest, RealTrashedFile_SupportedOfAttributes)
{
    trashedUrl = createTrashedFile("unit test payload for drag drop\n");
    if (!trashedUrl.isValid())
        GTEST_SKIP() << "gio trash not available in this environment";

    TrashFileInfo info(trashedUrl);

    // A non-root trash entry cannot receive drops.
    EXPECT_EQ(info.supportedOfAttributes(DFMBASE_NAMESPACE::FileInfo::SupportType::kDrop),
              Qt::IgnoreAction);
    // It can be dragged out for restore.
    EXPECT_EQ(info.supportedOfAttributes(DFMBASE_NAMESPACE::FileInfo::SupportType::kDrag),
              Qt::CopyAction | Qt::MoveAction);
}

TEST_F(TrashFileInfoRealTest, RealTrashedFile_RootUrlAttributes)
{
    TrashFileInfo info(TrashCoreHelper::rootUrl());

    EXPECT_TRUE(info.exists());
    EXPECT_TRUE(info.isAttributes(DFMBASE_NAMESPACE::FileInfo::FileIsType::kIsDir));
    EXPECT_TRUE(info.canAttributes(DFMBASE_NAMESPACE::FileInfo::FileCanType::kCanDrop));
    EXPECT_EQ(info.supportedOfAttributes(DFMBASE_NAMESPACE::FileInfo::SupportType::kDrop),
              Qt::MoveAction);
    // The root reports the aggregated trash size.
    auto data = TrashCoreHelper::calculateTrashRoot();
    EXPECT_EQ(info.size(), data.first);
}

TEST_F(TrashFileInfoRealTest, RealTrashedFile_DeletedEntry)
{
    trashedUrl = createTrashedFile("unit test payload for deleted entry\n");
    if (!trashedUrl.isValid())
        GTEST_SKIP() << "gio trash not available in this environment";

    // A URL that does not exist inside the trash must be handled gracefully.
    TrashFileInfo missing(QUrl("trash:///no_such_entry_udefined_xyz.bin"));
    EXPECT_FALSE(missing.exists());
    EXPECT_EQ(missing.size(), qint64(0));
}
