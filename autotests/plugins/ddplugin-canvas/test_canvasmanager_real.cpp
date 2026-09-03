// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#define private public
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"
#include "plugins/desktop/ddplugin-canvas/private/canvasmanager_p.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel_p.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"
#include "plugins/desktop/ddplugin-canvas/grid/canvasgrid.h"
#undef private

#include <dfm-base/base/application/application.h>
#include "canvas_test_common.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QTemporaryDir>
#include <QFile>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace ddplugin_canvas;

namespace {

QUrl makeUrl(QTemporaryDir *tempDir, const QString &name)
{
    const QString path = tempDir->filePath(name);
    QFile f(path);
    if (!QFile::exists(path)) {
        f.open(QIODevice::WriteOnly);
        f.close();
    }
    return QUrl::fromLocalFile(path);
}

} // namespace

class CanvasManagerReal : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(true); });

        stub.set_lamda(
            static_cast<FileInfoPointer (*)(const QUrl &, dfmbase::Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
            [](const QUrl &url, dfmbase::Global::CreateFileInfoType, QString *err) -> FileInfoPointer {
                if (err)
                    *err = QString();
                FileInfoPointer info(new SyncFileInfo(url));
                if (info)
                    info->refresh();
                return info;
            });

        // There must only be one CanvasManager instance; clean up any pre-existing one.
        if (CanvasManager::instance()) {
            delete CanvasManager::instance();
            CanvasManagerPrivate::global = nullptr;
        }

        manager = new CanvasManager();

        // init() creates the hook; publish paths such as setAutoArrange()
        // dereference it, so create it the same way init() does.
        manager->d->hookIfs = new CanvasManagerHook(manager);

        // Prepare the shared grid to avoid crashes in auto-arrange paths.
        grid = CanvasGrid::instance();
        grid->requestSync(0);
        grid->initSurface(1);
        grid->setMode(CanvasGrid::Mode::Custom);
        grid->updateSize(1, QSize(4, 4));
        grid->setItems(QStringList());

        // Wire up real models without using the broker init path.
        manager->d->sourceModel = new FileInfoModel(manager);
        manager->d->sourceModel->setRootUrl(QUrl::fromLocalFile(tempDir->path()));

        manager->d->canvasModel = new CanvasProxyModel(manager);
        manager->d->canvasModel->setSourceModel(manager->d->sourceModel);

        QList<QUrl> urls;
        urls << makeUrl(tempDir.data(), "a.txt")
             << makeUrl(tempDir.data(), "b.txt");
        manager->d->sourceModel->d->resetData(urls);

        manager->d->selectionModel = new CanvasSelectionModel(manager->d->canvasModel, manager);
    }

    void TearDown() override
    {
        if (manager) {
            delete manager;
            manager = nullptr;
        }
        CanvasManagerPrivate::global = nullptr;
        if (grid) {
            grid->initSurface(1);
            grid->setItems(QStringList());
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    CanvasManager *manager = nullptr;
    CanvasGrid *grid = nullptr;
};

TEST_F(CanvasManagerReal, accessors)
{
    EXPECT_EQ(manager->instance(), manager);

    EXPECT_NE(manager->fileModel(), nullptr);
    EXPECT_NE(manager->model(), nullptr);
    EXPECT_NE(manager->selectionModel(), nullptr);
    EXPECT_TRUE(manager->views().isEmpty());

    int level = manager->iconLevel();
    manager->setIconLevel(level);
    EXPECT_EQ(manager->iconLevel(), level);

    bool arrange = manager->autoArrange();
    manager->setAutoArrange(!arrange);
    EXPECT_EQ(manager->autoArrange(), !arrange);
    manager->setAutoArrange(arrange);
}

TEST_F(CanvasManagerReal, updateAndRefresh)
{
    EXPECT_NO_THROW(manager->update());
    EXPECT_NO_THROW(manager->refresh(true));
}

TEST_F(CanvasManagerReal, handlers)
{
    EXPECT_NO_THROW(manager->onChangeIconLevel(true));
    EXPECT_NO_THROW(manager->onTrashStateChanged());
    EXPECT_NO_THROW(manager->onFontChanged());
}
