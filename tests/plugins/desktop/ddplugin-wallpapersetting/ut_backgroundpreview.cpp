/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "backgroundpreview.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_WALLPAERSETTING_USE_NAMESPACE

TEST(BackgroundPreview, BackgroundPreview)
{
    BackgroundPreview wid(":0");
    EXPECT_EQ(wid.screen, QString(":0"));
    EXPECT_TRUE(wid.testAttribute(Qt::WA_TranslucentBackground));
}

TEST(BackgroundPreview, setMode)
{
    BackgroundPreview wid(":0");

    wid.setMode(0);
    EXPECT_EQ(wid.displayMode, BackgroundPreview::Statics);
}

TEST(BackgroundPreview, setDisplay)
{
    BackgroundPreview wid(":0");
    stub_ext::StubExt stub;
    bool call = false;
    stub.set_lamda(VADDR(BackgroundPreview, updateDisplay), [&call](){
        call = true;
    });

    wid.setDisplay("/test");
    EXPECT_EQ(wid.filePath, QString("/test"));
    EXPECT_TRUE(call);
}

TEST(BackgroundPreview, getPixmap_empty)
{
    BackgroundPreview wid(":0");
    QPixmap pix(100, 100);

    EXPECT_EQ(wid.getPixmap("", pix), pix);
}

TEST(BackgroundPreview, getPixmap_invalid)
{
    BackgroundPreview wid(":0");
    QPixmap pix(100, 100);

    EXPECT_EQ(wid.getPixmap("/tmp/dde-desktop/ssssssssssssssssssx.png", pix), pix);
}
