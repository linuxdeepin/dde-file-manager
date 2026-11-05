// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderpresentation.h"
#include "presentation_test_fixture.h"
#include "test_helpers.h"

#include <gtest/gtest.h>
#include <QAbstractItemModel>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>

namespace {

void write(const QString &path)
{
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    EXPECT_EQ(file.write("example"), qint64(7));
}

QAbstractItemModel *model(dock::FolderPresentation *view)
{
    return qobject_cast<QAbstractItemModel *>(view->property("directoryModel").value<QObject *>());
}

QQuickItem *visualChild(QQuickItem *root, const QString &name)
{
    if (!root)
        return nullptr;
    if (root->objectName() == name)
        return root;
    for (auto *child : root->childItems()) {
        if (auto *match = visualChild(child, name))
            return match;
    }
    return nullptr;
}

}  // namespace

// Two presentations watch and navigate their directories independently.
TEST_F(PresentationTest, WatchAndNavigateIndependently)
{
    QTemporaryDir first, second;
    ASSERT_TRUE(first.isValid());
    ASSERT_TRUE(second.isValid());
    ASSERT_TRUE(QDir(first.path()).mkdir("child"));
    write(second.filePath("other.txt"));

    std::unique_ptr<dock::FolderPresentation> a(create(first.path()));
    std::unique_ptr<dock::FolderPresentation> b(create(second.path()));
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    ASSERT_TRUE(waitUntil([&] { return model(a.get())->rowCount() == 1; }));
    ASSERT_TRUE(waitUntil([&] { return model(b.get())->rowCount() == 1; }));
    ASSERT_TRUE(waitUntil([&] { return a->previewIcons().size() == 1; }));

    const auto title = a->title();
    ASSERT_TRUE(QMetaObject::invokeMethod(a.get(), "navigateTo", Q_ARG(QString, first.filePath("child"))));
    ASSERT_TRUE(waitUntil([&] { return model(a.get())->rowCount() == 0; }));
    EXPECT_EQ(a->title(), title);
    EXPECT_EQ(a->previewIcons().size(), 1);

    write(first.filePath("created.txt"));
    ASSERT_TRUE(waitUntil([&] { return a->previewIcons().size() == 2; }));
    EXPECT_EQ(model(a.get())->rowCount(), 0);
    EXPECT_EQ(model(b.get())->rowCount(), 1);

    ASSERT_TRUE(QFile::rename(first.filePath("created.txt"), first.filePath("renamed.txt")));
    ASSERT_TRUE(QMetaObject::invokeMethod(a.get(), "goBack"));
    ASSERT_TRUE(waitUntil([&] { return model(a.get())->rowCount() == 2; }));
    EXPECT_EQ(model(a.get())->data(model(a.get())->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("renamed.txt"));

    ASSERT_TRUE(QFile::remove(first.filePath("renamed.txt")));
    ASSERT_TRUE(waitUntil([&] { return model(a.get())->rowCount() == 1; }));
    EXPECT_EQ(a->previewIcons().size(), 1);
}

// A pinned directory that disappears (with its parent intact) recovers once
// it comes back, including fresh directory contents.
TEST_F(PresentationTest, MissingDirectoryRecovers)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("parent/pinned");
    ASSERT_TRUE(QDir().mkpath(path));
    write(path + "/before.txt");

    std::unique_ptr<dock::FolderPresentation> presentation(create(path));
    ASSERT_TRUE(presentation);
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));

    ASSERT_TRUE(QDir(dir.filePath("parent")).removeRecursively());
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 0; }));
    ASSERT_TRUE(waitUntil([&] { return presentation->previewIcons().isEmpty(); }));

    ASSERT_TRUE(QDir().mkpath(path));
    write(path + "/after.txt");
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    ASSERT_TRUE(waitUntil([&] { return presentation->previewIcons().size() == 1; }));
    EXPECT_EQ(model(presentation.get())->data(model(presentation.get())->index(0, 0), Qt::DisplayRole).toString(),
              QStringLiteral("after.txt"));
}

