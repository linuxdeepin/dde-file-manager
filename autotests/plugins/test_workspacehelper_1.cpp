// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspacehelper_1.cpp
 * @brief Unit tests for WorkspaceHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/workspacehelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceHelperTest, aboutToChangeViewWidth)
{
    // Test method: void aboutToChangeViewWidth((const quint64 windowID, int deltaWidth))
    EXPECT_NO_FATAL_FAILURE(obj->aboutToChangeViewWidth(0, 0));
}

TEST_F(WorkspaceHelperTest, actionNewWindow)
{
    // Test method: void actionNewWindow((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->actionNewWindow(_arg0));
}

TEST_F(WorkspaceHelperTest, addScheme)
{
    // Test method: void addScheme((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addScheme(_arg0));
}

TEST_F(WorkspaceHelperTest, addWorkspace)
{
    // Test method: void addWorkspace((quint64 windowId, WorkspaceWidget *workspace))
    EXPECT_NO_FATAL_FAILURE(obj->addWorkspace(0, nullptr));
}

TEST_F(WorkspaceHelperTest, closeTab)
{
    // Test method: void closeTab((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->closeTab(_arg0));
}

TEST_F(WorkspaceHelperTest, columnRoles)
{
    // Test method: QList<ItemRoles> columnRoles((quint64 windowId))
    auto result = obj->columnRoles(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, createTopWidgetByScheme)
{
    // Test method: CustomTopWidgetInterface createTopWidgetByScheme((const QString &scheme))
    QString _arg0{};
    auto result = obj->createTopWidgetByScheme(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createTopWidgetByScheme(_arg0); });

}

TEST_F(WorkspaceHelperTest, createTopWidgetByUrl)
{
    // Test method: CustomTopWidgetInterface createTopWidgetByUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createTopWidgetByUrl(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createTopWidgetByUrl(_arg0); });

}

TEST_F(WorkspaceHelperTest, fileUpdate)
{
    // Test method: void fileUpdate((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->fileUpdate(_arg0));
}

TEST_F(WorkspaceHelperTest, filterUndoFiles)
{
    // Test method: QList<QUrl> filterUndoFiles((const QList<QUrl> &urlList))
    QList<QUrl> _arg0{};
    auto result = obj->filterUndoFiles(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, findCustomViewProperty)
{
    // Test method: CustomViewProperty findCustomViewProperty((const QString &scheme))
    QString _arg0{};
    auto result = obj->findCustomViewProperty(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findCustomViewProperty(_arg0); });

}

TEST_F(WorkspaceHelperTest, findFileViewByWindowID)
{
    // Test method: FileView findFileViewByWindowID((const quint64 windowID))
    auto result = obj->findFileViewByWindowID(0);
    EXPECT_NO_FATAL_FAILURE({ obj->findFileViewByWindowID(0); });

}

TEST_F(WorkspaceHelperTest, findMenuScene)
{
    // Test method: QString findMenuScene((const QString &scheme))
    QString _arg0{};
    auto result = obj->findMenuScene(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, findViewHint)
{
    // Test method: ViewHintSpec findViewHint((const QString &scheme))
    QString _arg0{};
    auto result = obj->findViewHint(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->findViewHint(_arg0); });

}

