// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfm-base/utils/protocolutils.h"
#include "groups/groupingengine.h"
#include "groups/filegroupdata.h"
#include "groups/modelitemwrapper.h"
#include "models/fileitemdata.h"
#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfile.h>
#include <dfm-io/dfileinfo.h>
#include <dfm-base/file/local/private/asyncfileinfo_p.h>
#include <dfm-base/utils/universalutils.h>
#include <gtest/gtest.h>

#include <QUrl>
#include <QVariant>
#include <QHash>
#include <QList>
#include <QElapsedTimer>
#include <QDebug>
#include <QPair>

#include "stubext.h"

DPWORKSPACE_BEGIN_NAMESPACE

// Mock strategy for testing
class MockGroupStrategy : public dfmbase::AbstractGroupStrategy
{
public:
    MockGroupStrategy(const QString &name = "MockStrategy")
        : strategyName(name)
    {
    }

    QString getStrategyName() const override
    {
        return strategyName;
    }

    QString getGroupKey(const FileInfoPointer &fileInfo) const override
    {
        if (!fileInfo) return "";
        return "mock_group";
    }

    QString getGroupDisplayName(const QString &groupKey) const override
    {
        return "Mock Group";
    }

    QStringList getGroupOrder() const override
    {
        return QStringList();
    }

    int getGroupDisplayOrder(const QString &groupKey) const override
    {
        return 1;
    }

    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override
    {
        return !infos.isEmpty();
    }

private:
    QString strategyName;
};

class TestGroupingEngine : public testing::Test
{
public:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/test");
        engine = new GroupingEngine(testUrl);

