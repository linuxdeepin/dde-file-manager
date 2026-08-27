// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceutils_high.cpp
 * @brief Mode 2 supplement: High-priority DeviceUtils methods.
 *
 * Targets (from .ut-inventory.json, level=high, usecase_count==0):
 *   - DeviceUtils::getLongestMountRootPath
 *   - DeviceUtils::nameOfOptical
 *
 * Branch清单 (declared, cross-checked via MCP get_code_snippet):
 *   getLongestMountRootPath: mnt_table_parse_mtab fail → "/" / loop / sort / startsWith / found
 *   nameOfOptical: label not empty → label / opticalBlank+media → Blank Disc / optical+discMapper
 *
 * Key names (from global_server_defines.h):
 *   kIdLabel="IdLabel"  kSizeTotal="SizeTotal"  kMedia="Media"
 *   kOptical="Optical"  kOpticalBlank="OpticalBlank"
 */

#include <gtest/gtest.h>
#include <QString>
#include <QVariantMap>

#include <dfm-base/base/device/deviceutils.h>

using namespace dfmbase;

// ── getLongestMountRootPath ──

TEST(DeviceUtilsHighTest, GetLongestMountRootPathForRootPath)
{
    // The root "/" should always be a mount point
    QString result = DeviceUtils::getLongestMountRootPath("/");
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, GetLongestMountRootPathForTmpPath)
{
    // /tmp is typically under "/" mount
    QString result = DeviceUtils::getLongestMountRootPath("/tmp");
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, GetLongestMountRootPathForNonExistentPath)
{
    // Even non-existent paths should return something (at least "/")
    QString result = DeviceUtils::getLongestMountRootPath("/nonexistent/path/here");
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, GetLongestMountRootPathReturnsMountPointOrRoot)
{
    // The result should be either "/" or a mount point path
    QString result = DeviceUtils::getLongestMountRootPath("/home");
    EXPECT_TRUE(result.startsWith("/"));
}

TEST(DeviceUtilsHighTest, GetLongestMountRootPathForHomePath)
{
    QString result = DeviceUtils::getLongestMountRootPath("/home/user");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.startsWith("/"));
}

// ── nameOfOptical ──

TEST(DeviceUtilsHighTest, NameOfOpticalWithLabelReturnsLabel)
{
    QVariantMap data;
    data.insert("IdLabel", "My Disc");
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_EQ(result, QString("My Disc"));
}

TEST(DeviceUtilsHighTest, NameOfOpticalEmptyDataReturnsNonEmpty)
{
    QVariantMap data;
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalOpticalCdMedia)
{
    QVariantMap data;
    data.insert("Optical", true);
    data.insert("OpticalBlank", false);
    data.insert("Media", "optical_cd");
    data.insert("SizeTotal", static_cast<qulonglong>(700ULL * 1024 * 1024));
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalBlankDisc)
{
    QVariantMap data;
    data.insert("Optical", true);
    data.insert("OpticalBlank", true);
    data.insert("Media", "optical_cd_r");
    data.insert("SizeTotal", static_cast<qulonglong>(0));
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalDvdMedia)
{
    QVariantMap data;
    data.insert("Optical", true);
    data.insert("OpticalBlank", false);
    data.insert("Media", "optical_dvd");
    data.insert("SizeTotal", static_cast<qulonglong>(4ULL * 1024 * 1024 * 1024));
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalBdMedia)
{
    QVariantMap data;
    data.insert("Optical", true);
    data.insert("OpticalBlank", false);
    data.insert("Media", "optical_bd");
    data.insert("SizeTotal", static_cast<qulonglong>(25ULL * 1024 * 1024 * 1024));
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalNoOpticalFlagReturnsNonEmpty)
{
    QVariantMap data;
    data.insert("Optical", false);
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}

TEST(DeviceUtilsHighTest, NameOfOpticalEmptyLabelReturnsFallback)
{
    QVariantMap data;
    data.insert("IdLabel", "");
    QString result = DeviceUtils::nameOfOptical(data);
    EXPECT_FALSE(result.isEmpty());
}
