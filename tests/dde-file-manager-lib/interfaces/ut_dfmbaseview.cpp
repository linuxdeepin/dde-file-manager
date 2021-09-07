/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>
#include <QSharedPointer>
#include <QProcess>

#include "stub.h"
#include "stub-ext/stubext.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfilemenumanager.h"
#include "testhelper.h"
#include "durl.h"
#include "interfaces/dfmbaseview.h"

using namespace testing;
DFM_USE_NAMESPACE

#include <QObject>
class TestDfmBaseView: public DFMBaseView,public QObject
{
public:
    TestDfmBaseView() : DFMBaseView(),QObject() {

    }
    QWidget *widget() const override{
        QWidget *pWidget = new QWidget();
        pWidget->deleteLater();
        return pWidget;
    }
    DUrl rootUrl() const override{
        return DUrl();
    }
    bool setRootUrl(const DUrl &url) override{
        return true;
    }
    void deleteLater() {
        DFMBaseView::deleteLater();
    }
    void notifyUrlChanged(){
        DFMBaseView::notifyUrlChanged();
    }
    void notifyStateChanged(){
        DFMBaseView::notifyStateChanged();
    }
    void requestCdTo(const DUrl &url){
        DFMBaseView::requestCdTo(url);
    }
    void notifySelectUrlChanged(const QList<DUrl> &urlList){
        DFMBaseView::notifySelectUrlChanged(urlList);
    }
};


class DfmBaseViewTest:public testing::Test{

public:
    virtual void SetUp() override{
        baseview.reset(new TestDfmBaseView());
        std::cout << "start DfmBaseViewTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DfmBaseViewTest" << std::endl;
    }

    QSharedPointer<TestDfmBaseView> baseview = nullptr;
};

TEST_F(DfmBaseViewTest,start_deleteLater) {
    TestDfmBaseView *viewptr = new TestDfmBaseView();
    EXPECT_NO_FATAL_FAILURE(viewptr->deleteLater());
    TestHelper::runInLoop([](){});
}

TEST_F(DfmBaseViewTest,start_widget) {
    EXPECT_TRUE(baseview->widget());
}

TEST_F(DfmBaseViewTest,start_rootUrl) {
    EXPECT_FALSE(baseview->rootUrl().isValid());
}

TEST_F(DfmBaseViewTest,start_viewState) {
    EXPECT_EQ(DFMBaseView::ViewIdle,baseview->viewState());
}

TEST_F(DfmBaseViewTest,start_setRootUrl) {
    EXPECT_TRUE(baseview->setRootUrl(DUrl()));
}

TEST_F(DfmBaseViewTest,start_toolBarActionList) {
    EXPECT_TRUE(baseview->toolBarActionList().count() == 0);
}

TEST_F(DfmBaseViewTest,start_reflesh) {
    EXPECT_NO_FATAL_FAILURE(baseview->refresh());
}

TEST_F(DfmBaseViewTest,start_other) {

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileMenuManager, needDeleteAction), [](){return true;});

    DFileManagerWindow testbaseview;

    stu.set_lamda(ADDR(QWidget, window), [&testbaseview](){ return qobject_cast<QWidget*>(&testbaseview);});

    bool (*cd)(const DUrl &) = [](const DUrl &){return true;};
    bool (*cdForTab)(int , const DUrl &) = [](int ,const DUrl &){return true;};
    bool (*cdForTabByView)(DFMBaseView *, const DUrl &) = [](DFMBaseView *, const DUrl &){return true;};
    stu.set(ADDR(DFileManagerWindow,cd),cd);
    stu.set(ADDR(DFileManagerWindow,cdForTab),cdForTab);
    stu.set(ADDR(DFileManagerWindow,cdForTabByView),cdForTabByView);
    TestHelper::runInLoop([=]{
        ASSERT_NO_FATAL_FAILURE(baseview->notifyUrlChanged());
    });
    testbaseview.clearActions();
    TestHelper::runInLoop([=]{
        EXPECT_NO_FATAL_FAILURE(baseview->notifyStateChanged());
    });
    testbaseview.clearActions();
    EXPECT_NO_FATAL_FAILURE(baseview->requestCdTo(DUrl()));
    testbaseview.clearActions();
    TestHelper::runInLoop([=]{
        EXPECT_NO_FATAL_FAILURE(baseview->notifySelectUrlChanged(DUrlList()));
    });
    stu.reset(ADDR(QWidget, window));
    testbaseview.clearActions();
}