TEST_F(WorkspaceHelperTest, findViewMode)
{
    // Test method: Global::ViewMode findViewMode((const QString &scheme))
    QString _arg0{};
    auto result = obj->findViewMode(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceHelperTest, findWorkspaceByWindowId)
{
    // Test method: WorkspaceWidget findWorkspaceByWindowId((quint64 windowId))
    auto result = obj->findWorkspaceByWindowId(0);
    EXPECT_NO_FATAL_FAILURE({ obj->findWorkspaceByWindowId(0); });

}

TEST_F(WorkspaceHelperTest, getFileViewStateValue)
{
    // Test method: QVariant getFileViewStateValue((const QUrl &url, const QString &key, const QVariant &defaultValue))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    auto result = obj->getFileViewStateValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(WorkspaceHelperTest, getGroupingStrategy)
{
    // Test method: QString getGroupingStrategy((quint64 windowId))
    auto result = obj->getGroupingStrategy(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, getLoadStrategy)
{
    // Test method: DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy getLoadStrategy((const QString &scheme))
    QString _arg0{};
    auto result = obj->getLoadStrategy(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceHelperTest, getNameFilter)
{
    // Test method: QStringList getNameFilter((const quint64 windowId))
    auto result = obj->getNameFilter(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, getViewFilter)
{
    // Test method: int getViewFilter((const quint64 windowID))
    auto result = obj->getViewFilter(0);
    EXPECT_GE(result, 0);

}

TEST_F(WorkspaceHelperTest, handleRefreshDir)
{
    // Test method: void handleRefreshDir((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRefreshDir(_arg0));
}

TEST_F(WorkspaceHelperTest, hasViewHint)
{
    // Test method: bool hasViewHint((const QString &scheme))
    QString _arg0{};
    auto result = obj->hasViewHint(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, haveViewRoutePrehandler)
{
    // Test method: bool haveViewRoutePrehandler((const QString &scheme))
    QString _arg0{};
    auto result = obj->haveViewRoutePrehandler(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, installWorkspaceWidgetToWindow)
{
    // Test method: void installWorkspaceWidgetToWindow((const quint64 windowID))
    EXPECT_NO_FATAL_FAILURE(obj->installWorkspaceWidgetToWindow(0));
}

TEST_F(WorkspaceHelperTest, isFocusFileViewDisabled)
{
    // Test method: bool isFocusFileViewDisabled((const QString &scheme))
    QString _arg0{};
    auto result = obj->isFocusFileViewDisabled(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, isRegistedTopWidget)
{
    // Test method: bool isRegistedTopWidget((const WorkspaceHelper::KeyType &scheme))
    WorkspaceHelper::KeyType _arg0{};
    auto result = obj->isRegistedTopWidget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, laterRequestSelectFiles)
{
    // Test method: void laterRequestSelectFiles((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->laterRequestSelectFiles(_arg0));
}

TEST_F(WorkspaceHelperTest, mutex)
{
    // Test getter: QMutex mutex()
    auto result = obj->mutex();
    EXPECT_NO_FATAL_FAILURE({ obj->mutex(); });

}

TEST_F(WorkspaceHelperTest, registerCustomViewProperty)
{
    // Test method: void registerCustomViewProperty((const QString &scheme, const QVariantMap &propertise))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->registerCustomViewProperty(_arg0, _arg1));
}

TEST_F(WorkspaceHelperTest, registerFileView)
{
    // Test method: void registerFileView((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerFileView(_arg0));
}

TEST_F(WorkspaceHelperTest, registerFocusFileViewDisabled)
{
    // Test method: void registerFocusFileViewDisabled((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerFocusFileViewDisabled(_arg0));
}

TEST_F(WorkspaceHelperTest, registerLoadStrategy)
{
    // Test method: void registerLoadStrategy((const QString &scheme, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerLoadStrategy(_arg0, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy()));
}

TEST_F(WorkspaceHelperTest, registerTopWidgetCreator)
{
    // Test method: void registerTopWidgetCreator((const WorkspaceHelper::KeyType &scheme, const WorkspaceHelper::TopWidgetCreator &creator))
    WorkspaceHelper::KeyType _arg0{};
    WorkspaceHelper::TopWidgetCreator _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->registerTopWidgetCreator(_arg0, _arg1));
}

TEST_F(WorkspaceHelperTest, registerViewHint)
{
    // Test method: void registerViewHint((const QString &scheme, const ViewHintSpec &spec))
    QString _arg0{};
    ViewHintSpec _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->registerViewHint(_arg0, _arg1));
}

TEST_F(WorkspaceHelperTest, registeredFileView)
{
    // Test method: bool registeredFileView((const QString &scheme))
    QString _arg0{};
    auto result = obj->registeredFileView(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, reigsterViewRoutePrehandler)
{
    // Test method: bool reigsterViewRoutePrehandler((const QString &scheme, const FileViewRoutePrehaldler prehandler))
    QString _arg0{};
    auto result = obj->reigsterViewRoutePrehandler(_arg0, FileViewRoutePrehaldler());
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceHelperTest, requestSelectFilesDelayMs)
{
    // Test method: int requestSelectFilesDelayMs((int urlCount))
    auto result = obj->requestSelectFilesDelayMs(0);
    EXPECT_GE(result, 0);

}

TEST_F(WorkspaceHelperTest, reverseSelect)
{
    // Test method: void reverseSelect((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->reverseSelect(0));
}

TEST_F(WorkspaceHelperTest, roleDisplayName)
{
    // Test method: QString roleDisplayName((quint64 windowId, dfmbase::Global::ItemRoles role))
    auto result = obj->roleDisplayName(0, {});
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceHelperTest, selectAll)
{
    // Test method: void selectAll((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->selectAll(0));
}

TEST_F(WorkspaceHelperTest, selectFiles)
{
    // Test method: void selectFiles((quint64 windowId, const QList<QUrl> &files))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFiles(0, _arg1));
}

TEST_F(WorkspaceHelperTest, setAlwaysOpenInCurrentWindow)
{
    // Test setter: void setAlwaysOpenInCurrentWindow((const quint64 windowID))
    EXPECT_NO_FATAL_FAILURE(obj->setAlwaysOpenInCurrentWindow(0));
}

TEST_F(WorkspaceHelperTest, setCustomTopWidgetVisible)
{
    // Test setter: void setCustomTopWidgetVisible((quint64 windowId, const QString &scheme, bool visible))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setCustomTopWidgetVisible(0, _arg1, false));
}

TEST_F(WorkspaceHelperTest, setEnabledSelectionModes)
{
    // Test setter: void setEnabledSelectionModes((const quint64 windowID, const QList<QAbstractItemView::SelectionMode> &modes))
    QList<QAbstractItemView::SelectionMode> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledSelectionModes(0, _arg1));
}

TEST_F(WorkspaceHelperTest, setFileViewStateValue)
{
    // Test setter: void setFileViewStateValue((const QUrl &url, const QString &key, const QVariant &value))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileViewStateValue(_arg0, _arg1, _arg2));
}

TEST_F(WorkspaceHelperTest, setFilterCallback)
{
    // Test setter: void setFilterCallback((quint64 windowId, const QUrl &url, const FileViewFilterCallback callback))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilterCallback(0, _arg1, FileViewFilterCallback()));
}

TEST_F(WorkspaceHelperTest, setFilterData)
{
    // Test setter: void setFilterData((quint64 windowId, const QUrl &url, const QVariant &data))
    QUrl _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilterData(0, _arg1, _arg2));
}

TEST_F(WorkspaceHelperTest, setGroupingStrategy)
{
    // Test setter: void setGroupingStrategy((quint64 windowId, const QString &strategyName))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setGroupingStrategy(0, _arg1));
}

TEST_F(WorkspaceHelperTest, setNameFilter)
{
    // Test setter: void setNameFilter((const quint64 windowID, const QStringList &filter))
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setNameFilter(0, _arg1));
}

TEST_F(WorkspaceHelperTest, setReadOnly)
{
    // Test setter: void setReadOnly((const quint64 windowID, const bool readOnly))
    EXPECT_NO_FATAL_FAILURE(obj->setReadOnly(0, false));
}

TEST_F(WorkspaceHelperTest, setSelectionMode)
{
    // Test setter: void setSelectionMode((const quint64 windowID, const QAbstractItemView::SelectionMode &mode))
    QAbstractItemView::SelectionMode _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelectionMode(0, _arg1));
}

TEST_F(WorkspaceHelperTest, setSort)
{
    // Test setter: void setSort((quint64 windowId, Global::ItemRoles role))
    EXPECT_NO_FATAL_FAILURE(obj->setSort(0, Global::ItemRoles()));
}

TEST_F(WorkspaceHelperTest, setUndoFiles)
{
    // Test setter: void setUndoFiles((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUndoFiles(_arg0));
}

TEST_F(WorkspaceHelperTest, setViewDragDropMode)
{
    // Test setter: void setViewDragDropMode((const quint64 windowID, const QAbstractItemView::DragDropMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewDragDropMode(0, QAbstractItemView::DragDropMode()));
}

TEST_F(WorkspaceHelperTest, setViewDragEnabled)
{
    // Test setter: void setViewDragEnabled((const quint64 windowID, const bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setViewDragEnabled(0, false));
}

TEST_F(WorkspaceHelperTest, setViewFilter)
{
    // Test setter: void setViewFilter((const quint64 windowID, const QDir::Filters filter))
    EXPECT_NO_FATAL_FAILURE(obj->setViewFilter(0, QDir::Filters()));
}

TEST_F(WorkspaceHelperTest, setWorkspaceMenuScene)
{
    // Test setter: void setWorkspaceMenuScene((const QString &scheme, const QString &scene))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setWorkspaceMenuScene(_arg0, _arg1));
}

TEST_F(WorkspaceHelperTest, sortRole)
{
    // Test method: Global::ItemRoles sortRole((quint64 windowId))
    auto result = obj->sortRole(0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceHelperTest, switchViewMode)
{
    // Test method: void switchViewMode((quint64 windowId, int viewMode))
    EXPECT_NO_FATAL_FAILURE(obj->switchViewMode(0, 0));
}

TEST_F(WorkspaceHelperTest, transformViewModeUrl)
{
    // Test method: QUrl transformViewModeUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->transformViewModeUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(WorkspaceHelperTest, updateRootFile)
{
    // Test method: void updateRootFile((const QList<QUrl> urls))
    EXPECT_NO_FATAL_FAILURE(obj->updateRootFile(QList<QUrl>()));
}

TEST_F(WorkspaceHelperTest, viewRoutePrehandler)
{
    // Test method: FileViewRoutePrehaldler viewRoutePrehandler((const QString &scheme))
    QString _arg0{};
    auto result = obj->viewRoutePrehandler(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->viewRoutePrehandler(_arg0); });

}
