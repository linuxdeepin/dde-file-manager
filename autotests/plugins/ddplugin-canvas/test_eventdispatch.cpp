// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/desktop/ddplugin-canvas/broker/canvasgridbroker.h"
#include "plugins/desktop/ddplugin-canvas/broker/canvasviewbroker.h"
#include "plugins/desktop/ddplugin-canvas/broker/canvasmodelbroker.h"
#include "plugins/desktop/ddplugin-canvas/broker/canvasmanagerbroker.h"
#include "plugins/desktop/ddplugin-canvas/broker/fileinfomodelbroker.h"
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"
#include "plugins/desktop/ddplugin-canvas/private/canvasmanager_p.h"
#include "plugins/desktop/ddplugin-canvas/grid/canvasgrid.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/recentproxy/canvasrecentproxy.h"

#include <dfm-framework/dpf.h>
#include <dfm-base/dfm_event_defines.h>

#include <dlfcn.h>

#include <gtest/gtest.h>
#include <QHash>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QVariant>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

namespace {
// Maps "space::topic" to a stable id inside the custom event type range so
// dpfSlotChannel / dpfSignalDispatcher resolve topics in the test process.
int canvasTestEventConverter(const QString &space, const QString &topic)
{
    static QHash<QString, int> mapping;
    const QString key = space + "::" + topic;
    const auto it = mapping.constFind(key);
    if (it != mapping.constEnd())
        return it.value();
    const int id = static_cast<int>(EventTypeScope::kCustomBase) + mapping.size();
    mapping.insert(key, id);
    return id;
}

void installCanvasTestConverter()
{
    // Construct the framework Event singleton first: its constructor registers
    // the production converter exactly once (std::call_once) and would clobber
    // a direct assignment made before the first dpfEvent evaluation.
    dpf::Event::instance();
    // C++17 inline variables get separate instances in the test executable and
    // in libdfm6-framework.so (the two are not unified at dynamic link). Assign
    // every reachable instance so dpfSlotChannel/dpfSignalDispatcher resolve
    // topics no matter which TU the inline call site was instantiated in.
    EventConverter::convertFunc = &canvasTestEventConverter;
    if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                dlsym(RTLD_DEFAULT, "_ZN3dpf14EventConverter11convertFuncE"))) {
        *func = &canvasTestEventConverter;
    }
    if (void *framework = dlopen("libdfm6-framework.so.1", RTLD_LAZY | RTLD_NOLOAD)) {
        if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                    dlsym(framework, "_ZN3dpf14EventConverter11convertFuncE"))) {
            *func = &canvasTestEventConverter;
        }
    }
}
}   // namespace

static constexpr auto kCanvasSpace = "ddplugin_canvas";

class CanvasGridBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        grid = new CanvasGrid(nullptr);
        broker = new CanvasGridBroker(grid);
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        delete grid;
        stub.clear();
    }

    CanvasGrid *grid = nullptr;
    CanvasGridBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridBrokerEventTest, SlotChannel_PushGridItems_InvokesBrokerMethod)
{
    QStringList canned { "file:///home/a", "file:///home/b" };
    int capturedIndex = -1;
    stub.set_lamda(ADDR(CanvasGridBroker, items),
                   [&canned, &capturedIndex](CanvasGridBroker *, int index) -> QStringList {
                       __DBG_STUB_INVOKE__
                       capturedIndex = index;
                       return canned;
                   });

    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasGrid_Items", 2);
    EXPECT_EQ(capturedIndex, 2);
    EXPECT_EQ(ret.value<QStringList>(), canned);
}

TEST_F(CanvasGridBrokerEventTest, SlotChannel_PushGridItem_InvokesBrokerMethod)
{
    QString canned("file:///home/a");
    int capturedIndex = -1;
    QPoint capturedPos(-1, -1);
    stub.set_lamda(ADDR(CanvasGridBroker, item),
                   [&canned, &capturedIndex, &capturedPos](CanvasGridBroker *, int index, const QPoint &pos) -> QString {
                       capturedIndex = index;
                       capturedPos = pos;
                       return canned;
                   });

    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasGrid_Item", 1, QPoint(3, 4));
    EXPECT_EQ(capturedIndex, 1);
    EXPECT_EQ(capturedPos, QPoint(3, 4));
    EXPECT_EQ(ret.toString(), canned);
}