        // Mock UniversalUtils::urlEquals
        stub.set_lamda(&dfmbase::UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
            __DBG_STUB_INVOKE__
            return url1.toString() == url2.toString();
        });

        // Mock ProtocolUtils::isLocalFile
        stub.set_lamda(&dfmbase::ProtocolUtils::isLocalFile, [](const QUrl &url) -> bool {
            __DBG_STUB_INVOKE__
            return url.scheme() == "file";
        });

        // Mock InfoFactory::create
        stub.set_lamda(&dfmbase::InfoFactory::create<dfmbase::FileInfo>, [](const QUrl &url, dfmbase::Global::CreateFileInfoType type, QString *) -> FileInfoPointer {
            __DBG_STUB_INVOKE__
            Q_UNUSED(type)
            Q_UNUSED(url)
            // Create a mock FileInfo using the concrete class
            // Create a mock FileInfo that satisfies the Q_ASSERT
            class MockFileInfo : public dfmbase::FileInfo {
            public:
                MockFileInfo(const QUrl &url) : dfmbase::FileInfo(url) {}
                QString nameOf(dfmbase::FileInfo::FileNameInfoType type) const override {
                    Q_UNUSED(type);
                    return "mock";
                }
                QString displayOf(dfmbase::FileInfo::DisplayInfoType type) const override {
                    Q_UNUSED(type);
                    return "mock";
                }
                QString pathOf(dfmbase::FileInfo::FilePathInfoType type) const override {
                    Q_UNUSED(type);
                    return "/mock";
                }
                bool exists() const override { return true; }
                bool isAttributes(dfmbase::FileInfo::FileIsType type) const override {
                    Q_UNUSED(type);
                    return false;
                }
                qint64 size() const override { return 1024; }
                QVariant timeOf(dfmbase::FileInfo::FileTimeType type) const override {
                    Q_UNUSED(type);
                    return QDateTime::currentDateTime();
                }
                bool isReadable() const override { return true; }
                bool isWritable() const override { return true; }
                bool isExecutable() const override { return false; }
                QIcon fileIcon() override { return QIcon(); }
                QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override {
                    Q_UNUSED(mode);
                    return QMimeType();
                }
                QVariant customData(int role) const override {
                    Q_UNUSED(role);
                    return QVariant();
                }
            };
            
            return FileInfoPointer(new MockFileInfo(url));
        });
        
        // Mock getFileInfoFromFileItem to return a valid mock FileInfo
        stub.set_lamda(ADDR(GroupingEngine, getFileInfoFromFileItem), [this](GroupingEngine *, const FileItemDataPointer &file) -> FileInfoPointer {
            __DBG_STUB_INVOKE__
            if (!file) {
                return FileInfoPointer();
            }
            // Create a mock FileInfo using the interface
            class MockFileInfo : public dfmbase::FileInfo {
            public:
                MockFileInfo(const QUrl &url) : dfmbase::FileInfo(url) {}
                
                // Basic file info methods
                QUrl fileUrl() const override { return QUrl("file:///mock"); }
                bool exists() const override { return true; }
                void refresh() override {}
                QString filePath() const override { return "/mock/file.txt"; }
                QString absoluteFilePath() const override { return "/mock/file.txt"; }
                QString fileName() const override { return "file.txt"; }
                QString baseName() const override { return "file"; }
                QString completeBaseName() const override { return "file"; }
                QString suffix() const override { return "txt"; }
                QString completeSuffix() const override { return "txt"; }
                QString path() const override { return "/mock"; }
                QString absolutePath() const override { return "/mock"; }
                
                // Permission methods
                bool isReadable() const override { return true; }
                bool isWritable() const override { return true; }
                bool isExecutable() const override { return false; }
                bool isHidden() const override { return false; }
                bool isNativePath() const override { return true; }
                
                // File type methods
                bool isFile() const override { return true; }
                bool isDir() const override { return false; }
                bool isSymLink() const override { return false; }
                bool isRoot() const override { return false; }
                bool isBundle() const override { return false; }
                QString symLinkTarget() const override { return ""; }
                
                // Owner and group methods
                QString owner() const override { return "user"; }
                uint ownerId() const override { return 1000; }
                QString group() const override { return "group"; }
                uint groupId() const override { return 1000; }
                
                // Permission methods
                bool permission(QFile::Permissions permissions) const override {
                    Q_UNUSED(permissions);
                    return true;
                }
                QFile::Permissions permissions() const override {
                    return QFile::ReadOwner | QFile::WriteOwner;
                }
                int countChildFile() const override { return 0; }
                qint64 size() const override { return 1024; }
                
                // Time methods
                quint32 birthTime() const override { return 1234567890; }
                quint32 metadataChangeTime() const override { return 1234567890; }
                quint32 lastModified() const override { return 1234567890; }
                quint32 lastRead() const override { return 1234567890; }
                
                // FileInfo specific methods
                QString nameOf(dfmbase::FileInfo::FileNameInfoType type) const override {
                    Q_UNUSED(type);
                    return "file.txt";
                }
                QString displayOf(dfmbase::FileInfo::DisplayInfoType type) const override {
                    Q_UNUSED(type);
                    return "file.txt";
                }
                QString pathOf(dfmbase::FileInfo::FilePathInfoType type) const override {
                    Q_UNUSED(type);
                    return "/mock";
                }
                bool isAttributes(dfmbase::FileInfo::FileIsType type) const override {
                    Q_UNUSED(type);
                    return false;
                }
                QVariant timeOf(dfmbase::FileInfo::FileTimeType type) const override {
                    Q_UNUSED(type);
                    return QDateTime::currentDateTime();
                }
                QIcon fileIcon() override { return QIcon(); }
                QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override {
                    Q_UNUSED(mode);
                    return QMimeType();
                }
                QVariant customData(int role) const override {
                    Q_UNUSED(role);
                    return QVariant();
                }
            };
            
            return FileInfoPointer(new MockFileInfo(file->data(dfmbase::Global::ItemRoles::kItemUrlRole).toUrl()));
        });

        // Create test file item data
        QUrl file1Url = QUrl::fromLocalFile("/test/file1.txt");
        QUrl file2Url = QUrl::fromLocalFile("/test/file2.txt");
        testFileData = FileItemDataPointer::create(file1Url);
        testFileData2 = FileItemDataPointer::create(file2Url);

        // Setup test data structures
        visibleChildren.append(QUrl::fromLocalFile("/test/file1.txt"));
        visibleChildren.append(QUrl::fromLocalFile("/test/file2.txt"));

        childrenDataMap.insert(QUrl::fromLocalFile("/test/file1.txt"), testFileData);
        childrenDataMap.insert(QUrl::fromLocalFile("/test/file2.txt"), testFileData2);

        engine->setVisibleChildren(&visibleChildren);
        engine->setChildrenDataMap(&childrenDataMap);
    }

    void TearDown() override
    {
        delete engine;
        stub.clear();
    }

    GroupingEngine *engine;
    QUrl testUrl;
    FileItemDataPointer testFileData;
    FileItemDataPointer testFileData2;
    QList<QUrl> visibleChildren;
    QHash<QUrl, FileItemDataPointer> childrenDataMap;
    stub_ext::StubExt stub;
};

