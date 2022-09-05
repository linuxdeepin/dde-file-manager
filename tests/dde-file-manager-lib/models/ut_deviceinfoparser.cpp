// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/deviceinfoparser.h"

#include <gtest/gtest.h>

namespace {
class TestDeviceInfoParser : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDeviceInfoParser";

        parser = &DeviceInfoParser::Instance();
    }

    void TearDown() override
    {
        std::cout << "end TestDeviceInfoParser";
    }

public:
    DeviceInfoParser *parser;
};
} // namespace

TEST_F(TestDeviceInfoParser, InterfaceTest)
{
    parser->refreshDabase();
    const QStringList &lst = parser->getLshwCDRomList();
    parser->isInternalDevice("/dev/sr0");
}
