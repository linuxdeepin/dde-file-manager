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

#define private public
#include "interfaces/dfmapplication.cpp"
#undef private

#include <QSignalSpy>

DFM_USE_NAMESPACE
namespace  {
static DFMApplication m_application;
class TestDFMApplication : public testing::Test
{
    void SetUp() override
    {
        std::cout << "start TestDFMApplication" << std::endl;
    }

    void TearDown() override
    {
        std::cout << "end TestDFMApplication" << std::endl;
    }
};


TEST_F(TestDFMApplication, test_appAttribute)
{
    EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
}

TEST_F(TestDFMApplication, test_appUrlAttribute)
{
    DUrl url = m_application.appUrlAttribute(DFMApplication::AA_UrlOfNewWindow);
    EXPECT_EQ(url.toString(), COMPUTER_ROOT);
}

TEST_F(TestDFMApplication, test_setAppAttribute)
{
    EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
    m_application.setAppAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow, true);
    EXPECT_TRUE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
    m_application.setAppAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow, false);
    EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
}

TEST_F(TestDFMApplication, test_syncAppAttribute)
{
    EXPECT_TRUE(m_application.syncAppAttribute());
}

TEST_F(TestDFMApplication, test_genericAttribute)
{
    EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
}

TEST_F(TestDFMApplication, test_setGenericAttribute)
{
    EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
    m_application.setGenericAttribute(DFMApplication::GenericAttribute::GA_AutoMount, false);
    EXPECT_FALSE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
    m_application.setGenericAttribute(DFMApplication::GenericAttribute::GA_AutoMount, true);
    EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
}

TEST_F(TestDFMApplication, test_syncGenericAttribute)
{
    EXPECT_TRUE(m_application.syncGenericAttribute());
}

TEST_F(TestDFMApplication, test_instance)
{
    EXPECT_NE(m_application.instance(), nullptr);
}

TEST_F(TestDFMApplication, test_genericSetting)
{
    EXPECT_NE(m_application.genericSetting(), nullptr);
}

TEST_F(TestDFMApplication, test_appSetting)
{
    EXPECT_NE(m_application.appSetting(), nullptr);
}

TEST_F(TestDFMApplication, test_enericObtuselySetting)
{
    EXPECT_NE(m_application.genericObtuselySetting(), nullptr);
}

TEST_F(TestDFMApplication, test_appObtuselySetting)
{
    EXPECT_NE(m_application.appObtuselySetting(), nullptr);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_applicationAttribute_edited)
{
    QSignalSpy spy(&m_application, SIGNAL(appAttributeEdited(ApplicationAttribute, const QVariant &)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(ApplicationAttribute), QT_STRINGIFY(AllwayOpenOnNewWindow), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_iconSizeLevelChanged)
{
    QSignalSpy spy(&m_application, SIGNAL(iconSizeLevelChanged(int)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(ApplicationAttribute), QT_STRINGIFY(IconSizeLevel), 0);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_viewModeChanged)
{
    QSignalSpy spy(&m_application, SIGNAL(viewModeChanged(int)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(ApplicationAttribute), QT_STRINGIFY(ViewMode), 0);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_genericAttribute_edited)
{
    QSignalSpy spy(&m_application, SIGNAL(genericAttributeChanged(GenericAttribute, const QVariant &)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(IndexInternal), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_previewAttributeChanged)
{
    QSignalSpy spy(&m_application, SIGNAL(previewAttributeChanged(GenericAttribute, bool)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(PreviewImage), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_showedHiddenFilesChanged)
{
    QSignalSpy spy(&m_application, SIGNAL(showedHiddenFilesChanged(bool)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(ShowedHiddenFiles), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_recentDisplayChanged)
{
    QSignalSpy spy(&m_application, SIGNAL(recentDisplayChanged(bool)));
    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(ShowRecentFileEntry), true);
    EXPECT_EQ(spy.count(), 1);
}

//TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_previewCompressFileChanged)
//{
//    QSignalSpy spy(&m_application, SIGNAL(previewCompressFileChanged(bool)));
//    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(PreviewCompressFile), true);
//    EXPECT_EQ(spy.count(), 1);
//}

//TEST_F(TestDFMApplication, test_q_onSettingsValueChanged_csdClickableAreaAttributeChanged)
//{
//    QSignalSpy spy(&m_application, SIGNAL(csdClickableAreaAttributeChanged(bool)));
//    m_application.d_func()->_q_onSettingsValueEdited(QT_STRINGIFY(GenericAttribute), QT_STRINGIFY(ShowCsdCrumbBarClickableArea), true);
//    EXPECT_EQ(spy.count(), 1);
//}

}
