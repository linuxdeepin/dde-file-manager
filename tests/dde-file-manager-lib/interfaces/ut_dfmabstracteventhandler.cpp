/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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

#include "dfmevent.h"
#define protected public
#include "dfmabstracteventhandler.h"

DFM_USE_NAMESPACE

namespace  {
class TestDFMAbstractEventHandler: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestDFMAbstractEventHandler" << std::endl;
        handler = new DFMAbstractEventHandler();
    }

    virtual void TearDown() override
    {
        std::cout << "start TestDFMAbstractEventHandler" << std::endl;
        if (handler) {
            delete handler;
            handler = nullptr;
        }
    }

public:
    DFMAbstractEventHandler *handler;
};
}

TEST_F(TestDFMAbstractEventHandler, object)
{
    EXPECT_TRUE(handler->object() == nullptr);
}

TEST_F(TestDFMAbstractEventHandler, fmEvent)
{
    EXPECT_FALSE(handler->fmEvent(dMakeEventPointer<DFMEvent>()));
}

TEST_F(TestDFMAbstractEventHandler, fmEventFilter)
{
    EXPECT_FALSE(handler->fmEventFilter(dMakeEventPointer<DFMEvent>()));
}
