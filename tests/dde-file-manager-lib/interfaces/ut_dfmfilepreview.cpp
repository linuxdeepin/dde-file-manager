/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghao<yanghao@uniontech.com>
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

#include "interfaces/dfmfilepreview.h"

DFM_USE_NAMESPACE
namespace  {

    class MockDFMFilePreview: public DFMFilePreview
    {
    public:
        explicit MockDFMFilePreview(QObject *parent = nullptr): DFMFilePreview(parent)
        {

        }

        bool setFileUrl(const DUrl &url){
            m_url = url;
            return true;
        }
        DUrl fileUrl() const {
            return m_url;
        }

        QWidget *contentWidget() const {
            return 0;
        }
    private:
        DUrl m_url;
    };
    class TestDFMFilePreview : public testing::Test
    {
        void SetUp() override
        {
            std::cout << "start TestDFMFilePreview" << std::endl;
            m_preview = new MockDFMFilePreview();
        }

        void TearDown() override
        {
            std::cout << "end TestDFMFilePreview" << std::endl;
            delete m_preview;
            m_preview = nullptr;
        }
    public:
        DFMFilePreview *m_preview;

    };

/*
TEST_F(TestDFMFilePreview, test_filePreview)
{
    m_preview->initialize(nullptr, nullptr);
    m_preview->statusBarWidget();
    m_preview->statusBarWidgetAlignment();
    m_preview->title() ;
    m_preview->showStatusBarSeparator();
    m_preview->DoneCurrent();
    m_preview->play();
    m_preview->pause();
    m_preview->stop();
    DFMGlobal::clearClipboard();
    DUrl tmpUrl = DUrl::fromLocalFile("/tmp");
    EXPECT_TRUE(m_preview->setFileUrl(tmpUrl));
    m_preview->copyFile();
    QList<QUrl> clipUrls = DFMGlobal::fetchUrlsFromClipboard();
    EXPECT_EQ(clipUrls.first().toString(), tmpUrl.toString());
    EXPECT_EQ(m_preview->contentWidget(), nullptr);
}*/
}
