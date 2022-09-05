// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "io/dfilehandler.h"

using namespace testing;
DFM_USE_NAMESPACE
class TestDFileHandler : public DFileHandler {
public:
    TestDFileHandler() : DFileHandler() {

    }
};

class DFileHandlerTest:public testing::Test{

public:
    virtual void SetUp() override{
//        job.reset(new TestDFileHandler());
        std::cout << "start DFileHandlerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileHandlerTest" << std::endl;
    }

    QSharedPointer<TestDFileHandler> handler = nullptr;
};

