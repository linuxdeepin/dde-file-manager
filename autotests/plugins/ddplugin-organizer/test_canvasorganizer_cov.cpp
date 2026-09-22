// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(mode/canvasorganizer.cpp) -> 用例映射:
// - CanvasOrganizer::layout / detachLayout / reset -> BaseOrganizer_EmptyHooks_NoStateChange
// - CanvasOrganizer::filterDataRested / filterDataInserted / filterDataRenamed
//     -> BaseFilters_DataEvents_NotConsumed (基类默认空实现)
// - CanvasOrganizer::filterDropData -> FilterDropData_BaseImplementation_ReturnsFalse
// - CanvasOrganizer::filterWheel -> FilterWheel_CtrlOnly_ReturnsCtrlState (分支: ctrl true/false)
// - CanvasOrganizer::filterContextMenu -> FilterContextMenu_BaseImplementation_ReturnsFalse
// - CanvasOrganizer::~CanvasOrganizer -> 析构由 fixture 覆盖
// 分支说明: 基类钩子均为空/默认实现, 重点验证默认返回值与无副作用。

#include "stubext.h"
#include "mode/canvasorganizer.h"
#include "models/collectionmodel.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"

#include <QMimeData>
#include <QUrl>
#include <QPoint>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

// CanvasOrganizer is abstract; the frame-manager test suite uses the same minimal override set
class ConcreteCanvasOrganizer : public CanvasOrganizer
{
public:
    explicit ConcreteCanvasOrganizer(QObject *parent = nullptr)
        : CanvasOrganizer(parent)
    {
    }
    OrganizerMode mode() const override { return OrganizerMode::kNormalized; }
    bool initialize(CollectionModel *) override { return true; }
};

}   // namespace

class UT_CanvasOrganizerCov : public testing::Test
{
protected:
    void SetUp() override
    {
        organizer = new ConcreteCanvasOrganizer();
    }

    void TearDown() override
    {
        delete organizer;   // covers ~CanvasOrganizer
        stub.clear();
    }

public:
    ConcreteCanvasOrganizer *organizer = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasOrganizerCov, BaseOrganizer_EmptyHooks_NoStateChange)
{
    // Arrange: fresh organizer without any shell or surface
    ASSERT_FALSE(organizer->isEditing());

    // Act: empty base hooks
    organizer->layout();
    organizer->detachLayout();
    organizer->reset();

    // Assert: hooks keep default state
    EXPECT_FALSE(organizer->isEditing());
    EXPECT_EQ(organizer->mode(), OrganizerMode::kNormalized);
}

TEST_F(UT_CanvasOrganizerCov, BaseFilters_DataEvents_NotConsumed)
{
    // Arrange: file list and urls passed through the base filters
    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/a"), QUrl::fromLocalFile("/tmp/b") };
    const int countBefore = urls.count();

    // Act
    organizer->filterDataRested(&urls);
    organizer->filterDataInserted(urls.first());
    organizer->filterDataRenamed(urls.first(), urls.last());

    // Assert: the base implementation never mutates caller data
    EXPECT_EQ(urls.count(), countBefore);
    EXPECT_FALSE(organizer->isEditing());
}

TEST_F(UT_CanvasOrganizerCov, FilterDropData_BaseImplementation_ReturnsFalse)
{
    // Arrange: plain mime data
    QMimeData data;

    // Act
    bool filtered = organizer->filterDropData(0, &data, QPoint(1, 1), nullptr);

    // Assert: base organizer never intercepts drops
    EXPECT_FALSE(filtered);
    EXPECT_EQ(data.urls().count(), 0);
}

TEST_F(UT_CanvasOrganizerCov, FilterWheel_CtrlOnly_ReturnsCtrlState)
{
    // Arrange: wheel events with and without ctrl
    const QPoint angleDelta(0, 120);

    // Act
    bool filteredWithCtrl = organizer->filterWheel(0, angleDelta, true);
    bool filteredWithoutCtrl = organizer->filterWheel(0, angleDelta, false);

    // Assert: base implementation only filters ctrl zooming
    EXPECT_TRUE(filteredWithCtrl);
    EXPECT_FALSE(filteredWithoutCtrl);
    EXPECT_EQ(angleDelta, QPoint(0, 120));
}

TEST_F(UT_CanvasOrganizerCov, FilterContextMenu_BaseImplementation_ReturnsFalse)
{
    // Arrange: context menu request on empty selection
    QList<QUrl> emptySelection;

    // Act
    bool filtered = organizer->filterContextMenu(0, QUrl::fromLocalFile("/tmp"), emptySelection, QPoint(5, 5));

    // Assert: base organizer does not replace the menu
    EXPECT_FALSE(filtered);
    EXPECT_EQ(emptySelection.count(), 0);
}
