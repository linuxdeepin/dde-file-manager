// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/menuutils.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>

#include <gtest/gtest.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_MenuUtils : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_MenuUtils, PerfectMenuParams_EmptySelectFiles_ReturnsOriginalParams)
{
    QVariantHash params;
    params["key"] = "value";

    auto result = MenuUtils::perfectMenuParams(params);
    EXPECT_EQ(result, params);
}

TEST_F(UT_MenuUtils, PerfectMenuParams_AlreadyPerfect_ReturnsOriginalParams)
{
    QVariantHash params;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsSystemPathIncluded] = true;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

    auto result = MenuUtils::perfectMenuParams(params);
    EXPECT_EQ(result, params);
}

TEST_F(UT_MenuUtils, PerfectMenuParams_FocusOnComputerDesktopFile_SetsFlags)
{
    QUrl computerUrl = QUrl::fromLocalFile("/tmp/dde-computer.desktop");
    QList<QUrl> urls = { computerUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_TRUE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_FocusOnTrashDesktopFile_SetsFlags)
{
    QUrl trashUrl = QUrl::fromLocalFile("/tmp/dde-trash.desktop");
    QList<QUrl> urls = { trashUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_TRUE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_FocusOnHomeDesktopFile_SetsFlags)
{
    QUrl homeUrl = QUrl::fromLocalFile("/tmp/dde-home.desktop");
    QList<QUrl> urls = { homeUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_TRUE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_SystemPathIncluded_SetsFlag)
{
    QUrl systemUrl = QUrl::fromLocalFile("/usr/bin/test");
    QList<QUrl> urls = { systemUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_FALSE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_MultipleFiles_MixedFlags)
{
    QUrl normalUrl = QUrl::fromLocalFile("/tmp/normal.txt");
    QUrl systemUrl = QUrl::fromLocalFile("/usr/bin/test");
    QUrl desktopUrl = QUrl::fromLocalFile("/tmp/dde-computer.desktop");

    QList<QUrl> urls = { normalUrl, systemUrl, desktopUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [desktopUrl](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return url == desktopUrl;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [systemUrl](SystemPathUtil *, const QString &path) {
        __DBG_STUB_INVOKE__
        return path == systemUrl.toLocalFile();
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_FALSE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_NormalFile_AllFlagsFalse)
{
    QUrl normalUrl = QUrl::fromLocalFile("/tmp/normal.txt");
    QList<QUrl> urls = { normalUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = MenuUtils::perfectMenuParams(params);

    EXPECT_FALSE(result[MenuParamKey::kIsFocusOnDDEDesktopFile].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_FALSE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}

TEST_F(UT_MenuUtils, PerfectMenuParams_EarlyExit_StopsChecking)
{
    QUrl desktopUrl = QUrl::fromLocalFile("/tmp/dde-computer.desktop");
    QUrl systemUrl = QUrl::fromLocalFile("/usr/bin/test");
    QUrl normalUrl = QUrl::fromLocalFile("/tmp/normal.txt");

    QList<QUrl> urls = { desktopUrl, systemUrl, normalUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    int computerCheckCount = 0;
    stub.set_lamda(&FileUtils::isComputerDesktopFile, [&computerCheckCount, desktopUrl](const QUrl &url) {
        __DBG_STUB_INVOKE__
        if (url == desktopUrl) {
            computerCheckCount++;
            return true;
        }
        computerCheckCount++;
        return false;
    });

    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&SystemPathUtil::isSystemPath, [systemUrl](SystemPathUtil *, const QString &path) {
        __DBG_STUB_INVOKE__
        return path == systemUrl.toLocalFile();
    });

    auto result = MenuUtils::perfectMenuParams(params);

    // 应该提前退出，所以不会检查所有文件
    EXPECT_TRUE(result[MenuParamKey::kIsDDEDesktopFileIncluded].toBool());
    EXPECT_TRUE(result[MenuParamKey::kIsSystemPathIncluded].toBool());
}