TEST_F(TestGroupingEngine, Constructor)
{
    GroupingEngine testEngine(testUrl);
    EXPECT_EQ(testEngine.currentUpdateMode(), GroupingEngine::UpdateMode::kNoGrouping);
    auto range = testEngine.currentUpdateChildrenRange();
    EXPECT_EQ(range.first, 0);
    EXPECT_EQ(range.second, 0);
}

TEST_F(TestGroupingEngine, Destructor)
{
    // Just test that destructor doesn't crash
    GroupingEngine *testEngine = new GroupingEngine(testUrl);
    delete testEngine;
}

TEST_F(TestGroupingEngine, SetGroupOrder)
{
    // Test setting ascending order
    engine->setGroupOrder(Qt::AscendingOrder);
    
    // Test setting descending order
    engine->setGroupOrder(Qt::DescendingOrder);
    
    // Test setting same order again (should not cause issues)
    engine->setGroupOrder(Qt::DescendingOrder);
}

TEST_F(TestGroupingEngine, SetVisibleTreeChildren)
{
    QHash<QUrl, QList<QUrl>> treeChildren;
    engine->setVisibleTreeChildren(&treeChildren);
}

TEST_F(TestGroupingEngine, SetChildrenDataMap)
{
    QHash<QUrl, FileItemDataPointer> dataMap;
    engine->setChildrenDataMap(&dataMap);
}

TEST_F(TestGroupingEngine, SetVisibleChildren)
{
    QList<QUrl> children;
    engine->setVisibleChildren(&children);
}

TEST_F(TestGroupingEngine, SetUpdateMode)
{
    engine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    EXPECT_EQ(engine->currentUpdateMode(), GroupingEngine::UpdateMode::kInsert);
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kUpdate);
    EXPECT_EQ(engine->currentUpdateMode(), GroupingEngine::UpdateMode::kUpdate);
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    EXPECT_EQ(engine->currentUpdateMode(), GroupingEngine::UpdateMode::kRemove);
}

TEST_F(TestGroupingEngine, SetUpdateChildren)
{
    QList<QUrl> children;
    children.append(QUrl::fromLocalFile("/test/file1.txt"));
    engine->setUpdateChildren(children);
}

TEST_F(TestGroupingEngine, SetUpdateChildrenRange)
{
    engine->setUpdateChildrenRange(5, 10);
    auto range = engine->currentUpdateChildrenRange();
    EXPECT_EQ(range.first, 5);
    EXPECT_EQ(range.second, 10);
}

TEST_F(TestGroupingEngine, SetCancellationCheckCallback)
{
    bool callbackCalled = false;
    engine->setCancellationCheckCallback([&callbackCalled]() -> bool {
        callbackCalled = true;
        return false;
    });
}

TEST_F(TestGroupingEngine, GroupFiles_InvalidStrategy)
{
    QList<FileItemDataPointer> files;
    files.append(testFileData);
    
    auto result = engine->groupFiles(files, nullptr);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.isEmpty());
}

TEST_F(TestGroupingEngine, GroupFiles_ValidStrategy)
{
    QList<FileItemDataPointer> files;
    files.append(testFileData);
    files.append(testFileData2);
    
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    auto result = engine->groupFiles(files, &strategy);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.groups.size(), 1);  // All files go to the same group
    EXPECT_EQ(result.groups.first().files.size(), 2);
}

TEST_F(TestGroupingEngine, GroupFiles_WithCancellation)
{
    QList<FileItemDataPointer> files;
    files.append(testFileData);
    
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    // Set cancellation callback to always return true (cancel)
    engine->setCancellationCheckCallback([]() -> bool {
        return true;  // Cancel
    });
    
    auto result = engine->groupFiles(files, &strategy);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "Operation canceled");
}

TEST_F(TestGroupingEngine, GenerateModelData_FailedResult)
{
    GroupingEngine::GroupingResult failedResult;
    failedResult.success = false;
    
    QHash<QString, bool> expansionStates;
    auto modelData = engine->generateModelData(failedResult, expansionStates);
    
    EXPECT_TRUE(modelData.groups.isEmpty());
}

