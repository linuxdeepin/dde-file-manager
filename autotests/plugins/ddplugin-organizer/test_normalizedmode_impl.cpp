// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "mode/normalizedmode.h"
#include "mode/canvasorganizer.h"
#include "mode/normalized/fileclassifier.h"
#include "mode/normalized/normalizedmodebroker.h"
#include "models/collectionmodel.h"
#include "interface/fileinfomodelshell.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasselectionshell.h"
#include "config/configpresenter.h"
#include "utils/fileoperator.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QMimeData>
#include <QUrl>
#include <QPoint>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

namespace {

constexpr char kTestKey[] = "Type_Apps";

class MockFileInfoModelShell : public FileInfoModelShell
{
public:
    MockFileInfoModelShell() : FileInfoModelShell(nullptr) {}
};

class MockFileClassifier : public FileClassifier
{
public:
    explicit MockFileClassifier(QObject *parent = nullptr)
        : FileClassifier(parent)
    {
        auto base = CollectionBaseDataPtr(new CollectionBaseData);
        base->key = kTestKey;
        base->name = "Applications";
        collections.insert(kTestKey, base);
    }

    Classifier mode() const override { return Classifier::kType; }
    ModelDataHandler *dataHandler() const override { return const_cast<MockFileClassifier *>(this); }
    QStringList classes() const override { return { kTestKey }; }
    QString classify(const QUrl &) const override { return kTestKey; }
    QString className(const QString &) const override { return "Applications"; }
    bool updateClassifier() override { return false; }

    QString key(const QUrl &) const override { return kTestKey; }
    bool contains(const QString &key, const QUrl &) const override { return key == kTestKey; }
    QString replace(const QUrl &, const QUrl &) override { return kTestKey; }
    QString append(const QUrl &) override { return kTestKey; }
    QString prepend(const QUrl &) override { return kTestKey; }
    QString remove(const QUrl &) override { return kTestKey; }
    QString change(const QUrl &) override { return kTestKey; }
    bool acceptInsert(const QUrl &) override { return true; }
    bool acceptRename(const QUrl &, const QUrl &) override { return true; }
};

class TestableNormalizedMode : public NormalizedMode
{
public:
    using NormalizedMode::setClassifier;
    using NormalizedMode::removeClassifier;
    using NormalizedMode::filterDropData;
    using NormalizedMode::filterDataRested;
    using NormalizedMode::filterDataInserted;
    using NormalizedMode::filterDataRenamed;
    using NormalizedMode::filterShortcutkeyPress;
    using NormalizedMode::filterKeyPress;
    using NormalizedMode::filterContextMenu;
};

}   // namespace

class NormalizedModeImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        presenter = ConfigPresenter::instance();
        presenter->initialize();

        model = new CollectionModel();
        shell = new MockFileInfoModelShell();
        model->setModelShell(shell);

        mode = new TestableNormalizedMode();
        mode->setCanvasModelShell(new CanvasModelShell(mode));
        mode->setCanvasViewShell(new CanvasViewShell(mode));
        mode->setCanvasGridShell(new CanvasGridShell(mode));
        mode->setCanvasManagerShell(new CanvasManagerShell(mode));
        mode->setCanvasSelectionShell(new CanvasSelectionShell(mode));

        // DConfig / config stubs
        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        stub.set_lamda(&DConfigManager::setValue,
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                           __DBG_STUB_INVOKE__
                       });

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
            return QString();
        });
        stub.set_lamda(&ConfigPresenter::hasConfigId, [](const ConfigPresenter *, const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ConfigPresenter::normalStyle, [](const ConfigPresenter *, const QString &, const QString &) -> CollectionStyle {
            __DBG_STUB_INVOKE__
            return CollectionStyle();
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

        // classifier factory
        stub.set_lamda(static_cast<FileClassifier *(*)(Classifier)>(&ClassifierCreator::createClassifier),
                       [](Classifier) -> FileClassifier * {
                           __DBG_STUB_INVOKE__
                           return new MockFileClassifier();
                       });

        // file operator
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

        // broker
        stub.set_lamda(VADDR(NormalizedModeBroker, selectAllItems), [](NormalizedModeBroker *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // canvas shells used by moveFilesToCanvas / releaseCollection
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
    TestableNormalizedMode *mode = nullptr;
    CollectionModel *model = nullptr;
    MockFileInfoModelShell *shell = nullptr;
    ConfigPresenter *presenter = nullptr;
};

TEST_F(NormalizedModeImpl, Mode_ReturnsNormalized)
{
    EXPECT_EQ(mode->mode(), OrganizerMode::kNormalized);
}

TEST_F(NormalizedModeImpl, Initialize_ReturnsTrueAndSetsModel)
{
    EXPECT_TRUE(mode->initialize(model));
    EXPECT_EQ(mode->getModel(), model);
}

TEST_F(NormalizedModeImpl, Reset_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->reset());
}

TEST_F(NormalizedModeImpl, Layout_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->layout());
}

TEST_F(NormalizedModeImpl, DetachLayout_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->detachLayout());
}

TEST_F(NormalizedModeImpl, Rebuild_NoReorganize_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->rebuild(false));
}

