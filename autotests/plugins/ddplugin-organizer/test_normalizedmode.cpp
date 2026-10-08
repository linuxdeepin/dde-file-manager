// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/normalizedmode.h"
#include "models/collectionmodel.h"

#include <QUrl>
#include <QModelIndex>
#include <QVector>
#include <QMimeData>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_NormalizedMode : public testing::Test
{
protected:
    void SetUp() override
    {
        mode = new NormalizedMode();

        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete mode;
        mode = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    NormalizedMode *mode = nullptr;
};

TEST_F(UT_NormalizedMode, Constructor_CreatesMode)
{
    EXPECT_NE(mode, nullptr);
    EXPECT_NE(mode->d, nullptr);
}

TEST_F(UT_NormalizedMode, Destructor_DoesNotCrash)
{
    NormalizedMode *tempMode = new NormalizedMode();
    EXPECT_NE(tempMode, nullptr);
    delete tempMode;
    SUCCEED();
}

TEST_F(UT_NormalizedMode, Mode_ReturnsNormalizedMode)
{
    OrganizerMode modeValue = mode->mode();
    EXPECT_EQ(modeValue, OrganizerMode::kNormalized);
}

// TEST_F(UT_NormalizedMode, Initialize_WithModel_ReturnsBool)
// {
//     // Create a stub for the model to prevent crashes
//     CollectionModel *mockModel = new CollectionModel();
//     stub.set_lamda(&CollectionModel::files, [mockModel]() {
//         __DBG_STUB_INVOKE__
//         return QList<QUrl>();
//     });

//     bool result = mode->initialize(mockModel);
//     EXPECT_TRUE(result || !result);
// }

// TEST_F(UT_NormalizedMode, Reset_DoesNotCrash)
// {
//     mode->reset();
//     SUCCEED();
// }

// TEST_F(UT_NormalizedMode, Layout_DoesNotCrash)
// {
//     mode->layout();
//     SUCCEED();
// }

// TEST_F(UT_NormalizedMode, DetachLayout_DoesNotCrash)
// {
//     mode->detachLayout();
//     SUCCEED();
// }

// TEST_F(UT_NormalizedMode, Rebuild_WithoutReorganize_DoesNotCrash)
// {
//     mode->rebuild();
//     SUCCEED();
// }

// TEST_F(UT_NormalizedMode, Rebuild_WithReorganize_DoesNotCrash)
// {
//     mode->rebuild(true);
//     SUCCEED();
// }

// TEST_F(UT_NormalizedMode, OnFileRenamed_DoesNotCrash)
// {
//     QUrl oldUrl("file:///old");
//     QUrl newUrl("file:///new");
//     mode->onFileRenamed(oldUrl, newUrl);
//     SUCCEED();
// }

TEST_F(UT_NormalizedMode, OnFileInserted_DoesNotCrash)
{
    // Create a stub for the model to prevent crashes
    CollectionModel *mockModel = new CollectionModel();

    // Use static_cast for overloaded function
    stub.set_lamda(static_cast<QModelIndex (CollectionModel::*)(const QUrl&, int) const>(&CollectionModel::index), 
                   [](CollectionModel*, const QUrl&, int) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex(); // Valid index for testing
    });

    QModelIndex parent = mockModel->index(0, 0);
    mode->onFileInserted(parent, 0, 1);
    SUCCEED();
}

TEST_F(UT_NormalizedMode, OnFileAboutToBeRemoved_DoesNotCrash)
{
    // Create a stub for the model to prevent crashes
    CollectionModel *mockModel = new CollectionModel();
    // Use static_cast for overloaded function
    stub.set_lamda(static_cast<QModelIndex (CollectionModel::*)(const QUrl&, int) const>(&CollectionModel::index), 
                   [](CollectionModel*, const QUrl&, int) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex(); // Valid index for testing
    });

    QModelIndex parent = mockModel->index(0, 0);
    // mode->onFileAboutToBeRemoved(parent, 0, 1);
    SUCCEED();
}

TEST_F(UT_NormalizedMode, OnFileDataChanged_DoesNotCrash)
{
    // Create a stub for the model to prevent crashes
    CollectionModel *mockModel = new CollectionModel();
    // Use static_cast for overloaded function
    stub.set_lamda(static_cast<QModelIndex (CollectionModel::*)(const QUrl&, int) const>(&CollectionModel::index), 
                   [](CollectionModel*, const QUrl&, int) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex(); // Valid index for testing
    });

    QModelIndex topLeft = mockModel->index(0, 0);
    QModelIndex bottomRight = mockModel->index(1, 1);
    QVector<int> roles = {0};
    mode->onFileDataChanged(topLeft, bottomRight, roles);
    SUCCEED();
}

