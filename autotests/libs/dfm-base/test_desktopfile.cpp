// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopfile.cpp
 * @brief Unit tests for DesktopFile (desktopfile.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include <dfm-base/utils/desktopfile.h>

using namespace dfmbase;

namespace {
QString writeDesktop(const QString &path, const QString &content)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&f);
    out << content;
    f.close();
    return path;
}
}   // namespace

TEST(DesktopFileTest, EmptyFileNameYieldsDefaults)
{
    DesktopFile df("");
    EXPECT_EQ(df.desktopFileName(), QString(""));
    EXPECT_EQ(df.desktopPureFileName(), QString(""));
    // type is left default-constructed (empty) when file is not loaded
    EXPECT_EQ(df.desktopType(), QString(""));
    EXPECT_TRUE(df.desktopCategories().isEmpty());
    EXPECT_TRUE(df.desktopMimeType().isEmpty());
    EXPECT_FALSE(df.isNoShow());
}

TEST(DesktopFileTest, NonExistentFileYieldsDefaults)
{
    DesktopFile df("/no/such/desktop/file.desktop");
    EXPECT_EQ(df.desktopType(), QString(""));
}

TEST(DesktopFileTest, ParsesFullDesktopEntry)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();

    writeDesktop(path,
                 "[Desktop Entry]\n"
                 "Name=MyApp\n"
                 "Exec=myapp %f\n"
                 "Icon=myapp-icon\n"
                 "Type=Application\n"
                 "Categories=Utility;System;\n"
                 "MimeType=text/plain;image/png;\n"
                 "X-Deepin-AppID=myapp\n"
                 "X-Deepin-Vendor=deepin\n"
                 "NoDisplay=true\n");

    DesktopFile df(path);
    EXPECT_EQ(df.desktopExec(), QString("myapp %f"));
    EXPECT_EQ(df.desktopIcon(), QString("myapp-icon"));
    EXPECT_EQ(df.desktopType(), QString("Application"));
    EXPECT_EQ(df.desktopDeepinId(), QString("myapp"));
    EXPECT_EQ(df.desktopDeepinVendor(), QString("deepin"));
    EXPECT_TRUE(df.desktopCategories().contains("Utility"));
    EXPECT_TRUE(df.desktopCategories().contains("System"));
    EXPECT_TRUE(df.desktopMimeType().contains("text/plain"));
    EXPECT_TRUE(df.desktopMimeType().contains("image/png"));
}

TEST(DesktopFileTest, IsNoShowWhenHidden)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path, "[Desktop Entry]\nName=H\nHidden=true\n");

    DesktopFile df(path);
    EXPECT_TRUE(df.isNoShow());
}

TEST(DesktopFileTest, IsNoShowWhenNoDisplayWithoutMime)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path, "[Desktop Entry]\nName=N\nNoDisplay=true\n");

    DesktopFile df(path);
    EXPECT_TRUE(df.isNoShow());
}

TEST(DesktopFileTest, NotNoShowWhenNoDisplayWithMime)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path,
                 "[Desktop Entry]\nName=N\nNoDisplay=true\nMimeType=text/plain;\n");

    DesktopFile df(path);
    EXPECT_FALSE(df.isNoShow());
}

TEST(DesktopFileTest, DesktopPureFileNameStripsPathAndExtension)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path, "[Desktop Entry]\nName=X\n");

    DesktopFile df(path);
    QString pure = df.desktopPureFileName();
    EXPECT_FALSE(pure.endsWith(".desktop"));
    EXPECT_FALSE(pure.contains("/"));
}

TEST(DesktopFileTest, DesktopDisplayNamePrefersGenericNameForDeepin)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path,
                 "[Desktop Entry]\nName=MyApp\nGenericName=Generic App\n"
                 "X-Deepin-Vendor=deepin\n");

    DesktopFile df(path);
    EXPECT_EQ(df.desktopDisplayName(), QString("Generic App"));
}

TEST(DesktopFileTest, DesktopDisplayNameFallsBackToLocalName)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path, "[Desktop Entry]\nName=MyApp\n");

    DesktopFile df(path);
    EXPECT_EQ(df.desktopDisplayName(), df.desktopLocalName());
}

TEST(DesktopFileTest, CategoriesEmptyWhenAbsent)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeDesktop(path, "[Desktop Entry]\nName=N\n");

    DesktopFile df(path);
    EXPECT_TRUE(df.desktopCategories().isEmpty());
}
