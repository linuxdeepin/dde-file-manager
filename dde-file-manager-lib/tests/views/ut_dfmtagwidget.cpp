#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "dfmglobal.h"

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
            m_dirTest->rmdir(m_utDirUrl);
            delete m_dirTest;

        }
        QDir *m_dirTest;
        DFMTagWidget *m_dw;
        QString m_utDirUrl;
    };
}


TEST_F(DFMTagWidgetTest, loadTags){
    m_dw->loadTags(DUrl(m_utDirUrl));
    auto expectValue = m_dw ->d_func()->m_url.path() == m_utDirUrl;
    EXPECT_TRUE(expectValue);
}

TEST_F(DFMTagWidgetTest, shouldShow){
    auto tempp = m_utDirUrl;
    DUrl temppp(m_utDirUrl);
    DUrl url = m_dw->d_func()->redirectUrl(temppp);
    auto expectValue = m_dw->shouldShow(url);
    EXPECT_TRUE(expectValue);
}

DFM_END_NAMESPACE
