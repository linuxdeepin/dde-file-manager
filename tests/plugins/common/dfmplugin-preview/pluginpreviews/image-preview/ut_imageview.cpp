// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "imageview.h"

#include <gtest/gtest.h>

#include <QMovie>

PREVIEW_USE_NAMESPACE

TEST(UT_imageView, setFile_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMovie::start, [ &isOk ]{
        isOk = true;
    });

    ImageView view("/UT_TEST", QByteArray("gif"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imageView, setFile_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMovie::start, [ &isOk ]{
        isOk = true;
    });

    ImageView view("/UT_TEST", QByteArray("gif"));
    view.setFile("/UT_TEST", QByteArray("gif"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imageView, setFile_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMovie::stop, [ &isOk ]{
        isOk = true;
    });

    ImageView view("/UT_TEST", QByteArray("png"));
    view.movie = new QMovie("/UT_TEST", QByteArray("png"));
    view.setFile("/UT_TEST", QByteArray("png"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imageView, sourceSize)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&ImageView::setFile, []{});

    ImageView view("/UT_TEST", QByteArray("gif"));
    view.sourceImageSize = QSize(0, 0);

    EXPECT_TRUE(view.sourceSize() == QSize(0, 0));
}
