// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "backgrounddde.h"
#include "stubext.h"

#include <gtest/gtest.h>



DDP_BACKGROUND_BEGIN_NAMESPACE
class UT_backGroundgroundDDE : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        dde = new  BackgroundDDE;
    }
    virtual void TearDown() override
    {
        delete dde;
        stub.clear();
    }
    BackgroundDDE *dde = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_backGroundgroundDDE, getBackgroundFromDDE)
{
    bool call = true;
    dde->getBackgroundFromDDE("temp");
    EXPECT_TRUE(call);
}

TEST_F(UT_backGroundgroundDDE, getBackgroundFromConfig)
{
    bool call = false;
    typedef bool(*FuncType1)(QIODevice::OpenMode);
    stub.set_lamda((FuncType1)(bool(QFile::*)(QIODevice::OpenMode))(&QFile::open),
                       [] { __DBG_STUB_INVOKE__ return true;});
    QJsonDocument *qjson = new QJsonDocument;
    stub.set_lamda((QJsonDocument(*)(const QByteArray&,QJsonParseError *))(&QJsonDocument::fromJson),
                   [&call,&qjson]( const QByteArray &json, QJsonParseError *error = nullptr){
        __DBG_STUB_INVOKE__
        call = true;
        error->error= QJsonParseError::NoError;

        return *qjson; });

    stub.set_lamda(&QJsonDocument::isArray,[&call](){__DBG_STUB_INVOKE__ call = true;return true;});
    stub.set_lamda(&QJsonArray::size,[](){__DBG_STUB_INVOKE__ return 1;});
    stub.set_lamda(&QJsonValue::isObject,[&call](){__DBG_STUB_INVOKE__ call = true; return true;});
    typedef QString(*FuncType)(void);
    stub.set_lamda((FuncType)(QString(QJsonValue::*)(void)const)(&QJsonValue::toString),[](){__DBG_STUB_INVOKE__ return "index+monitorName";});
    stub.set_lamda(&QJsonValue::isArray,[&call](){__DBG_STUB_INVOKE__ call = true; return true;});
    dde->getBackgroundFromConfig("temp");
    EXPECT_TRUE(call);
    delete qjson;
}

TEST_F(UT_backGroundgroundDDE, background)
{
    bool call = false;
    stub.set_lamda(&BackgroundDDE::getBackgroundFromDDE,[](){
       __DBG_STUB_INVOKE__
       return "file:/temp";
    });
    auto funType = static_cast<bool(*)(const QString &)>(&QFile::exists);
    stub.set_lamda(funType,[&call](const QString &){
                   __DBG_STUB_INVOKE__
                   call =true;
                   return false;
         });

    EXPECT_EQ(QString("/usr/share/backgrounds/default_background.jpg"), dde->background("temp"));
    EXPECT_TRUE(call);
    call = false;
    dde->background("");
    EXPECT_FALSE(call);
}

TEST_F(UT_backGroundgroundDDE, getDefaultBackground)
{
    EXPECT_EQ(QString("/usr/share/backgrounds/default_background.jpg"),dde->getDefaultBackground());
}

DDP_BACKGROUND_END_NAMESPACE