TEST_F(TestGroupingEngine, GenerateModelData_SuccessResult)
{
    GroupingEngine::GroupingResult successResult;
    successResult.success = true;
    
    FileGroupData group;
    group.groupKey = "test_group";
    group.displayName = "Test Group";
    group.files.append(testFileData);
    group.fileCount = 1;
    group.isExpanded = true;
    group.displayOrder = 1;
    
    successResult.groups.append(group);
    
    QHash<QString, bool> expansionStates;
    expansionStates["test_group"] = true;
    
    auto modelData = engine->generateModelData(successResult, expansionStates);
    
    EXPECT_EQ(modelData.groups.size(), 1);
    EXPECT_EQ(modelData.groups.first().groupKey, "test_group");
    EXPECT_TRUE(modelData.groups.first().isExpanded);
}

TEST_F(TestGroupingEngine, FindPrecedingAnchor_ValidRange)
{
    QList<QUrl> container;
    container.append(QUrl::fromLocalFile("/test/file1.txt"));
    container.append(QUrl::fromLocalFile("/test/file2.txt"));
    container.append(QUrl::fromLocalFile("/test/file3.txt"));
    
    QPair<int, int> range(1, 2);  // Start at index 1, length 2
    
    auto anchor = engine->findPrecedingAnchor(container, range);
    ASSERT_TRUE(anchor.has_value());
    EXPECT_EQ(anchor.value(), QUrl::fromLocalFile("/test/file1.txt"));
}

TEST_F(TestGroupingEngine, FindPrecedingAnchor_StartOfList)
{
    QList<QUrl> container;
    container.append(QUrl::fromLocalFile("/test/file1.txt"));
    container.append(QUrl::fromLocalFile("/test/file2.txt"));
    
    QPair<int, int> range(0, 1);  // Start at index 0, length 1
    
    auto anchor = engine->findPrecedingAnchor(container, range);
    ASSERT_TRUE(anchor.has_value());
    EXPECT_TRUE(anchor.value().isEmpty());  // Empty URL for start of list
}

TEST_F(TestGroupingEngine, FindPrecedingAnchor_InvalidRange)
{
    QList<QUrl> container;
    container.append(QUrl::fromLocalFile("/test/file1.txt"));
    
    QPair<int, int> range(1, 2);  // Invalid range (exceeds container size)
    
    auto anchor = engine->findPrecedingAnchor(container, range);
    EXPECT_FALSE(anchor.has_value());
}

TEST_F(TestGroupingEngine, InsertFilesToModelData_InvalidMode)
{
    GroupedModelData oldData;
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    // Set wrong mode
    engine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    
    auto result = engine->insertFilesToModelData(QUrl(), oldData, &strategy);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, InsertFilesToModelData_EmptyChildren)
{
    GroupedModelData oldData;
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    // Don't set any children to update
    
    auto result = engine->insertFilesToModelData(QUrl(), oldData, &strategy);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, InsertFilesToModelData_NoStrategy)
{
    GroupedModelData oldData;
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    engine->setUpdateChildren(visibleChildren);
    
    auto result = engine->insertFilesToModelData(QUrl(), oldData, nullptr);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, InsertFilesToModelData_ValidData)
{
    GroupedModelData oldData;
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    engine->setUpdateChildren(visibleChildren);
    
    auto result = engine->insertFilesToModelData(QUrl(), oldData, &strategy);
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.newData.getItemCount(), 0);
}

TEST_F(TestGroupingEngine, UpdateFilesToModelData_InvalidMode)
{
    GroupedModelData oldData;
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    // Set wrong mode
    engine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    
    auto result = engine->updateFilesToModelData(QUrl(), oldData, &strategy);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, UpdateFilesToModelData_ValidData)
{
    GroupedModelData oldData;
    MockGroupStrategy strategy("Name");  // Use built-in strategy name
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kUpdate);
    engine->setUpdateChildren(visibleChildren);
    
    auto result = engine->updateFilesToModelData(QUrl(), oldData, &strategy);
    EXPECT_TRUE(result.success);
}

TEST_F(TestGroupingEngine, RemoveFilesFromModelData_InvalidMode)
{
    GroupedModelData oldData;
    
    // Set wrong mode
    engine->setUpdateMode(GroupingEngine::UpdateMode::kInsert);
    
    auto result = engine->removeFilesFromModelData(oldData);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, RemoveFilesFromModelData_EmptyChildren)
{
    GroupedModelData oldData;
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    // Don't set any children to update
    
    auto result = engine->removeFilesFromModelData(oldData);
    EXPECT_FALSE(result.success);
}