TEST_F(CanvasGridBrokerEventTest, SlotChannel_PushGridPoint_InvokesBrokerMethod)
{
    const QString item("file:///home/a");
    int capturedScreen = -2;
    stub.set_lamda(ADDR(CanvasGridBroker, point),
                   [&capturedScreen](CanvasGridBroker *, const QString &, QPoint *pos) -> int {
                       if (pos)
                           *pos = QPoint(7, 8);
                       capturedScreen = 1;
                       return 1;
                   });

    QPoint pos;
    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasGrid_Point",
                                        item, QVariant::fromValue(&pos));
    EXPECT_EQ(capturedScreen, 1);
    EXPECT_EQ(pos, QPoint(7, 8));
    EXPECT_EQ(ret.toInt(), 1);
}

TEST_F(CanvasGridBrokerEventTest, SlotChannel_PushGridTryAppendAfter_InvokesBrokerMethod)
{
    bool invoked = false;
    QStringList capturedItems;
    stub.set_lamda(ADDR(CanvasGridBroker, tryAppendAfter),
                   [&invoked, &capturedItems](CanvasGridBroker *, const QStringList &items, int, const QPoint &) {
                       invoked = true;
                       capturedItems = items;
                   });

    QStringList input { "x", "y" };
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasGrid_TryAppendAfter", input, 0, QPoint(0, 0));
    EXPECT_TRUE(invoked);
    EXPECT_EQ(capturedItems, input);
}

class CanvasViewBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        manager = new CanvasManager();
        broker = new CanvasViewBroker(manager);
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        delete manager;
        stub.clear();
    }

    CanvasManager *manager = nullptr;
    CanvasViewBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushVisualRect_InvokesBrokerMethod)
{
    QRect canned(1, 2, 30, 30);
    QUrl capturedUrl;
    stub.set_lamda(ADDR(CanvasViewBroker, visualRect),
                   [&canned, &capturedUrl](CanvasViewBroker *, int, const QUrl &url) -> QRect {
                       capturedUrl = url;
                       return canned;
                   });

    QUrl url("file:///home/a.txt");
    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_VisualRect", 1, url);
    EXPECT_EQ(capturedUrl, url);
    EXPECT_EQ(ret.value<QRect>(), canned);
}

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushGridPosAndGridVisualRect_InvokesBrokerMethods)
{
    QPoint cannedPos(11, 12);
    stub.set_lamda(ADDR(CanvasViewBroker, gridPos),
                   [&cannedPos](CanvasViewBroker *, int, const QPoint &) -> QPoint {
                       return cannedPos;
                   });
    QRect cannedRect(5, 6, 80, 90);
    stub.set_lamda(ADDR(CanvasViewBroker, gridVisualRect),
                   [&cannedRect](CanvasViewBroker *, int, const QPoint &) -> QRect {
                       return cannedRect;
                   });

    QVariant pos = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_GridPos", 1, QPoint(50, 60));
    QVariant rect = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_GridVisualRect", 1, QPoint(0, 1));
    EXPECT_EQ(pos.value<QPoint>(), cannedPos);
    EXPECT_EQ(rect.value<QRect>(), cannedRect);
}

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushGridSize_InvokesBrokerMethod)
{
    QSize canned(10, 8);
    stub.set_lamda(ADDR(CanvasViewBroker, gridSize),
                   [&canned](CanvasViewBroker *, int) -> QSize {
                       return canned;
                   });

    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_GridSize", 1);
    EXPECT_EQ(ret.value<QSize>(), canned);
}

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushRefreshAndUpdate_InvokesBrokerMethods)
{
    int refreshIdx = -1;
    int updateIdx = -1;
    stub.set_lamda(ADDR(CanvasViewBroker, refresh),
                   [&refreshIdx](CanvasViewBroker *, int idx) {
                       refreshIdx = idx;
                   });
    stub.set_lamda(ADDR(CanvasViewBroker, update),
                   [&updateIdx](CanvasViewBroker *, int idx) {
                       updateIdx = idx;
                   });

    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_Refresh", 3);
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_Update", -1);
    EXPECT_EQ(refreshIdx, 3);
    EXPECT_EQ(updateIdx, -1);
}

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushSelectAndSelectedUrls_InvokesBrokerMethods)
{
    QList<QUrl> capturedSelect;
    stub.set_lamda(ADDR(CanvasViewBroker, select),
                   [&capturedSelect](CanvasViewBroker *, const QList<QUrl> &urls) {
                       capturedSelect = urls;
                   });
    QList<QUrl> cannedSelected { QUrl("file:///home/s1") };
    stub.set_lamda(ADDR(CanvasViewBroker, selectedUrls),
                   [&cannedSelected](CanvasViewBroker *, int) -> QList<QUrl> {
                       return cannedSelected;
                   });

    QList<QUrl> input { QUrl("file:///home/a"), QUrl("file:///home/b") };
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_Select", input);
    QVariant ret = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasView_SelectedUrls", 0);
    EXPECT_EQ(capturedSelect, input);
    EXPECT_EQ(ret.value<QList<QUrl>>(), cannedSelected);
}

