// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "views/computerviewcontainer.h"
#include "views/computerview.h"
#include "views/computerstatusbar.h"

#include <QUrl>
#include <QWidget>
#include <QVBoxLayout>

using namespace dfmplugin_computer;

class UT_ComputerViewContainer : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        container = new ComputerViewContainer(QUrl());
    }

    virtual void TearDown() override
    {
        delete container;
        container = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerViewContainer *container = nullptr;
};

TEST_F(UT_ComputerViewContainer, SetRootUrl)
{
    QUrl testUrl("computer:///");
    container->setRootUrl(testUrl);
    EXPECT_EQ(container->rootUrl(), testUrl);
}

TEST_F(UT_ComputerViewContainer, ContentWidget)
{
    EXPECT_EQ(container->contentWidget(), container->viewContainer);
}

TEST_F(UT_ComputerViewContainer, Widget)
{
    EXPECT_TRUE(container->widget());
}

TEST_F(UT_ComputerViewContainer, ViewState)
{
    EXPECT_EQ(container->viewState(), AbstractBaseView::ViewState::kViewIdle);
}

TEST_F(UT_ComputerViewContainer, SelectedUrlList)
{
    stub.set_lamda(VADDR(ComputerView, selectedUrlList), [] { __DBG_STUB_INVOKE__ return QList<QUrl>(); });

    auto list = container->selectedUrlList();
    EXPECT_TRUE(list.isEmpty());
}