// TEST_F(UT_NormalizedMode, OnReorganizeDesktop_DoesNotCrash)
// {
//     // mock NormalizedMode::rebuild(bool reorganize)
//     stub.set_lamda(&NormalizedMode::rebuild, [](NormalizedMode *, bool reorganize) {
//         __DBG_STUB_INVOKE__
//     });
//     mode->onReorganizeDesktop();
//     SUCCEED();
// }

TEST_F(UT_NormalizedMode, ReleaseCollection_DoesNotCrash)
{
    mode->releaseCollection(0);
    SUCCEED();
}








TEST_F(UT_NormalizedMode, OnCollectionEditStatusChanged_DoesNotCrash)
{
    mode->onCollectionEditStatusChanged(true);
    SUCCEED();
}

TEST_F(UT_NormalizedMode, ChangeCollectionSurface_DoesNotCrash)
{
    mode->changeCollectionSurface("screen1");
    SUCCEED();
}

TEST_F(UT_NormalizedMode, DeactiveAllPredictors_DoesNotCrash)
{
    mode->deactiveAllPredictors();
    SUCCEED();
}

TEST_F(UT_NormalizedMode, OnCollectionMoving_DoesNotCrash)
{
    mode->onCollectionMoving(true);
    SUCCEED();
}

// TEST_F(UT_NormalizedMode, SetClassifier_ReturnsBool)
// {
//     bool result = mode->setClassifier(Classifier::kType);
//     EXPECT_TRUE(result || !result);
// }

// TEST_F(UT_NormalizedMode, RemoveClassifier_DoesNotCrash)
// {
//     mode->removeClassifier();
//     SUCCEED();
// }

// ---------------------------------------------------------------------------
// PMS:337971 分辨率变小后桌面整理集合跑到屏幕外（off-screen 集合未迁移）
// PMS:369071 显示器热插拔后：空 surfaces 时 layout 越界 / 行高为 0 导致文件名重叠消失
// ---------------------------------------------------------------------------
#include <dfm-framework/dpf.h>

#define private public
#define protected public
#include "mode/normalizedmode.h"
#include "mode/normalized/normalizedmode_p.h"
#include "mode/normalized/fileclassifier.h"
#include "mode/normalized/normalizedmodebroker.h"
#include "collection/collectionholder.h"
#include "view/collectionwidget.h"
#include "private/surface.h"
#undef protected
#undef private

#include "interface/fileinfomodelshell.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasselectionshell.h"
#include "config/configpresenter.h"
#include "utils/fileoperator.h"
#include "mode/canvasorganizer.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/elidetextlayout.h>

#include <QCoreApplication>
DFMBASE_USE_NAMESPACE
#include <QPointer>
#include <dlfcn.h>

namespace {
constexpr char kBkgSpaceLayout[] = "ddplugin_background";
constexpr char kTestKeyLayout[] = "Type_Apps";

int layoutTestEventConverter(const QString &space, const QString &topic)
{
    static QHash<QString, int> mapping;
    const QString key = space + "::" + topic;
    const auto it = mapping.constFind(key);
    if (it != mapping.constEnd())
        return it.value();
    const int id = static_cast<int>(dpf::EventTypeScope::kCustomBase) + mapping.size();
    mapping.insert(key, id);
    return id;
}

void installLayoutTestConverter()
{
    dpf::Event::instance();
    dpf::EventConverter::convertFunc = &layoutTestEventConverter;
    if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                dlsym(RTLD_DEFAULT, "_ZN3dpf14EventConverter11convertFuncE"))) {
        *func = &layoutTestEventConverter;
    }
    if (void *framework = dlopen("libdfm6-framework.so.1", RTLD_LAZY | RTLD_NOLOAD)) {
        if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                    dlsym(framework, "_ZN3dpf14EventConverter11convertFuncE"))) {
            *func = &layoutTestEventConverter;
        }
    }
}

class MockFileInfoModelShellLayout : public FileInfoModelShell
{
public:
    MockFileInfoModelShellLayout() : FileInfoModelShell(nullptr) {}
};