TEST_F(CanvasViewBrokerEventTest, SlotChannel_PushFileOperatorAndIconRect_InvokesBrokerMethods)
{
    QObject cannedOperator;
    stub.set_lamda(ADDR(CanvasViewBroker, fileOperator),
                   [&cannedOperator](CanvasViewBroker *) -> QObject * {
                       return &cannedOperator;
                   });
    QRect cannedIcon(2, 3, 40, 40);
    stub.set_lamda(ADDR(CanvasViewBroker, iconRect),
                   [&cannedIcon](CanvasViewBroker *, int, QRect) -> QRect {
                       return cannedIcon;
                   });

    QVariant op = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasViewPrivate_FileOperator");
    QVariant icon = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasItemDelegate_IconRect", 1, QRect(0, 0, 100, 100));
    EXPECT_EQ(op.value<QObject *>(), &cannedOperator);
    EXPECT_EQ(icon.value<QRect>(), cannedIcon);
}

class CanvasModelBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        model = new CanvasProxyModel();
        broker = new CanvasModelBroker(model);
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        delete model;
        stub.clear();
    }

    CanvasProxyModel *model = nullptr;
    CanvasModelBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelBrokerEventTest, SlotChannel_PushUrlQueries_InvokesBrokerMethods)
{
    QUrl cannedRoot("file:///home");
    stub.set_lamda(ADDR(CanvasModelBroker, rootUrl),
                   [&cannedRoot](CanvasModelBroker *) -> QUrl {
                       return cannedRoot;
                   });
    QModelIndex cannedIndex(0, 0, nullptr, model);
    stub.set_lamda(ADDR(CanvasModelBroker, urlIndex),
                   [&cannedIndex](CanvasModelBroker *, const QUrl &) -> QModelIndex {
                       return cannedIndex;
                   });
    stub.set_lamda(ADDR(CanvasModelBroker, index),
                   [&cannedIndex](CanvasModelBroker *, int) -> QModelIndex {
                       return cannedIndex;
                   });
    QUrl cannedUrl("file:///home/a");
    stub.set_lamda(ADDR(CanvasModelBroker, fileUrl),
                   [&cannedUrl](CanvasModelBroker *, const QModelIndex &) -> QUrl {
                       return cannedUrl;
                   });
    QList<QUrl> cannedFiles { QUrl("file:///home/f1") };
    stub.set_lamda(ADDR(CanvasModelBroker, files),
                   [&cannedFiles](CanvasModelBroker *) -> QList<QUrl> {
                       return cannedFiles;
                   });

    QUrl input("file:///home/in");
    QVariant root = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_RootUrl");
    QVariant urlIndex = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_UrlIndex", input);
    QVariant row = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Index", 0);
    QVariant fileUrl = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_FileUrl", QVariant::fromValue(cannedIndex));
    QVariant files = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Files");
    EXPECT_EQ(root.value<QUrl>(), cannedRoot);
    EXPECT_EQ(urlIndex.value<QModelIndex>(), cannedIndex);
    EXPECT_EQ(row.value<QModelIndex>(), cannedIndex);
    EXPECT_EQ(fileUrl.value<QUrl>(), cannedUrl);
    EXPECT_EQ(files.value<QList<QUrl>>(), cannedFiles);
}