// A renamed pinned folder keeps its inode, so the pin has to follow the new
// name instead of degrading into an empty folder on a dead path.
TEST_F(PresentationTest, RenamedDirectoryIsFollowed)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("original");
    ASSERT_TRUE(QDir().mkpath(path));
    write(path + "/inside.txt");

    std::unique_ptr<dock::FolderPresentation> presentation(create(path));
    ASSERT_TRUE(presentation);
    QSignalSpy identitySpy(presentation.get(), &dock::FolderPresentation::identityChanged);
    ASSERT_TRUE(identitySpy.isValid());
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    EXPECT_EQ(presentation->title(), QStringLiteral("original"));

    const QString renamed = dir.filePath("moved");
    ASSERT_TRUE(QFile::rename(path, renamed));

    ASSERT_TRUE(waitUntil([&] { return identitySpy.size() == 1; }));
    const QString reported = identitySpy.first().at(0).toString();
    EXPECT_EQ(reported, QUrl::fromLocalFile(renamed).toString(QUrl::FullyEncoded));
    // The new id has to be a usable provider id, otherwise the dock cannot
    // persist it and the pin stays broken.
    EXPECT_EQ(provider()->normalizeId(reported), reported);
    EXPECT_EQ(presentation->title(), QStringLiteral("moved"));
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    EXPECT_EQ(model(presentation.get())->property("path").toString(), renamed);
    ASSERT_TRUE(waitUntil([&] { return presentation->previewIcons().size() == 1; }));
}

// A deleted pinned folder can never have contents again, so the provider
// reports the loss and the dock drops the pin. Removing the whole tree
// instead keeps the pin, because the path may come back.
TEST_F(PresentationTest, DeletedDirectoryReportsLoss)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("parent/pinned");
    ASSERT_TRUE(QDir().mkpath(path));
    write(path + "/inside.txt");

    std::unique_ptr<dock::FolderPresentation> presentation(create(path));
    ASSERT_TRUE(presentation);
    QSignalSpy lostSpy(presentation.get(), &dock::FolderPresentation::contentLost);
    ASSERT_TRUE(lostSpy.isValid());
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    EXPECT_EQ(presentation->title(), QStringLiteral("pinned"));

    // The parent survives, so the entry itself was deleted.
    ASSERT_TRUE(QDir(path).removeRecursively());

    ASSERT_TRUE(waitUntil([&] { return lostSpy.size() == 1; }));
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 0; }));
    ASSERT_TRUE(waitUntil([&] { return presentation->previewIcons().isEmpty(); }));
}

// Deleting an ancestor is treated as the path becoming unreachable instead
// of the pinned folder being deleted, so the pin has to survive.
TEST_F(PresentationTest, RemovedAncestorDoesNotReportLoss)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("parent/pinned");
    ASSERT_TRUE(QDir().mkpath(path));
    write(path + "/inside.txt");

    std::unique_ptr<dock::FolderPresentation> presentation(create(path));
    ASSERT_TRUE(presentation);
    QSignalSpy lostSpy(presentation.get(), &dock::FolderPresentation::contentLost);
    ASSERT_TRUE(lostSpy.isValid());
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));

    ASSERT_TRUE(QDir(dir.filePath("parent")).removeRecursively());
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 0; }));
    QTest::qWait(1200);   // outlive the loss grace period
    EXPECT_EQ(lostSpy.size(), 0);
}

// A desktop entry inside the pin starts the application it points to
// (AM1 first, GIO fallback), exactly like double-clicking it in the file
// manager: the entry itself must not be opened as a plain file.
TEST_F(PresentationTest, DesktopEntryLaunchesApplication)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(QDir().mkpath(dir.filePath("pinned")));
    const QString marker = dir.filePath("launched.marker");
    const QString desktopPath = dir.filePath("pinned/folder-content-launch-test.desktop");
    {
        QFile desktop(desktopPath);
        ASSERT_TRUE(desktop.open(QIODevice::WriteOnly | QIODevice::Truncate));
        const QByteArray body = "[Desktop Entry]\n"
                                "Type=Application\n"
                                "Name=FolderContentLaunchTest\n"
                                "Icon=text-x-generic\n"
                                "Exec=/bin/touch " + QFile::encodeName(marker) + "\n";
        ASSERT_EQ(desktop.write(body), qint64(body.size()));
    }

    std::unique_ptr<dock::FolderPresentation> presentation(create(dir.filePath("pinned")));
    ASSERT_TRUE(presentation);
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    // The localized entry name replaces the file name in the view.
    EXPECT_EQ(model(presentation.get())->data(model(presentation.get())->index(0, 0), Qt::DisplayRole).toString(),
              QStringLiteral("FolderContentLaunchTest"));

    ASSERT_TRUE(QMetaObject::invokeMethod(presentation.get(), "openFile", Q_ARG(QString, desktopPath)));
    // The Exec line ran, i.e. the entry launched instead of being opened.
    ASSERT_TRUE(waitUntil([&] { return QFile::exists(marker); }))
            << "desktop entry did not launch its application";
}