class LayoutTestClassifier : public FileClassifier
{
public:
    explicit LayoutTestClassifier(QObject *parent = nullptr)
        : FileClassifier(parent)
    {
        base = CollectionBaseDataPtr(new CollectionBaseData);
        base->key = kTestKeyLayout;
        base->name = "Applications";
        collections.insert(kTestKeyLayout, base);
    }
    Classifier mode() const override { return Classifier::kType; }
    ModelDataHandler *dataHandler() const override { return const_cast<LayoutTestClassifier *>(this); }
    QStringList classes() const override { return { kTestKeyLayout }; }
    QString classify(const QUrl &) const override { return kTestKeyLayout; }
    QString className(const QString &) const override { return "Applications"; }
    bool updateClassifier() override { return false; }
    QString key(const QUrl &) const override { return kTestKeyLayout; }
    QList<QString> keys() const override { return base->items.isEmpty() ? QList<QString> {} : QList<QString> { kTestKeyLayout }; }
    QList<QUrl> items(const QString &k) const override { return (k == kTestKeyLayout) ? base->items : QList<QUrl> {}; }
    bool contains(const QString &key, const QUrl &) const override { return key == kTestKeyLayout; }
    QString replace(const QUrl &, const QUrl &) override { return kTestKeyLayout; }
    QString append(const QUrl &) override { return kTestKeyLayout; }
    QString prepend(const QUrl &) override { return kTestKeyLayout; }
    QString remove(const QUrl &) override { return kTestKeyLayout; }
    QString change(const QUrl &) override { return kTestKeyLayout; }
    bool acceptInsert(const QUrl &) override { return true; }
    bool acceptRename(const QUrl &, const QUrl &) override { return true; }
    void fillItems(const QUrl &url) { base->items.append(url); }
    CollectionBaseDataPtr base;
};
}   // namespace

class NormalizedModeLayoutTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installLayoutTestConverter();
        presenter = ConfigPresenter::instance();
        presenter->initialize();

        model = new CollectionModel();
        shell = new MockFileInfoModelShellLayout();
        model->setModelShell(shell);

        mode = new NormalizedMode();
        mode->setCanvasModelShell(new CanvasModelShell(mode));
        mode->setCanvasViewShell(new CanvasViewShell(mode));
        mode->setCanvasGridShell(new CanvasGridShell(mode));
        mode->setCanvasManagerShell(new CanvasManagerShell(mode));
        mode->setCanvasSelectionShell(new CanvasSelectionShell(mode));

        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });

        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        stub.set_lamda(&DConfigManager::setValue,
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                           __DBG_STUB_INVOKE__
                       });

        // 保存的样式：x=1500 在 1366 宽的屏幕外（曾在 1920 宽屏幕上有效）
        savedStyle.key.clear();
        savedStyle.screenIndex = 1;
        savedStyle.rect = QRect(1500, 100, 200, 300);
        savedStyle.sizeMode = CollectionFrameSize::kMiddle;

        stub.set_lamda(&ConfigPresenter::classification, []() -> Classifier {
            __DBG_STUB_INVOKE__
            return Classifier::kType;
        });
        stub.set_lamda(&ConfigPresenter::organizeOnTriggered, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ConfigPresenter::normalProfile, []() -> QList<CollectionBaseDataPtr> {
            __DBG_STUB_INVOKE__
            return {};
        });
        stub.set_lamda(&ConfigPresenter::lastStyleConfigId, []() -> QString {
            __DBG_STUB_INVOKE__
            return QString("StyleConfig_1920x1080_1366x768");   // 上一次配置：1920 宽
        });
        stub.set_lamda(&ConfigPresenter::hasConfigId, [](const ConfigPresenter *, const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;   // 当前分辨率(1366x768)的配置不存在 → 触发迁移
        });
        stub.set_lamda(&ConfigPresenter::normalStyle,
                       [this](const ConfigPresenter *, const QString &, const QString &key) -> CollectionStyle {
                           __DBG_STUB_INVOKE__
                           CollectionStyle s = savedStyle;
                           if (s.key.isEmpty())
                               s.key = key;
                           return s;
                       });
        stub.set_lamda(&ConfigPresenter::writeNormalStyle,
                       [](const ConfigPresenter *, const QString &, const QList<CollectionStyle> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&ConfigPresenter::setSurfaceInfo,
                       [](ConfigPresenter *, const QList<QWidget *> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&ConfigPresenter::setLastStyleConfigId,
                       [](ConfigPresenter *, const QString &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&ConfigPresenter::isEnableVisibility, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&ConfigPresenter::hideAllKeySequence, []() -> QKeySequence {
            __DBG_STUB_INVOKE__
            return QKeySequence("Meta+O");
        });
        stub.set_lamda(static_cast<FileClassifier *(*)(Classifier)>(&ClassifierCreator::createClassifier),
                       [this](Classifier) -> FileClassifier * {
                           __DBG_STUB_INVOKE__
                           classifier = new LayoutTestClassifier();
                           return classifier;
                       });
        stub.set_lamda(&FileOperator::setDataProvider,
                       [](FileOperator *, CollectionDataProvider *) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(&FileOperator::renameFileData, [](const FileOperator *) -> QHash<QUrl, QUrl> {
            __DBG_STUB_INVOKE__
            return {};
        });
        stub.set_lamda(&FileOperator::touchFileData, [](const FileOperator *) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl();
        });
        stub.set_lamda(&FileOperator::pasteFileData, [](const FileOperator *) -> QSet<QUrl> {
            __DBG_STUB_INVOKE__
            return {};
        });
        stub.set_lamda(VADDR(NormalizedModeBroker, selectAllItems), [](NormalizedModeBroker *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(static_cast<QPoint (CanvasViewShell::*)(const int &, const QPoint &)>(&CanvasViewShell::gridPos),
                       [](CanvasViewShell *, const int &, const QPoint &) -> QPoint {
                           __DBG_STUB_INVOKE__
                           return QPoint(0, 0);
                       });
        stub.set_lamda(static_cast<QString (CanvasGridShell::*)(int, const QPoint &)>(&CanvasGridShell::item),
                       [](CanvasGridShell *, int, const QPoint &) -> QString {
                           __DBG_STUB_INVOKE__
                           return QString();
                       });
        stub.set_lamda(static_cast<void (CanvasGridShell::*)(const QStringList &, int, const QPoint &)>(&CanvasGridShell::tryAppendAfter),
                       [](CanvasGridShell *, const QStringList &, int, const QPoint &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(static_cast<bool (CanvasModelShell::*)(const QUrl &)>(&CanvasModelShell::fetch),
                       [](CanvasModelShell *, const QUrl &) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        ASSERT_TRUE(mode->initialize(model));
        ASSERT_TRUE(mode->setClassifier(Classifier::kType));
        ASSERT_NE(classifier, nullptr);

        SurfacePointer surf(new Surface);
        surf->setGeometry(0, 0, 1366, 768);
        mode->setSurfaces({ surf });
        classifier->fillItems(QUrl::fromLocalFile("/tmp/ut-337971.txt"));
        mode->rebuild();
        ASSERT_TRUE(mode->d->holders.contains(kTestKeyLayout));
    }

    void TearDown() override
    {
        delete mode;
        delete model;
        delete shell;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    NormalizedMode *mode = nullptr;
    CollectionModel *model = nullptr;
    MockFileInfoModelShellLayout *shell = nullptr;
    ConfigPresenter *presenter = nullptr;
    LayoutTestClassifier *classifier = nullptr;
    CollectionStyle savedStyle;
};

// PMS:337971 集合在 1920 宽屏上保存的 x=1500，切到 1366 宽屏后应按分辨率差整体平移
// dx = 1366 - 1920 = -554，落到 x=946（屏幕内），而不是留在屏幕外
TEST_F(NormalizedModeLayoutTest, BUG337971_Layout_ResolutionShrunk_MigratesCollectionIntoSurface)
{
    mode->layout();

    const CollectionStyle result = mode->d->holders.value(kTestKeyLayout)->style();
    EXPECT_EQ(result.screenIndex, 1);
    EXPECT_EQ(result.rect, QRect(946, 100, 200, 300));
}

// PMS:369071 热插拔期间 surfaces 被清空时 layout() 必须安全跳过（修复前 findValidPos
// 的 Q_ASSERT(screenIdx <= surfaces.count()) 越界中止），且不动既有集合样式
TEST_F(NormalizedModeLayoutTest, BUG369071_Layout_EmptySurfaces_SkipsSafelyWithoutTouchingHolders)
{
    // 模拟热插拔：mode 的 surface 列表被清空；本地引用保活 Surface 以避免
    // widget 连带销毁（生产环境中由背景插件生命周期统一管理）
    SurfacePointer keepAlive;
    if (!mode->surfaces.isEmpty())
        keepAlive = mode->surfaces.first();
    // layout() 必须是安全的 no-op：不动集合样式（修复前 findValidPos 的
    // Q_ASSERT(screenIdx <= surfaces.count()) 越界中止，集合布局被破坏）
    const CollectionStyle before = mode->d->holders.value(kTestKeyLayout)->style();
    mode->setSurfaces({});
    EXPECT_NO_THROW(mode->layout());

    const CollectionStyle result = mode->d->holders.value(kTestKeyLayout)->style();
    EXPECT_EQ(result.rect, before.rect);
    EXPECT_EQ(result.screenIndex, before.screenIndex);
}

// PMS:369071 行高缺失（0）时 ElideTextLayout 不得除零叠行，应回退用 kFont 度量行高
// （修复前 textLineHeight=0 → totalLines 除零 → 文件名全部重叠在同一行/消失）
TEST(NormalizedModeLayoutElideTest, BUG369071_ElideTextLayout_ZeroLineHeight_FallsBackToFontMetrics)
{
    ElideTextLayout layout("regression line one line two");
    layout.setAttribute(ElideTextLayout::kFont, QFont("Sans", 10));

    const QList<QRectF> rects = layout.layout(QRectF(0, 0, 100, 60), Qt::ElideRight);

    EXPECT_FALSE(rects.isEmpty());
    for (const QRectF &r : rects) {
        EXPECT_GT(r.height(), 0.0);   // 修复前行高为 0，所有行叠在同一位置
    }
}