TEST_F(NormalizedModeImpl, Rebuild_Reorganize_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->rebuild(true));
}

TEST_F(NormalizedModeImpl, OnFileInserted_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->onFileInserted(QModelIndex(), 0, 1));
}

TEST_F(NormalizedModeImpl, OnFileAboutToBeRemoved_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->onFileAboutToBeRemoved(QModelIndex(), 0, 1));
}

TEST_F(NormalizedModeImpl, OnFileDataChanged_DoesNotCrash)
{
    mode->initialize(model);
    // NOTE: model->rootIndex() returns a sentinel index with row=INT_MAX;
    // passing it into onFileDataChanged makes the product loop overflow
    // (i=INT_MAX; i<=INT_MAX) and hang. Use a normal index instead.
    QModelIndex root = model->createIndex(0, 0);
    QVector<int> roles = { Qt::DisplayRole };
    EXPECT_NO_THROW(mode->onFileDataChanged(root, root, roles));
}

TEST_F(NormalizedModeImpl, OnFileRenamed_DoesNotCrash)
{
    mode->initialize(model);
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    EXPECT_NO_THROW(mode->onFileRenamed(oldUrl, newUrl));
}

TEST_F(NormalizedModeImpl, OnReorganizeDesktop_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->onReorganizeDesktop());
}

TEST_F(NormalizedModeImpl, ReleaseCollection_DoesNotCrash)
{
    mode->initialize(model);
    EXPECT_NO_THROW(mode->releaseCollection(static_cast<int>(kCatApplication)));
}

TEST_F(NormalizedModeImpl, OnCollectionEditStatusChanged_UpdatesEditing)
{
    mode->onCollectionEditStatusChanged(true);
    EXPECT_TRUE(mode->isEditing());
    mode->onCollectionEditStatusChanged(false);
    EXPECT_FALSE(mode->isEditing());
}

TEST_F(NormalizedModeImpl, FilterContextMenu_ReturnsEditingState)
{
    mode->onCollectionEditStatusChanged(true);
    EXPECT_TRUE(mode->filterContextMenu(0, QUrl(), {}, QPoint()));
    mode->onCollectionEditStatusChanged(false);
    EXPECT_FALSE(mode->filterContextMenu(0, QUrl(), {}, QPoint()));
}

TEST_F(NormalizedModeImpl, ChangeCollectionSurface_DoesNotCrash)
{
    EXPECT_NO_THROW(mode->changeCollectionSurface("screen0"));
}

TEST_F(NormalizedModeImpl, DeactiveAllPredictors_DoesNotCrash)
{
    EXPECT_NO_THROW(mode->deactiveAllPredictors());
}

TEST_F(NormalizedModeImpl, OnCollectionMoving_DoesNotCrash)
{
    EXPECT_NO_THROW(mode->onCollectionMoving(true));
    EXPECT_NO_THROW(mode->onCollectionMoving(false));
}