// The popup content is an embedded QML item wired to the presentation.
TEST_F(PresentationTest, EmbeddedQmlContent)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    write(dir.filePath("hello.txt"));

    std::unique_ptr<dock::FolderPresentation> presentation(create(dir.path()));
    ASSERT_TRUE(presentation);
    QQmlEngine engine;
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    ASSERT_TRUE(warnings.isValid());
    QQmlComponent component(&engine, presentation->contentUrl());
    std::unique_ptr<QObject> content(component.createWithInitialProperties(
            { { "presentation", QVariant::fromValue(presentation.get()) } }));
    ASSERT_TRUE(content) << qPrintable(component.errorString());
    auto item = qobject_cast<QQuickItem *>(content.get());
    ASSERT_TRUE(item);

    QQuickWindow window;
    window.resize(480, 400);
    item->setParentItem(window.contentItem());
    item->setSize(QSizeF(480, 400));
    window.show();
    ASSERT_TRUE(waitUntil([&] { return model(presentation.get())->rowCount() == 1; }));
    QTest::qWait(100);
    for (const auto &arguments : warnings)
        for (const auto &error : arguments.at(0).value<QList<QQmlError>>())
            qWarning() << error;
    EXPECT_EQ(qmlErrorCount(warnings), 0);
}

// The host neither reads nor composes previews. The plugin's own icon reacts
// to live filesystem changes while keeping four visual slots.
TEST_F(PresentationTest, IconIsProvidedByPlugin)
{
    QTemporaryDir dir, empty;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(empty.isValid());

    std::unique_ptr<dock::FolderPresentation> presentation(create(dir.path()));
    std::unique_ptr<dock::FolderPresentation> other(create(empty.path()));
    ASSERT_TRUE(presentation);
    ASSERT_TRUE(other);
    EXPECT_EQ(presentation->iconUrl(), QUrl(QStringLiteral("qrc:/dfm-folder-content/FolderIcon.qml")));

    QQmlEngine engine;
    QSignalSpy warnings(&engine, &QQmlEngine::warnings);
    ASSERT_TRUE(warnings.isValid());
    QQmlComponent component(&engine, presentation->iconUrl());
    std::unique_ptr<QObject> iconObject(component.createWithInitialProperties(
            { { "presentation", QVariant::fromValue(static_cast<QObject *>(presentation.get())) } }));
    ASSERT_TRUE(iconObject) << qPrintable(component.errorString());
    auto item = qobject_cast<QQuickItem *>(iconObject.get());
    ASSERT_TRUE(item);

    QQuickWindow window;
    item->setParentItem(window.contentItem());
    item->setSize(QSizeF(64, 64));
    window.show();
    EXPECT_EQ(item->property("presentation").value<QObject *>(), presentation.get());
    auto placeholder = item->findChild<QQuickItem *>("folderPlaceholder");
    ASSERT_TRUE(placeholder);
    ASSERT_TRUE(waitUntil([&] { return placeholder->isVisible(); }));

    for (int i = 0; i < 6; ++i)
        write(dir.filePath(QString::number(i) + ".txt"));
    ASSERT_TRUE(waitUntil([&] { return presentation->previewIcons().size() == 4; }));
    ASSERT_TRUE(waitUntil([&] { return !placeholder->isVisible(); }));
    for (int i = 0; i < 4; ++i) {
        auto preview = visualChild(item, "folderPreview" + QString::number(i));
        ASSERT_TRUE(preview) << "missing folderPreview" << i;
        ASSERT_TRUE(waitUntil([&preview] { return preview->isVisible(); }));
    }
    item->setSize(QSizeF(32, 32));
    ASSERT_TRUE(waitUntil([&] { return item->property("width").toInt() == 32; }));
    EXPECT_EQ(other->previewIcons().size(), 0);
    for (const auto &arguments : warnings)
        for (const auto &error : arguments.at(0).value<QList<QQmlError>>())
            qWarning() << error;
    EXPECT_EQ(qmlErrorCount(warnings), 0);
}
