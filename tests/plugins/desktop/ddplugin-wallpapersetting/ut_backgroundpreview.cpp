// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