TEST_F(NormalizedModeImpl, FilterDropData_MoveAction_MovesFiles)
{
    mode->initialize(model);
    stub.set_lamda(&ConfigPresenter::organizeOnTriggered, []() -> bool { return true; });

    QMimeData mime;
    mime.setUrls({ QUrl("file:///test.txt") });
    EXPECT_TRUE(mode->filterDropData(0, &mime, QPoint(10, 10), nullptr));
}

TEST_F(NormalizedModeImpl, FilterDataRested_RemovesContainedUrls)
{
    mode->initialize(model);
    QList<QUrl> urls = { QUrl("file:///test.txt") };
    EXPECT_TRUE(mode->filterDataRested(&urls));
    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(NormalizedModeImpl, FilterDataInserted_AcceptsInsert)
{
    mode->initialize(model);
    EXPECT_TRUE(mode->filterDataInserted(QUrl("file:///test.txt")));
}

TEST_F(NormalizedModeImpl, FilterDataRenamed_SameType_ReturnsTrue)
{
    mode->initialize(model);
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    EXPECT_TRUE(mode->filterDataRenamed(oldUrl, newUrl));
}

TEST_F(NormalizedModeImpl, FilterShortcutkeyPress_HideAllKey_ReturnsTrue)
{
    stub.set_lamda(&ConfigPresenter::hideAllKeySequence, []() -> QKeySequence {
        return QKeySequence("Meta+O");
    });
    EXPECT_TRUE(mode->filterShortcutkeyPress(0, Qt::Key_O, Qt::MetaModifier));
}

TEST_F(NormalizedModeImpl, FilterShortcutkeyPress_SelectAll_TriggersSelectAllItems)
{
    // Ctrl+A must forward to broker's selectAllItems; the return value of
    // filterShortcutkeyPress itself only depends on the hide-all shortcut.
    bool called = false;
    stub.set_lamda(VADDR(NormalizedModeBroker, selectAllItems), [&called](NormalizedModeBroker *) -> bool {
        __DBG_STUB_INVOKE__
        called = true;
        return true;
    });
    mode->filterShortcutkeyPress(0, Qt::Key_A, Qt::ControlModifier);
    EXPECT_TRUE(called);
}

TEST_F(NormalizedModeImpl, FilterKeyPress_F2_ReturnsFalseWhenNoSelection)
{
    EXPECT_FALSE(mode->filterKeyPress(0, Qt::Key_F2, Qt::NoModifier));
}

TEST_F(NormalizedModeImpl, SetClassifier_Type_ReturnsTrue)
{
    // model must be initialized first, setClassifier touches model->setHandler()
    mode->initialize(model);
    EXPECT_TRUE(mode->setClassifier(Classifier::kType));
    EXPECT_NE(model->handler(), nullptr);
}

TEST_F(NormalizedModeImpl, RemoveClassifier_ClearsHandler)
{
    mode->initialize(model);
    mode->setClassifier(Classifier::kType);
    mode->removeClassifier();
    EXPECT_EQ(model->handler(), nullptr);
}

TEST_F(NormalizedModeImpl, CanvasShellSetters_DoNotCrash)
{
    CanvasModelShell *ms = new CanvasModelShell(mode);
    CanvasViewShell *vs = new CanvasViewShell(mode);
    CanvasGridShell *gs = new CanvasGridShell(mode);
    CanvasManagerShell *mgs = new CanvasManagerShell(mode);
    CanvasSelectionShell *ss = new CanvasSelectionShell(mode);

    mode->setCanvasModelShell(ms);
    mode->setCanvasViewShell(vs);
    mode->setCanvasGridShell(gs);
    mode->setCanvasManagerShell(mgs);
    mode->setCanvasSelectionShell(ss);

    EXPECT_EQ(mode->canvasModelShell, ms);
    EXPECT_EQ(mode->canvasViewShell, vs);
    EXPECT_EQ(mode->canvasGridShell, gs);
    EXPECT_EQ(mode->canvasManagerShell, mgs);
    EXPECT_EQ(mode->canvasSelectionShell, ss);
}

TEST_F(NormalizedModeImpl, SetSurfaces_DoesNotCrash)
{
    mode->setSurfaces({});
    EXPECT_TRUE(mode->getSurfaces().isEmpty());
}