TEST_F(CanvasModelBrokerEventTest, SlotChannel_PushFlagsAndSortSettings_InvokesBrokerMethods)
{
    bool cannedHidden = true;
    stub.set_lamda(ADDR(CanvasModelBroker, showHiddenFiles),
                   [&cannedHidden](CanvasModelBroker *) -> bool {
                       return cannedHidden;
                   });
    int capturedShow = -1;
    stub.set_lamda(ADDR(CanvasModelBroker, setShowHiddenFiles),
                   [&capturedShow](CanvasModelBroker *, bool show) {
                       capturedShow = static_cast<int>(show);
                   });
    int cannedOrder = Qt::DescendingOrder;
    stub.set_lamda(ADDR(CanvasModelBroker, sortOrder),
                   [&cannedOrder](CanvasModelBroker *) -> int {
                       return cannedOrder;
                   });
    int capturedOrder = -1;
    stub.set_lamda(ADDR(CanvasModelBroker, setSortOrder),
                   [&capturedOrder](CanvasModelBroker *, int order) {
                       capturedOrder = order;
                   });
    int cannedRole = 257;
    stub.set_lamda(ADDR(CanvasModelBroker, sortRole),
                   [&cannedRole](CanvasModelBroker *) -> int {
                       return cannedRole;
                   });
    int capturedRole = -1;
    int capturedRoleOrder = -1;
    stub.set_lamda(ADDR(CanvasModelBroker, setSortRole),
                   [&capturedRole, &capturedRoleOrder](CanvasModelBroker *, int role, int order) {
                       capturedRole = role;
                       capturedRoleOrder = order;
                   });
    int cannedRowCount = 42;
    stub.set_lamda(ADDR(CanvasModelBroker, rowCount),
                   [&cannedRowCount](CanvasModelBroker *) -> int {
                       return cannedRowCount;
                   });

    QVariant hidden = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_ShowHiddenFiles");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_SetShowHiddenFiles", true);
    QVariant order = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_SortOrder");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_SetSortOrder", 1);
    QVariant role = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_SortRole");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_SetSortRole", 257, 1);
    QVariant rows = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_RowCount");
    EXPECT_TRUE(hidden.toBool());
    EXPECT_EQ(capturedShow, 1);
    EXPECT_EQ(order.toInt(), Qt::DescendingOrder);
    EXPECT_EQ(capturedOrder, 1);
    EXPECT_EQ(role.toInt(), 257);
    EXPECT_EQ(capturedRole, 257);
    EXPECT_EQ(capturedRoleOrder, 1);
    EXPECT_EQ(rows.toInt(), 42);
}

TEST_F(CanvasModelBrokerEventTest, SlotChannel_PushDataSortRefreshFetchTake_InvokesBrokerMethods)
{
    QVariant cannedData("icon-name");
    QUrl capturedDataUrl;
    int capturedDataRole = -1;
    stub.set_lamda(ADDR(CanvasModelBroker, data),
                   [&cannedData, &capturedDataUrl, &capturedDataRole](CanvasModelBroker *, const QUrl &url, int role) -> QVariant {
                       capturedDataUrl = url;
                       capturedDataRole = role;
                       return cannedData;
                   });
    bool sortInvoked = false;
    stub.set_lamda(ADDR(CanvasModelBroker, sort),
                   [&sortInvoked](CanvasModelBroker *) {
                       sortInvoked = true;
                   });
    int capturedGlobal = -1;
    int capturedMs = -1;
    int capturedUpdate = -1;
    stub.set_lamda(ADDR(CanvasModelBroker, refresh),
                   [&capturedGlobal, &capturedMs, &capturedUpdate](CanvasModelBroker *, bool global, int ms, bool updateFile) {
                       capturedGlobal = static_cast<int>(global);
                       capturedMs = ms;
                       capturedUpdate = static_cast<int>(updateFile);
                   });
    bool cannedFetch = true;
    stub.set_lamda(ADDR(CanvasModelBroker, fetch),
                   [&cannedFetch](CanvasModelBroker *, const QUrl &) -> bool {
                       return cannedFetch;
                   });
    bool cannedTake = false;
    stub.set_lamda(ADDR(CanvasModelBroker, take),
                   [&cannedTake](CanvasModelBroker *, const QUrl &) -> bool {
                       return cannedTake;
                   });

    QUrl input("file:///home/data.txt");
    QVariant data = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Data", input, 257);
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Sort");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Refresh", false, 200, true);
    QVariant fetch = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Fetch", input);
    QVariant take = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasModel_Take", input);
    EXPECT_EQ(capturedDataUrl, input);
    EXPECT_EQ(capturedDataRole, 257);
    EXPECT_EQ(data.value<QVariant>().toString(), QString("icon-name"));   // QVariant return is wrapped
    EXPECT_TRUE(sortInvoked);
    EXPECT_EQ(capturedGlobal, 0);
    EXPECT_EQ(capturedMs, 200);
    EXPECT_EQ(capturedUpdate, 1);
    EXPECT_TRUE(fetch.toBool());
    EXPECT_FALSE(take.toBool());
}

class CanvasManagerBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        manager = new CanvasManager();
        broker = new CanvasManagerBroker(manager);
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        delete manager;
        stub.clear();
    }

    CanvasManager *manager = nullptr;
    CanvasManagerBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerBrokerEventTest, SlotChannel_PushVoidAndValueQueries_InvokesBrokerMethods)
{
    bool updateInvoked = false;
    stub.set_lamda(ADDR(CanvasManagerBroker, update),
                   [&updateInvoked](CanvasManagerBroker *) {
                       updateInvoked = true;
                   });
    QStandardItemModel cannedModel;
    stub.set_lamda(ADDR(CanvasManagerBroker, fileInfoModel),
                   [&cannedModel](CanvasManagerBroker *) -> QAbstractItemModel * {
                       return &cannedModel;
                   });
    int cannedLevel = 3;
    stub.set_lamda(ADDR(CanvasManagerBroker, iconLevel),
                   [&cannedLevel](CanvasManagerBroker *) -> int {
                       return cannedLevel;
                   });
    int capturedLevel = -1;
    stub.set_lamda(ADDR(CanvasManagerBroker, setIconLevel),
                   [&capturedLevel](CanvasManagerBroker *, int lv) {
                       capturedLevel = lv;
                   });
    bool cannedArrange = true;
    stub.set_lamda(ADDR(CanvasManagerBroker, autoArrange),
                   [&cannedArrange](CanvasManagerBroker *) -> bool {
                       return cannedArrange;
                   });
    int capturedArrange = -1;
    stub.set_lamda(ADDR(CanvasManagerBroker, setAutoArrange),
                   [&capturedArrange](CanvasManagerBroker *, bool on) {
                       capturedArrange = static_cast<int>(on);
                   });
    QUrl capturedEditUrl;
    stub.set_lamda(ADDR(CanvasManagerBroker, edit),
                   [&capturedEditUrl](CanvasManagerBroker *, const QUrl &url) {
                       capturedEditUrl = url;
                   });

    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_Update");
    QVariant model = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_FileInfoModel");
    QVariant level = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_IconLevel");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_SetIconLevel", 4);
    QVariant arrange = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_AutoArrange");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_SetAutoArrange", true);
    QUrl editUrl("file:///home/edit.txt");
    dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_Edit", editUrl);
    EXPECT_TRUE(updateInvoked);
    EXPECT_EQ(model.value<QAbstractItemModel *>(), &cannedModel);
    EXPECT_EQ(level.toInt(), 3);
    EXPECT_EQ(capturedLevel, 4);
    EXPECT_TRUE(arrange.toBool());
    EXPECT_EQ(capturedArrange, 1);
    EXPECT_EQ(capturedEditUrl, editUrl);
}

TEST_F(CanvasManagerBrokerEventTest, SlotChannel_PushViewAndSelectionModel_InvokesBrokerMethods)
{
    QAbstractItemView *cannedView = reinterpret_cast<QAbstractItemView *>(0x10);
    stub.set_lamda(ADDR(CanvasManagerBroker, view),
                   [cannedView](CanvasManagerBroker *, int) -> QAbstractItemView * {
                       return cannedView;
                   });
    QItemSelectionModel *cannedSelection = reinterpret_cast<QItemSelectionModel *>(0x20);
    stub.set_lamda(ADDR(CanvasManagerBroker, selectionModel),
                   [cannedSelection](CanvasManagerBroker *) -> QItemSelectionModel * {
                       return cannedSelection;
                   });

    QVariant view = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_View", 1);
    QVariant selection = dpfSlotChannel->push(kCanvasSpace, "slot_CanvasManager_SelectionModel");
    EXPECT_EQ(view.value<QAbstractItemView *>(), cannedView);
    EXPECT_EQ(selection.value<QItemSelectionModel *>(), cannedSelection);
}

class FileInfoModelBrokerEventTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        model = new FileInfoModel();
        broker = new FileInfoModelBroker(model);
        ASSERT_TRUE(broker->init());
    }

    void TearDown() override
    {
        delete broker;
        delete model;
        stub.clear();
    }

    FileInfoModel *model = nullptr;
    FileInfoModelBroker *broker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoModelBrokerEventTest, SlotChannel_PushUrlAndIndexQueries_InvokesBrokerMethods)
{
    QUrl cannedRoot("file:///home");
    stub.set_lamda(ADDR(FileInfoModelBroker, rootUrl),
                   [&cannedRoot](FileInfoModelBroker *) -> QUrl {
                       return cannedRoot;
                   });
    QModelIndex cannedRootIndex(0, 0, nullptr, model);
    stub.set_lamda(ADDR(FileInfoModelBroker, rootIndex),
                   [&cannedRootIndex](FileInfoModelBroker *) -> QModelIndex {
                       return cannedRootIndex;
                   });
    QModelIndex cannedIndex(1, 0, nullptr, model);
    stub.set_lamda(ADDR(FileInfoModelBroker, urlIndex),
                   [&cannedIndex](FileInfoModelBroker *, const QUrl &) -> QModelIndex {
                       return cannedIndex;
                   });
    QUrl cannedUrl("file:///home/index-url");
    stub.set_lamda(ADDR(FileInfoModelBroker, indexUrl),
                   [&cannedUrl](FileInfoModelBroker *, const QModelIndex &) -> QUrl {
                       return cannedUrl;
                   });
    QList<QUrl> cannedFiles { QUrl("file:///home/x") };
    stub.set_lamda(ADDR(FileInfoModelBroker, files),
                   [&cannedFiles](FileInfoModelBroker *) -> QList<QUrl> {
                       return cannedFiles;
                   });
    int cannedState = 2;
    stub.set_lamda(ADDR(FileInfoModelBroker, modelState),
                   [&cannedState](FileInfoModelBroker *) -> int {
                       return cannedState;
                   });

    QUrl input("file:///home/q");
    QVariant root = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_RootUrl");
    QVariant rootIndex = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_RootIndex");
    QVariant urlIndex = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_UrlIndex", input);
    QVariant indexUrl = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_IndexUrl", QVariant::fromValue(cannedIndex));
    QVariant files = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_Files");
    QVariant state = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_ModelState");
    EXPECT_EQ(root.value<QUrl>(), cannedRoot);
    EXPECT_EQ(rootIndex.value<QModelIndex>(), cannedRootIndex);
    EXPECT_EQ(urlIndex.value<QModelIndex>(), cannedIndex);
    EXPECT_EQ(indexUrl.value<QUrl>(), cannedUrl);
    EXPECT_EQ(files.value<QList<QUrl>>(), cannedFiles);
    EXPECT_EQ(state.toInt(), 2);
}

TEST_F(FileInfoModelBrokerEventTest, SlotChannel_PushFileInfoRefreshUpdateFile_InvokesBrokerMethods)
{
    FileInfoPointer cannedInfo;
    stub.set_lamda(ADDR(FileInfoModelBroker, fileInfo),
                   [&cannedInfo](FileInfoModelBroker *, const QModelIndex &) -> FileInfoPointer {
                       return cannedInfo;
                   });
    QModelIndex capturedRefresh;
    stub.set_lamda(ADDR(FileInfoModelBroker, refresh),
                   [&capturedRefresh](FileInfoModelBroker *, const QModelIndex &parent) {
                       capturedRefresh = parent;
                   });
    QUrl capturedUpdateUrl;
    stub.set_lamda(ADDR(FileInfoModelBroker, updateFile),
                   [&capturedUpdateUrl](FileInfoModelBroker *, const QUrl &url) {
                       capturedUpdateUrl = url;
                   });

    QModelIndex input(3, 0, nullptr, model);
    QVariant info = dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_FileInfo", QVariant::fromValue(input));
    dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_Refresh", QVariant::fromValue(input));
    QUrl updateUrl("file:///home/update.txt");
    dpfSlotChannel->push(kCanvasSpace, "slot_FileInfoModel_UpdateFile", updateUrl);
    EXPECT_EQ(info.value<FileInfoPointer>(), cannedInfo);
    EXPECT_EQ(capturedRefresh, input);
    EXPECT_EQ(capturedUpdateUrl, updateUrl);
}