TEST_F(TestGroupingEngine, RemoveFilesFromModelData_ValidData)
{
    GroupedModelData oldData;
    
    // Add some files to oldData first
    FileGroupData group;
    group.groupKey = "test_group";
    group.files.append(testFileData);
    oldData.addGroup(group);
    oldData.rebuildFlattenedItems();
    
    engine->setUpdateMode(GroupingEngine::UpdateMode::kRemove);
    engine->setUpdateChildren(visibleChildren);
    
    auto result = engine->removeFilesFromModelData(oldData);
    EXPECT_TRUE(result.success);
}

TEST_F(TestGroupingEngine, GetFileInfoFromFileItem_ValidItem)
{
    auto fileInfo = engine->getFileInfoFromFileItem(testFileData);
    // The result depends on mock, just ensure it doesn't crash
}

TEST_F(TestGroupingEngine, GetFileInfoFromFileItem_NullItem)
{
    auto fileInfo = engine->getFileInfoFromFileItem(FileItemDataPointer());
    EXPECT_EQ(fileInfo, nullptr);
}

TEST_F(TestGroupingEngine, FindExpandedFiles_NoTreeChildren)
{
    auto expandedFiles = engine->findExpandedFiles(testFileData);
    EXPECT_TRUE(expandedFiles.isEmpty());
}

TEST_F(TestGroupingEngine, FindExpandedFiles_NotExpanded)
{
    QHash<QUrl, QList<QUrl>> treeChildren;
    engine->setVisibleTreeChildren(&treeChildren);
    
    auto expandedFiles = engine->findExpandedFiles(testFileData);
    EXPECT_TRUE(expandedFiles.isEmpty());
}

TEST_F(TestGroupingEngine, FindExpandedFiles_Expanded)
{
    // Set file as expanded
    testFileData->setExpanded(true);
    
    QHash<QUrl, QList<QUrl>> treeChildren;
    QList<QUrl> children;
    children.append(QUrl::fromLocalFile("/test/child1.txt"));
    treeChildren.insert(QUrl::fromLocalFile("/test/file1.txt"), children);
    
    // Add child data to map
    QUrl childUrl = QUrl::fromLocalFile("/test/child1.txt");
    FileItemDataPointer childData = FileItemDataPointer::create(childUrl);
    childrenDataMap.insert(QUrl::fromLocalFile("/test/child1.txt"), childData);
    
    engine->setVisibleTreeChildren(&treeChildren);
    
    auto expandedFiles = engine->findExpandedFiles(testFileData);
    EXPECT_EQ(expandedFiles.size(), 1);
}

TEST_F(TestGroupingEngine, FindTopLevelAncestorOf_NoTreeChildren)
{
    auto ancestor = engine->findTopLevelAncestorOf(QUrl::fromLocalFile("/test/file1.txt"));
    EXPECT_TRUE(ancestor.isEmpty());
}

TEST_F(TestGroupingEngine, FindTopLevelAncestorOf_RootUrl)
{
    auto ancestor = engine->findTopLevelAncestorOf(testUrl);
    EXPECT_TRUE(ancestor.isEmpty());
}

TEST_F(TestGroupingEngine, FindTopLevelAncestorOf_TopLevelChild)
{
    QHash<QUrl, QList<QUrl>> treeChildren;
    QList<QUrl> rootChildren;
    rootChildren.append(QUrl::fromLocalFile("/test/file1.txt"));
    treeChildren.insert(testUrl, rootChildren);
    
    engine->setVisibleTreeChildren(&treeChildren);
    
    auto ancestor = engine->findTopLevelAncestorOf(QUrl::fromLocalFile("/test/file1.txt"));
    EXPECT_EQ(ancestor, QUrl::fromLocalFile("/test/file1.txt"));
}

TEST_F(TestGroupingEngine, ReorderGroups_EmptyData)
{
    GroupedModelData modelData;
    engine->reorderGroups(&modelData);
    // Should not crash
}

TEST_F(TestGroupingEngine, ReorderGroups_ValidData)
{
    GroupedModelData modelData;
    
    FileGroupData group1;
    group1.groupKey = "group1";
    group1.displayOrder = 2;
    
    FileGroupData group2;
    group2.groupKey = "group2";
    group2.displayOrder = 1;
    
    modelData.addGroup(group1);
    modelData.addGroup(group2);
    
    engine->setGroupOrder(Qt::AscendingOrder);
    engine->reorderGroups(&modelData);
    
    // Groups should be reordered by displayOrder
    EXPECT_EQ(modelData.groups.first().groupKey, "group2");
    EXPECT_EQ(modelData.groups.last().groupKey, "group1");
}

DPWORKSPACE_END_NAMESPACE
