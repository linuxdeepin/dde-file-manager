// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pluginInterface/previewpluginloader.h"
#include "pluginInterface/filepreviewfactory.h"
#include "pluginInterface/private/pluginloader_p.h"
#include "views/unknowfilepreview.h"
#include "test_fakepreviewplugin.h"

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

using namespace dfmplugin_filepreview;
using namespace covplugin;
DFMBASE_USE_NAMESPACE

class PreviewPluginLoaderTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        // copy the fake plugin .so into an isolated directory and add a
        // garbage .so so update() walks both the success and failure paths
        ASSERT_TRUE(QFile::copy(QString(UT_FAKE_PLUGIN_DIR) + "/cov-fake-preview-plugin.so",
                                dir->filePath("cov-fake-preview-plugin.so")))
                << "fake plugin missing";
        QFile garbage(dir->filePath("not-a-plugin.so"));
        ASSERT_TRUE(garbage.open(QIODevice::WriteOnly));
        garbage.write("definitely not an elf shared object");
        garbage.close();

        PreviewPluginLoaderPrivate::pluginPaths.append(dir->path());
    }

    void TearDown() override
    {
        PreviewPluginLoaderPrivate::pluginPaths.removeAll(dir->path());
        qApp->processEvents();
        dir.reset();
    }

    std::unique_ptr<QTemporaryDir> dir;
};

TEST_F(PreviewPluginLoaderTest, Update_LoadsValidPluginAndSkipsGarbage)
{
    // Arrange / Act: the constructor runs update() over the temp directory
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid, "/previews",
                               Qt::CaseInsensitive);

    // Assert: only the real plugin is registered
    EXPECT_EQ(loader.dptr->pluginLoaderList.size(), 1);
    EXPECT_FALSE(loader.dptr->keyMap.values("text/plain").isEmpty());
    EXPECT_FALSE(loader.dptr->keyMap.values("x-cov/fake").isEmpty());
}

TEST_F(PreviewPluginLoaderTest, MetaData_ContainsIidAndKeys)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act
    const QList<QJsonObject> metaData = loader.metaData();

    // Assert
    ASSERT_EQ(metaData.size(), 1);
    EXPECT_EQ(metaData.at(0).value("IID").toString(),
              QString("com.deepin.filemanager.FilePreviewFactoryInterface_iid"));
    const QJsonArray keys = metaData.at(0).value("MetaData").toObject().value("Keys").toArray();
    EXPECT_EQ(keys.size(), 2);
}

TEST_F(PreviewPluginLoaderTest, Instance_ValidIndexAndInvalidIndex)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act
    QObject *factory = loader.instance(0);
    QObject *negative = loader.instance(-1);
    QObject *outOfRange = loader.instance(99);

    // Assert
    ASSERT_NE(factory, nullptr);
    EXPECT_NE(qobject_cast<AbstractFilePreviewPlugin *>(factory), nullptr);
    EXPECT_EQ(negative, nullptr);
    EXPECT_EQ(outOfRange, nullptr);
}

TEST_F(PreviewPluginLoaderTest, PluginLoader_ByKey_ReturnsLoaderOrNull)
{
    // Arrange: the default constructor is case-sensitive, the explicit one
    // honours the requested case-insensitivity
    PreviewPluginLoader csLoader(FilePreviewFactoryInterface_iid);
    PreviewPluginLoader ciLoader(FilePreviewFactoryInterface_iid, "/previews",
                                 Qt::CaseInsensitive);

    // Act
    QPluginLoader *hit = ciLoader.pluginLoader("TEXT/PLAIN");   // case-insensitive
    QPluginLoader *miss = ciLoader.pluginLoader("no/such-key");

    // Assert
    ASSERT_NE(hit, nullptr);
    EXPECT_EQ(miss, nullptr);
    EXPECT_EQ(ciLoader.pluginLoaderList("text/plain").size(), 1);
    EXPECT_TRUE(ciLoader.pluginLoaderList("no/such-key").isEmpty());
    // case-sensitive default: uppercase query misses, exact query hits
    EXPECT_EQ(csLoader.pluginLoader("TEXT/PLAIN"), nullptr);
    EXPECT_NE(csLoader.pluginLoader("text/plain"), nullptr);
}

TEST_F(PreviewPluginLoaderTest, KeyMap_IndexesKeysByLoaderPosition)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act
    const QMultiMap<int, QString> map = loader.keyMap();

    // Assert
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.values(0).contains(QString("text/plain")));
    EXPECT_TRUE(map.values(0).contains(QString("x-cov/fake")));
}

TEST_F(PreviewPluginLoaderTest, IndexOf_CaseInsensitiveAndMissing)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act / Assert
    EXPECT_EQ(loader.indexOf("TEXT/PLAIN"), 0);
    EXPECT_EQ(loader.indexOf("text/plain"), 0);
    EXPECT_EQ(loader.indexOf("nothing/here"), -1);
}

TEST_F(PreviewPluginLoaderTest, GetAllIndexByKey_CollectsAllIndexes)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act
    const QList<int> hits = loader.getAllIndexByKey("x-cov/FAKE");
    const QList<int> misses = loader.getAllIndexByKey("absent");

    // Assert
    EXPECT_EQ(hits, QList<int> { 0 });
    EXPECT_TRUE(misses.isEmpty());
}