class CanvasManagerSignalTest : public testing::Test
{
protected:
    void SetUp() override
    {
        installCanvasTestConverter();
        manager = new CanvasManager();
    }

    void TearDown() override
    {
        // Remove the handlers registered by the tests so later suites never
        // dispatch into stale objects.
        dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_WindowAboutToBeBuilded", manager, &CanvasManager::onDetachWindows);
        dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_WindowBuilded", manager, &CanvasManager::onCanvasBuild);
        dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_GeometryChanged", manager, &CanvasManager::onGeometryChanged);
        dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_AvailableGeometryChanged", manager, &CanvasManager::onGeometryChanged);
        delete manager;
        stub.clear();
    }

    CanvasManager *manager = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerSignalTest, Dispatcher_PublishWindowAboutToBeBuilded_InvokesHandler)
{
    bool invoked = false;
    stub.set_lamda(ADDR(CanvasManager, onDetachWindows),
                   [&invoked](CanvasManager *) {
                       invoked = true;
                   });

    EXPECT_TRUE(dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_WindowAboutToBeBuilded", manager, &CanvasManager::onDetachWindows));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_WindowAboutToBeBuilded"));
    EXPECT_TRUE(invoked);
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_WindowAboutToBeBuilded", manager, &CanvasManager::onDetachWindows));
}

TEST_F(CanvasManagerSignalTest, Dispatcher_PublishWindowBuilded_InvokesHandler)
{
    bool invoked = false;
    stub.set_lamda(ADDR(CanvasManager, onCanvasBuild),
                   [&invoked](CanvasManager *) {
                       invoked = true;
                   });

    EXPECT_TRUE(dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_WindowBuilded", manager, &CanvasManager::onCanvasBuild));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_WindowBuilded"));
    EXPECT_TRUE(invoked);
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("ddplugin_core", "signal_DesktopFrame_WindowBuilded", manager, &CanvasManager::onCanvasBuild));
}

TEST_F(CanvasManagerSignalTest, Dispatcher_PublishGeometryChanged_InvokesHandler)
{
    int invoked = 0;
    stub.set_lamda(ADDR(CanvasManager, onGeometryChanged),
                   [&invoked](CanvasManager *) {
                       ++invoked;
                   });

    EXPECT_TRUE(dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_GeometryChanged", manager, &CanvasManager::onGeometryChanged));
    EXPECT_TRUE(dpfSignalDispatcher->subscribe("ddplugin_core", "signal_DesktopFrame_AvailableGeometryChanged", manager, &CanvasManager::onGeometryChanged));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_GeometryChanged"));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_AvailableGeometryChanged"));
    EXPECT_EQ(invoked, 2);
}

