// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/canvasorganizer.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "mode/custommode.h"
#include <gtest/gtest.h>

#include <QMimeData>

using namespace testing;
using namespace ddplugin_organizer;

class CanvasOrganizerTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CanvasOrganizerTest, setCanvasManagerShell)
{

    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    CanvasManagerShell *sh = nullptr;
    organizer->setCanvasManagerShell(sh);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    sh = new CanvasManagerShell;
    organizer->setCanvasManagerShell(sh);
    EXPECT_EQ(organizer->canvasManagerShell, sh);

    organizer->setCanvasManagerShell(nullptr);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    delete sh;
    sh = nullptr;
    delete organizer;
}

TEST_F(CanvasOrganizerTest, setCanvasModelShell)
{
    CanvasModelShell sh;
    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    organizer->canvasModelShell = &sh;
    EXPECT_NO_FATAL_FAILURE(organizer->setCanvasModelShell(&sh));
    CanvasModelShell sh1;
    organizer->canvasModelShell = &sh1;
    EXPECT_NO_FATAL_FAILURE(organizer->setCanvasModelShell(&sh1));
    delete organizer;
}
TEST_F(CanvasOrganizerTest, setCanvasViewShell)
{
    bool connect = false;
    typedef bool (*fun_type1)(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *);
    stub.set_lamda((fun_type1)(&CanvasOrganizer::filterDropData),
                   [&connect]() {connect = true;return false; });
    CanvasViewShell sh;
    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    organizer->canvasViewShell = &sh;
    organizer->setCanvasViewShell(&sh);
    CanvasViewShell sh1;
    organizer->canvasViewShell = &sh1;
    organizer->setCanvasViewShell(&sh);
    organizer->canvasViewShell->filterDropData(1, nullptr, QPoint(), nullptr);
    EXPECT_TRUE(connect);
    delete organizer;
}

TEST_F(CanvasOrganizerTest, setCanvasGridShell)
{
    CanvasGridShell sh;
    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    EXPECT_NO_FATAL_FAILURE(organizer->setCanvasGridShell(&sh));
    CanvasGridShell sh1;
    EXPECT_NO_FATAL_FAILURE(organizer->setCanvasGridShell(&sh1));
    delete organizer;
}

TEST_F(CanvasOrganizerTest, test)
{
    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    EXPECT_NO_FATAL_FAILURE(organizer->setSurfaces(QList<SurfacePointer>()));
    QList<QUrl> lists;
    EXPECT_FALSE(organizer->filterDataRested(&lists));
    EXPECT_FALSE(organizer->filterDataInserted(QUrl()));
    QUrl url1("temp");
    QUrl url2("temp");
    EXPECT_FALSE(organizer->CanvasOrganizer::filterDataRenamed(url1, url2));
    EXPECT_FALSE(organizer->filterDropData(1, nullptr, QPoint(), nullptr));
    delete organizer;
}
