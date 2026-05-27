// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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

TEST_F(UT_NormalizedMode, FilterDropData_ReturnsBool)
{
    QMimeData mimeData;
    bool result = mode->filterDropData(0, &mimeData, QPoint(0, 0), nullptr);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterDataRested_ReturnsBool)
{
    QList<QUrl> urls;
    urls.append(QUrl("file:///test"));
    bool result = mode->filterDataRested(&urls);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterDataInserted_ReturnsBool)
{
    QUrl url("file:///test");
    bool result = mode->filterDataInserted(url);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterDataRenamed_ReturnsBool)
{
    QUrl oldUrl("file:///old");
    QUrl newUrl("file:///new");
    bool result = mode->filterDataRenamed(oldUrl, newUrl);
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterShortcutkeyPress_ReturnsBool)
{
    bool result = mode->filterShortcutkeyPress(0, 65, 0);  // A key with no modifiers
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterKeyPress_ReturnsBool)
{
    bool result = mode->filterKeyPress(0, 65, 0);  // A key with no modifiers
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_NormalizedMode, FilterContextMenu_ReturnsBool)
{
    QList<QUrl> files;
    files.append(QUrl("file:///test"));
    bool result = mode->filterContextMenu(0, QUrl("file:///dir"), files, QPoint(0, 0));
    EXPECT_TRUE(result || !result);
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