TEST_F(CanvasManagerSignalTest, Dispatcher_PublishTrashStateChanged_InvokesHandler)
{
    bool invoked = false;
    stub.set_lamda(ADDR(CanvasManager, onTrashStateChanged),
                   [&invoked](CanvasManager *) {
                       invoked = true;
                   });

    EXPECT_TRUE(dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", manager, &CanvasManager::onTrashStateChanged));
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged"));
    EXPECT_TRUE(invoked);
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", manager, &CanvasManager::onTrashStateChanged));
}

TEST_F(CanvasManagerSignalTest, Dispatcher_RealInitSubscriptions_AllHandlersInvokedAndRemoved)
{
    // Arrange: run the production init() so the subscribe sites inside
    // canvasmanager.cpp register the real handlers, with heavy internals
    // stubbed away.
    int detachCount = 0;
    int buildCount = 0;
    int geometryCount = 0;
    int trashCount = 0;
    stub.set_lamda(ADDR(CanvasManager, onDetachWindows),
                   [&detachCount](CanvasManager *) { ++detachCount; });
    stub.set_lamda(ADDR(CanvasManager, onCanvasBuild),
                   [&buildCount](CanvasManager *) { ++buildCount; });
    stub.set_lamda(ADDR(CanvasManager, onGeometryChanged),
                   [&geometryCount](CanvasManager *) { ++geometryCount; });
    stub.set_lamda(ADDR(CanvasManager, onTrashStateChanged),
                   [&trashCount](CanvasManager *) { ++trashCount; });
    int recentCount = 0;
    stub.set_lamda(ADDR(CanvasRecentProxy, handleReloadRecentFiles),
                   [&recentCount](CanvasRecentProxy *, const QList<QUrl> &, bool, const QString &) { ++recentCount; });
    stub.set_lamda(ADDR(CanvasManagerPrivate, initModel), [](CanvasManagerPrivate *) { __DBG_STUB_INVOKE__ });
    stub.set_lamda(ADDR(CanvasManagerPrivate, initSetting), [](CanvasManagerPrivate *) { __DBG_STUB_INVOKE__ });

    manager->init();

    // Act: fire every subscribed signal through the real dispatcher.
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_WindowAboutToBeBuilded"));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_WindowBuilded"));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_GeometryChanged"));
    EXPECT_TRUE(dpfSignalDispatcher->publish("ddplugin_core", "signal_DesktopFrame_AvailableGeometryChanged"));
    EXPECT_TRUE(dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged"));
    QList<QUrl> urls { QUrl("file:///home/r") };
    EXPECT_TRUE(dpfSignalDispatcher->publish(static_cast<int>(GlobalEventType::kMoveToTrashResult), urls, true, QString()));
    EXPECT_TRUE(dpfSignalDispatcher->publish(static_cast<int>(GlobalEventType::kDeleteFilesResult), urls, true, QString()));

    // Assert
    EXPECT_EQ(detachCount, 1);
    EXPECT_EQ(buildCount, 1);
    EXPECT_EQ(geometryCount, 2);
    EXPECT_EQ(trashCount, 1);
    EXPECT_EQ(recentCount, 2);
}

TEST_F(CanvasManagerSignalTest, Dispatcher_PublishJobResults_InvokesRecentProxyHandler)
{
    CanvasRecentProxy proxy;
    QList<QUrl> capturedUrls;
    bool capturedOk = false;
    QString capturedMsg;
    stub.set_lamda(ADDR(CanvasRecentProxy, handleReloadRecentFiles),
                   [&capturedUrls, &capturedOk, &capturedMsg](CanvasRecentProxy *, const QList<QUrl> &urls, bool ok, const QString &errMsg) {
                       capturedUrls = urls;
                       capturedOk = ok;
                       capturedMsg = errMsg;
                   });

    const int movedToTrash = static_cast<int>(GlobalEventType::kMoveToTrashResult);
    const int deleteFiles = static_cast<int>(GlobalEventType::kDeleteFilesResult);
    QList<QUrl> input { QUrl("file:///home/recent.txt") };
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(movedToTrash, &proxy, &CanvasRecentProxy::handleReloadRecentFiles));
    EXPECT_TRUE(dpfSignalDispatcher->publish(movedToTrash, input, true, QString("done")));
    EXPECT_EQ(capturedUrls, input);
    EXPECT_TRUE(capturedOk);
    EXPECT_EQ(capturedMsg, QString("done"));

    capturedUrls.clear();
    EXPECT_TRUE(dpfSignalDispatcher->subscribe(deleteFiles, &proxy, &CanvasRecentProxy::handleReloadRecentFiles));
    EXPECT_TRUE(dpfSignalDispatcher->publish(deleteFiles, input, false, QString("")));
    EXPECT_EQ(capturedUrls, input);
    EXPECT_FALSE(capturedOk);

    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(movedToTrash, &proxy, &CanvasRecentProxy::handleReloadRecentFiles));
    EXPECT_TRUE(dpfSignalDispatcher->unsubscribe(deleteFiles, &proxy, &CanvasRecentProxy::handleReloadRecentFiles));
}
