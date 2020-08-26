#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QDebug>
#include "../chinese2pinyin/chinese2pinyin.h"

TEST(Chinese2PinYintest,Chinese2PinYin)
{
    QString temp="stgsgfh";
    int sz=temp.size();
    QString result = Pinyin::Chinese2Pinyin(temp);
    QString compare=result.mid(0,sz);
    EXPECT_EQ(temp,compare);
    EXPECT_GE(result.size(),sz);
}
