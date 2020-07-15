#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
//#include "../../dde-file-manager-lib/interfaces/durl.h"
#include "durl.h"

using namespace testing;

class DurlTest:public testing::Test{

public:

    DUrl url ;
    virtual void SetUp() override{
        std::cout << "start DurlTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DurlTest" << std::endl;
    }
};

TEST_F(DurlTest, can_get_filename)
{
    url.setPath(QString("/data/home/max/Work/myfile.txt"));
    EXPECT_EQ("myfile.txt", url.fileName());
}

TEST_F(DurlTest, can_get_filepath)
{
    url.setPath(QString("/data/home/max/Work/myfile.txt"));
    std::cout << "test path" << url.path().toStdString() << std::endl;
    EXPECT_EQ("/data/home/max/Work/myfile.txt", url.path().toStdString());
}