TEST_F(PreviewPluginLoaderTest, DLoadPlugin_CreatesPreviewViaFactory)
{
    // Arrange
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Act
    AbstractBasePreview *view = dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>(
                &loader, "text/plain");
    AbstractBasePreview *none = dLoadPlugin<AbstractBasePreview, AbstractFilePreviewPlugin>(
                &loader, "no/such-key");
    QList<AbstractBasePreview *> views = dLoadPluginList<AbstractBasePreview, AbstractFilePreviewPlugin>(
                &loader, "text/plain");

    // Assert
    ASSERT_NE(view, nullptr);
    EXPECT_EQ(none, nullptr);
    ASSERT_EQ(views.size(), 1);
    // all previews come from the plugin factory (identified by class name:
    // the plugin DSO owns its own metaobject, qobject_cast cannot cross it)
    EXPECT_EQ(QString(view->metaObject()->className()), QString("covplugin::CovPreview"));
    EXPECT_EQ(QString(views.first()->metaObject()->className()), QString("covplugin::CovPreview"));

    delete view;
    qDeleteAll(views);
}

TEST_F(PreviewPluginLoaderTest, RefreshAll_ReScansAllRegisteredLoaders)
{
    // Arrange: one loader is already registered by the fixture use above
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);
    const int before = loader.dptr->pluginLoaderList.size();

    // Act: refresh must not duplicate already loaded plugins
    PreviewPluginLoader::refreshAll();
    const int after = loader.dptr->pluginLoaderList.size();

    // Assert
    EXPECT_EQ(before, after);
    EXPECT_EQ(loader.indexOf("text/plain"), 0);
}

TEST_F(PreviewPluginLoaderTest, PrivateDefaultPaths_UsedWhenPluginPathsEmpty)
{
    // Arrange: empty the shared path list so the private ctor falls back to
    // the compiled-in DFM_PLUGIN_PREVIEW_DIR (which does not exist here)
    const QStringList saved = PreviewPluginLoaderPrivate::pluginPaths;
    PreviewPluginLoaderPrivate::pluginPaths.clear();

    // Act
    PreviewPluginLoader loader(FilePreviewFactoryInterface_iid);

    // Assert: default path recorded, no plugins found there
    EXPECT_EQ(PreviewPluginLoaderPrivate::pluginPaths.size(), 1);
    EXPECT_EQ(loader.dptr->pluginLoaderList.size(), 0);

    // restore the fake plugin path for later loaders
    PreviewPluginLoaderPrivate::pluginPaths = saved;
}

TEST_F(PreviewPluginLoaderTest, KeyLiterals_ExposeMetadataKeyNames)
{
    // Arrange / Act / Assert: the private header exposes metadata key names
    EXPECT_EQ(iidKeyLiteral(), QString("IID"));
    EXPECT_EQ(versionKeyLiteral(), QString("version"));
    EXPECT_EQ(metaDataKeyLiteral(), QString("MetaData"));
    EXPECT_EQ(keysKeyLiteral(), QString("Keys"));
}

// ---------------------------------------------------------------------------
// FilePreviewFactory (uses its own global loader over the fake plugin)
// ---------------------------------------------------------------------------
class FilePreviewFactoryTest : public testing::Test
{
protected:
    void TearDown() override { qApp->processEvents(); }
};

TEST_F(FilePreviewFactoryTest, Keys_ContainFakePluginKeys)
{
    // Arrange / Act
    const QStringList keys = FilePreviewFactory::keys();

    // Assert
    EXPECT_TRUE(keys.contains(QString("text/plain")));
    EXPECT_TRUE(keys.contains(QString("x-cov/fake")));
}

TEST_F(FilePreviewFactoryTest, Create_RegisteredKey_ReturnsPluginPreview)
{
    // Arrange / Act
    AbstractBasePreview *view = FilePreviewFactory::create("text/plain");

    // Assert
    ASSERT_NE(view, nullptr);
    EXPECT_EQ(QString(view->metaObject()->className()), QString("covplugin::CovPreview"));
    EXPECT_TRUE(FilePreviewFactory::isSuitedWithKey(view, "TEXT/PLAIN"));

    // Act: destroy the view -> the factory forgets it
    delete view;
    qApp->processEvents();

    // Assert
    EXPECT_FALSE(FilePreviewFactory::isSuitedWithKey(view, "text/plain"));
}

TEST_F(FilePreviewFactoryTest, Create_UnknownKey_ReturnsNull)
{
    // Arrange / Act
    AbstractBasePreview *view = FilePreviewFactory::create("no/such-mimetype");

    // Assert
    EXPECT_EQ(view, nullptr);
    EXPECT_FALSE(FilePreviewFactory::hasPluginForKey("no/such-mimetype"));
}

TEST_F(FilePreviewFactoryTest, IsSuitedWithKey_NullOrForeignPreview_False)
{
    // Arrange
    UnknowFilePreview foreign;

    // Act / Assert
    EXPECT_FALSE(FilePreviewFactory::isSuitedWithKey(nullptr, "text/plain"));
    EXPECT_FALSE(FilePreviewFactory::isSuitedWithKey(&foreign, "text/plain"));
    EXPECT_TRUE(FilePreviewFactory::hasPluginForKey("text/plain"));
}
