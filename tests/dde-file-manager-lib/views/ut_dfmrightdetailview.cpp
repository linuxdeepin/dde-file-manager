#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include <views/dfmrightdetailview.h>
#include <views/dfmrightdetailview.cpp>
#include <QStandardPaths>

TEST(DFMRightDetailViewTest, setUrl){
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    DUrl temp = DUrl();
    DFMRightDetailView ddv(temp);

    EXPECT_TRUE(ddv.d_func()->m_url.isEmpty());

    ddv.setUrl(desktopUrl);
    auto expectValue = ddv.d_func()->m_url == desktopUrl;
    EXPECT_TRUE(expectValue);
}

TEST(DFMRightDetailViewTest, setTagWidgetVisible){
    DUrl temp = DUrl();
    DFMRightDetailView ddv(temp);

    auto tagVisible = ddv.d_func()->tagInfoWidget->isVisible();
    if(tagVisible){
        ddv.setTagWidgetVisible(false);
        EXPECT_TRUE(tagVisible != false);
    }
    else {
        ddv.setTagWidgetVisible(true);
        EXPECT_TRUE(tagVisible != true);
    }
}
