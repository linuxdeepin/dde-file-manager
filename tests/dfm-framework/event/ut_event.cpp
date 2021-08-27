#include "event/event.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QUrl>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
class UT_Event : public testing::Test
{
public:

    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};

TEST_F(UT_Event, test_setgetTopic)
{
    Event ins;
    ins.setTopic("WindowEvent");
    EXPECT_EQ(true, ins.topic() == "WindowEvent");

    ins.setTopic("LauchEvent");
    EXPECT_EQ(false, ins.topic() == "WindowEvent");

    ins.setTopic("LauchEvent");
    EXPECT_EQ(true, ins.topic() == "LauchEvent");

    ins.setTopic("");
    EXPECT_EQ(true, ins.topic().isEmpty());

}

TEST_F(UT_Event, test_setgetData)
{
    Event ins;
    ins.setData(QUrl()); //重要功能测试
    EXPECT_EQ(QUrl().isEmpty(), ins.data().toUrl().isEmpty());

    QList<QUrl> urllist; //重要功能测试
    urllist << QUrl("file:///home");
    urllist << QUrl("file:///temp");

    ins.setData(QVariant::fromValue(urllist));
    EXPECT_EQ(true, ins.data().canConvert<QList<QUrl>>());

    auto list = qvariant_cast<QList<QUrl>>(ins.data());
    EXPECT_EQ(urllist.size(), list.size());

    EXPECT_EQ(urllist[0],list[0]); //元素对比
    EXPECT_EQ(urllist[1],list[1]); //元素对比
}

TEST_F(UT_Event, test_setgetValue)
{
    Event ins;
    ins.setData(QUrl());
    ins.setProperty("Value",10);
    ins.setProperty("Value",20);
    EXPECT_EQ(true, 20 == ins.property("Value").toInt());
    EXPECT_EQ(false, 10 == ins.property("Value").toInt());

    //重要功能测试
    QUrl url("file:///home");
    ins.setProperty("url",QVariant::fromValue<QUrl>(url));
    EXPECT_EQ(url, qvariant_cast<QUrl>(ins.property("url")));

    //重要功能测试
    QList<QUrl> urllist;
    urllist << QUrl("file:///home");
    urllist << QUrl("file:///temp");
    ins.setProperty("urls",QVariant::fromValue<QList<QUrl>>(urllist));

    auto tempUrls = qvariant_cast<QList<QUrl>>(ins.property("urls"));
    EXPECT_EQ(urllist.size(), tempUrls.size());
    EXPECT_EQ(urllist[0],tempUrls[0]); //元素对比
    EXPECT_EQ(urllist[1],tempUrls[1]); //元素对比
}
