/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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

