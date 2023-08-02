// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "backgroundwm.h"

#include "stubext.h"
#include <gtest/gtest.h>
#include <QFile>


DDP_BACKGROUND_BEGIN_NAMESPACE

class UT_backGroundgroundWM : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        bwm = new BackgroundWM;
    }
    virtual void TearDown() override
    {
        delete bwm;
        stub.clear();
    }
    BackgroundWM *bwm = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_backGroundgroundWM, onAppearanceValueChanged)
{
    bool call = false;
    stub.set_lamda(&ddplugin_background::BackgroundService::backgroundChanged,[&call](){
       call = true;
    });
    bwm->onAppearanceValueChanged("backgroundUris");

    EXPECT_TRUE(call);
}



TEST_F(UT_backGroundgroundWM, getBackgroundFromConfig)
{

    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open),
                       [] { __DBG_STUB_INVOKE__ return true;});

    typedef bool(*FuncType2)();
    stub.set_lamda((FuncType2)(bool(QFileDevice::*)()const)(&QFileDevice::atEnd),[](){return false;});
    auto fun_type = static_cast<QByteArray(QIODevice::*)(qint64)>(&QIODevice::readLine);
    stub.set_lamda(fun_type,[](){ return QByteArray("test@ test = "); });
    bwm->currentWorkspaceIndex = 0;
    EXPECT_EQ(bwm->getBackgroundFromConfig(" test "),"");
}


TEST_F(UT_backGroundgroundWM, isWMActive)
{
    auto fun_type = static_cast<QDBusConnectionInterface *(QDBusConnection::*)()const>(&QDBusConnection::interface);
    stub.set_lamda(fun_type, [](){
        __DBG_STUB_INVOKE__
        QDBusConnectionInterface *res {nullptr};
        return res;
    });

    EXPECT_FALSE( bwm->isWMActive());
}

TEST_F(UT_backGroundgroundWM, background)
{
    QString screen ="temp";
    bwm->background(screen);
}

TEST_F(UT_backGroundgroundWM, getDefaultBackground)
{
    bool call = false;
    stub.set_lamda(&QString::isEmpty,[&call](){
        call = true;
        return true;
    });
    bwm->getDefaultBackground();
    EXPECT_TRUE(call);

}

DDP_BACKGROUND_END_NAMESPACE
