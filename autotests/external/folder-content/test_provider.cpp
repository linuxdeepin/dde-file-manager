// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderpresentation.h"
#include "networkguard.h"
#include "test_helpers.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QTemporaryDir>
#include <QUrl>

namespace {

void write(const QString &path)
{
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    EXPECT_EQ(file.write("example"), qint64(7));
}

}  // namespace

// The provider refuses to pin anything that is not a readable local
// directory: files, remote URLs and relative ids are all rejected.
TEST(FolderProviderTest, NormalizeIdAcceptsOnlyReadableDirectories)
{
    dfm::FolderProvider provider;
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString url = QUrl::fromLocalFile(dir.path()).toString(QUrl::FullyEncoded);
    EXPECT_EQ(provider.normalizeId(url), url);

    write(dir.filePath("file.txt"));
    EXPECT_TRUE(provider.normalizeId(QUrl::fromLocalFile(dir.filePath("file.txt")).toString()).isEmpty());
    EXPECT_TRUE(provider.normalizeId(QStringLiteral("https://example.org/folder")).isEmpty());
    EXPECT_TRUE(provider.normalizeId(QStringLiteral("file:relative")).isEmpty());
}

// containsPath backs openFile()/startFileDrag(): only entries actually shown
// by the view may be opened or dragged, everything else is ignored.
TEST(FolderProviderTest, ContainsPathRestrictsOperations)
{
    dfm::FolderProvider provider;
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    write(dir.filePath("known.txt"));

    std::unique_ptr<dock::FolderPresentation> presentation(
            provider.create(QUrl::fromLocalFile(dir.path()).toString(), nullptr));
    ASSERT_TRUE(presentation);
    const auto rowReady = [&presentation] {
        return qobject_cast<QAbstractItemModel *>(
                       presentation->property("directoryModel").value<QObject *>())->rowCount() == 1;
    };
    ASSERT_TRUE(waitUntil(rowReady));

    auto contains = [](QObject *presentation, const QString &path) {
        bool known = false;
        return QMetaObject::invokeMethod(presentation, "containsPath",
                                         Q_RETURN_ARG(bool, known), Q_ARG(QString, path)) && known;
    };
    // Entries shown by the view are accepted...
    EXPECT_TRUE(contains(presentation.get(), dir.filePath("known.txt")));
    // ...anything else (even a real file outside the view) is rejected.
    EXPECT_FALSE(contains(presentation.get(), dir.filePath("unknown.txt")));
    QTemporaryDir other;
    ASSERT_TRUE(other.isValid());
    write(other.filePath("elsewhere.txt"));
    EXPECT_FALSE(contains(presentation.get(), other.filePath("elsewhere.txt")));
    EXPECT_FALSE(contains(presentation.get(), QString()));
}

// Reference formatting is a pure static string operation; provider lookups
// live in the shell process and are covered there.
TEST(FolderContentRegistryTest, ReferenceFormat)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString url = QUrl::fromLocalFile(dir.path()).toString(QUrl::FullyEncoded);

    const QString reference = dock::FolderContentRegistry::reference(QStringLiteral("filemanager"), url);
    EXPECT_TRUE(dock::FolderContentRegistry::isReference(reference));
    EXPECT_FALSE(dock::FolderContentRegistry::isReference(QStringLiteral("folder/../bad")));
    EXPECT_FALSE(dock::FolderContentRegistry::isReference(reference + "="));
}
