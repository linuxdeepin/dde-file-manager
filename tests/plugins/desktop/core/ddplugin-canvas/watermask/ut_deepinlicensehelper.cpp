// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermask/deepinlicensehelper.h"

#include "watermask/licenceInterface.h"

#include <QApplication>
#include <QtConcurrent>

#include "stubext.h"
#include <gtest/gtest.h>
using namespace ddplugin_canvas;

class UT_DeepinLicenseHelper : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        helper = new DeepinLicenseHelper();
    }
    virtual void TearDown() override { delete helper; stub.clear(); }

protected:
    stub_ext::StubExt stub;
    DeepinLicenseHelper *helper = nullptr;
};

TEST_F(UT_DeepinLicenseHelper,requestLicenseState)
{
     EXPECT_NO_FATAL_FAILURE(helper->requestLicenseState());
}

TEST_F(UT_DeepinLicenseHelper,initFinshed)
{
    bool connection = false;
    stub.set_lamda(&DeepinLicenseHelper::requestLicenseState,[&connection](){connection = true;});
    ComDeepinLicenseInterface *interface = new ComDeepinLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->initFinshed(interface);
    helper->licenseInterface->LicenseStateChange();
    EXPECT_TRUE(connection);
}

TEST_F(UT_DeepinLicenseHelper,getLicenseState)
{
    DeepinLicenseHelper self;
    self.licenseInterface = new ComDeepinLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    bool connection = false;
    QObject::connect(&self,&DeepinLicenseHelper::postLicenseState,&self,[&connection](int _t1, int _t2){connection = true;});
    helper->getLicenseState(&self);
    EXPECT_TRUE(connection);
}

TEST_F(UT_DeepinLicenseHelper,getServiceProperty)
{
    ComDeepinLicenseInterface *interface = new ComDeepinLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->licenseInterface = interface;
    interface->setProperty("ServiceProperty",QVariant::fromValue(QString("0")));
    DeepinLicenseHelper::LicenseProperty res = helper->getServiceProperty();
    EXPECT_EQ(res,DeepinLicenseHelper::LicenseProperty::Noproperty);
}

TEST_F(UT_DeepinLicenseHelper,getAuthorizationProperty)
{
    ComDeepinLicenseInterface *interface = new ComDeepinLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->licenseInterface = interface;
    DeepinLicenseHelper::LicenseProperty res =  helper->getAuthorizationProperty();
    EXPECT_EQ(res,DeepinLicenseHelper::LicenseProperty::Noproperty);
}
