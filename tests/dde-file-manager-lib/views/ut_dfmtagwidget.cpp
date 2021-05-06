/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include <gmock/gmock-matchers.h>
#include "dfmglobal.h"
#include "stub.h"

#define private public
#define protected public

#include <views/dfmtagwidget.h>
#include <views/dfmtagwidget.cpp>
#include <QStandardPaths>
#include <QDir>

DWIDGET_USE_NAMESPACE
DFM_BEGIN_NAMESPACE
using namespace testing;
namespace  {
    class DFMTagWidgetTest : public Test
    {
    public:
        DFMTagWidgetTest():Test() {

        }
        virtual void SetUp() override {
            //拿到桌面路径
            QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
            //创建一个文件夹用于测试
            m_utDirUrl = desktopPath + "/testForUtTest";
            m_dirTest = new QDir(m_utDirUrl);
            if(!m_dirTest->exists())
                EXPECT_TRUE(m_dirTest->mkdir(m_utDirUrl));
            DUrl desktopUrl = DUrl::fromLocalFile(m_utDirUrl);
            m_dw = new  DFMTagWidget(desktopUrl);
        }

        virtual void TearDown() override {
            delete m_dw;
            m_dw = nullptr;
            m_dirTest->rmdir(m_utDirUrl);
            delete m_dirTest;
            m_dirTest = nullptr;

        }
        QDir *m_dirTest;
        DFMTagWidget *m_dw;
        QString m_utDirUrl;
    };
}

static bool inThere = false;
TEST_F(DFMTagWidgetTest, initConnection){
    // 阻塞CI
    // Stub stub;
    // auto utLoadTags = static_cast<void(*)(const DUrl &)>([](const DUrl &durl){
    //     inThere = true;
    // });
    // stub.set(ADDR(DFMTagWidget, loadTags), utLoadTags);
    // emit m_dw->d_func()->m_tagActionWidget->checkedColorChanged("Red");
    // m_dw->loadTags(DUrl(m_utDirUrl));
    // EXPECT_TRUE(inThere);
}

TEST_F(DFMTagWidgetTest, loadTags){
    m_dw->loadTags(DUrl(m_utDirUrl));
    auto expectValue = m_dw ->d_func()->m_url.path() == m_utDirUrl;
    EXPECT_TRUE(expectValue);

    Stub stub;
    auto utGetTagsThroughFiles = static_cast<QList<QString> (*)(const QList<DUrl> &)>([](const QList<DUrl> &){
        QList<QString> temp{"橙色"};
        return temp;
    });

    auto utGetColorByDisplayName = static_cast<QString(*)(const QString &)>([](const QString &tt){
        QString temp = "Orange";
        return temp;
    });
    stub.set(ADDR(TagManager, getTagsThroughFiles), utGetTagsThroughFiles);
    stub.set(ADDR(TagManager, getColorByDisplayName), utGetColorByDisplayName);
    m_dw->loadTags(DUrl(m_utDirUrl));

    auto expectValue1 = m_dw ->d_func()->m_url.path() == m_utDirUrl;
    EXPECT_TRUE(expectValue1);
}

TEST_F(DFMTagWidgetTest, shouldShow){
    auto tempp = m_utDirUrl;
    DUrl temppp(m_utDirUrl);
    DUrl url = m_dw->d_func()->redirectUrl(temppp);

    Stub stub;
    bool (*ut_whetherShowTagActions)() = [](){return true;};
    stub.set(ADDR(DFileMenuManager,whetherShowTagActions), ut_whetherShowTagActions);

    auto expectValue = m_dw->shouldShow(url);
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMTagWidgetTest, tagTitle){
    auto tempp = m_utDirUrl;
    DUrl temppp(m_utDirUrl);
    DUrl url = m_dw->d_func()->redirectUrl(temppp);
    auto expectValue = m_dw->d_func()->m_tagLable == m_dw->tagTitle();
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMTagWidgetTest, tagLeftTitle){
    auto expectValue = m_dw->d_func()->m_tagLeftLable == m_dw->tagLeftTitle();
    EXPECT_TRUE(expectValue);
}

TEST(DFMCrumbEdit, mouse_double_click_event){
    DCrumbEdit tempEdit;
    DFMCrumbEdit temp(&tempEdit);
    temp.m_isEditByDoubleClick = true;
    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    temp.mouseDoubleClickEvent(&me);
    EXPECT_FALSE(temp.m_isEditByDoubleClick);
}



DFM_END_NAMESPACE
