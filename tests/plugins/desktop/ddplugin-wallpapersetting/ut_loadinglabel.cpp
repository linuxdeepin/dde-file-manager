// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "loadinglabel.h"

#include "stubext.h"
#include <gtest/gtest.h>
#include <QLabel>
#include <DAnchors>
#include <DSpinner>
DWIDGET_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;
class UT_LoadingLabel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ll = new LoadingLabel;
    }
    virtual void TearDown() override
    {
        delete ll;
        stub.clear();
    }
    LoadingLabel *ll = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_LoadingLabel, start)
{
    bool call = false;
    auto fun_type = static_cast<bool (*)(QWidget *, const Qt::AnchorPoint &, QWidget *, const Qt::AnchorPoint &)>(&DAnchorsBase::setAnchor);
    stub.set_lamda(fun_type,[&call](QWidget *, const Qt::AnchorPoint &, QWidget *, const Qt::AnchorPoint &){
        __DBG_STUB_INVOKE__
        call = true;
        return true;
    });

    DAnchorsBase base(ll->animationSpinner);
    auto fun_type1 = static_cast<DAnchorsBase*(*)(const QWidget*)>(&DAnchorsBase::getAnchorBaseByWidget);
    stub.set_lamda (fun_type1,[&base](const QWidget* ){
        __DBG_STUB_INVOKE__
        return &base;
    });
    ll->start();
    EXPECT_TRUE( ll->animationSpinner->testAttribute(Qt::WA_TransparentForMouseEvents));
    EXPECT_FALSE(ll->animationSpinner->hasFocus());
    EXPECT_EQ(ll->animationSpinner->size(),QSize(24, 24));
    EXPECT_TRUE(call);
    EXPECT_EQ(base.rightMargin(),6);
}

TEST_F(UT_LoadingLabel, resize)
{
    QSize size(250,250);
    ll->resize(size);

    EXPECT_EQ(ll->icon->size(),ll->iconSize);
    EXPECT_EQ(ll->contant->size(),ll->contantSize);

    size.setWidth(200);
    ll->resize(size);
    EXPECT_EQ(ll->icon->size(),QSize(0,0));
    EXPECT_EQ(ll->contant->size(),QSize(0,0));
}

TEST_F(UT_LoadingLabel, setText)
{
    const QString str = QString("temp");
    ll->setText(str);
    EXPECT_EQ(str,ll->contant->text());
}
