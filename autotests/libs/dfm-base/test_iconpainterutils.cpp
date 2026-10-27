// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconpainterutils.cpp
 * @brief 回归测试：AppImage/UAB 缩略图不显示（commit ad83a888d 修复的缺陷）
 *
 * 缺陷根因：isThumbnailIcon() 曾对"跳过底板"的类型（AppImage/UAB）直接
 * 返回 false，而视图委托又复用该标志选择图标来源（isThumbnail ?
 * opt.icon : fileIcon），导致这类文件的缩略图永远画不出来。
 *
 * 因此必须守住两条不变量：
 * 1. 帧跳过类型有缩略图时，isThumbnailIcon() 必须返回 true
 *    （底板跳过由 shouldSkipThumbnailFrame 单独表达，两者不可合并）；
 * 2. skipFrame 只抑制底板/阴影绘制，不能吞掉缩略图本身。
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/iconpainterutils.h>

using namespace dfmbase;

class IconPainterUtilsTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
    }

    FileInfoPointer createFileInfo(const QString &fileName, const QByteArray &content)
    {
        const QString &filePath = tmpDir.filePath(fileName);
        QFile f(filePath);
        if (!f.open(QIODevice::WriteOnly))
            return nullptr;
        f.write(content);
        f.close();
        return InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    }

    // 最小可被识别的 AppImage：ELF magic + offset 8 处的 "AI\x02"
    static QByteArray minimalAppImageContent()
    {
        QByteArray content(1024, '\0');
        content[0] = 0x7f;
        content[1] = 'E';
        content[2] = 'L';
        content[3] = 'F';
        content[8] = 'A';
        content[9] = 'I';
        content[10] = 0x02;
        return content;
    }

    static QIcon solidIcon(const QColor &color)
    {
        QPixmap pm(64, 64);
        pm.fill(color);
        return QIcon(pm);
    }

    // 统计画布上"非图标本体"的不透明像素（即底板/阴影）。
    // 图标本体固定为纯红色方块，据此区分内容与底板。
    // 注意必须用 pixelColor()：QColor(QRgb) 会丢弃 alpha 通道（Qt 固定填 0xff），
    // 那样透明背景会全部算成"底板"，判定恒定失真。
    static int countPlatePixels(const QImage &canvas)
    {
        int count = 0;
        for (int y = 0; y < canvas.height(); ++y) {
            for (int x = 0; x < canvas.width(); ++x) {
                const QColor c = canvas.pixelColor(x, y);
                if (c.alpha() < 40)
                    continue;   // 透明背景
                if (c.red() > 200 && c.green() < 80 && c.blue() < 80)
                    continue;   // 红色图标本体
                ++count;
            }
        }
        return count;
    }

    QTemporaryDir tmpDir;

private:
    static std::once_flag flag;
};

std::once_flag IconPainterUtilsTest::flag;

/*
 * 回归用例：AppImage 有缩略图时必须判定为"有缩略图"。
 * 修复前 isThumbnailIcon() 因帧跳过排除直接返回 false，
 * 视图随之改画 mime 图标，用户看到的现象就是缩略图不显示。
 */
TEST_F(IconPainterUtilsTest, IsThumbnailIconAppImageWithThumbnailReturnsTrue)
{
    auto info = createFileInfo("sample.AppImage", minimalAppImageContent());
    ASSERT_NE(info, nullptr);
    // 前置条件：确认环境能将魔数文件识别为 AppImage，否则用例无意义
    ASSERT_EQ(info->nameOf(FileInfo::FileNameInfoType::kMimeTypeName),
              QString(Global::Mime::kTypeAppAppimage))
            << "shared-mime-info 未识别 AppImage 魔数，请检查测试环境";

    info->setExtendedAttributes(FileInfo::FileExtendedInfoType::kFileThumbnail, solidIcon(Qt::red));

    EXPECT_TRUE(IconPainterUtils::isThumbnailIcon(info));
}

/*
 * 不变量：同一文件"有缩略图"与"跳过底板"必须同时成立，
 * 两个判定互相独立。任何一方吞掉另一方都会复活本缺陷。
 */
TEST_F(IconPainterUtilsTest, ThumbnailAndFrameSkipAreIndependentForAppImage)
{
    auto info = createFileInfo("independent.AppImage", minimalAppImageContent());
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->nameOf(FileInfo::FileNameInfoType::kMimeTypeName),
              QString(Global::Mime::kTypeAppAppimage));

    info->setExtendedAttributes(FileInfo::FileExtendedInfoType::kFileThumbnail, solidIcon(Qt::green));

    EXPECT_TRUE(IconPainterUtils::isThumbnailIcon(info));
    EXPECT_TRUE(IconPainterUtils::shouldSkipThumbnailFrame(info));
}

TEST_F(IconPainterUtilsTest, ShouldSkipThumbnailFramePlainFileReturnsFalse)
{
    auto info = createFileInfo("plain.txt", "hello world");
    ASSERT_NE(info, nullptr);

    EXPECT_FALSE(IconPainterUtils::shouldSkipThumbnailFrame(info));
}

TEST_F(IconPainterUtilsTest, IsThumbnailIconWithoutThumbnailReturnsFalse)
{
    auto info = createFileInfo("nothumb.txt", "hello world");
    ASSERT_NE(info, nullptr);

    // 未生成（QIcon() 占位）与从未请求（无效 QVariant）都视为无缩略图
    info->setExtendedAttributes(FileInfo::FileExtendedInfoType::kFileThumbnail, QIcon());
    EXPECT_FALSE(IconPainterUtils::isThumbnailIcon(info));
}

TEST_F(IconPainterUtilsTest, NullInfoReturnsFalseForBothChecks)
{
    EXPECT_FALSE(IconPainterUtils::isThumbnailIcon(FileInfoPointer()));
    EXPECT_FALSE(IconPainterUtils::shouldSkipThumbnailFrame(FileInfoPointer()));
}

/*
 * 底板行为：isThumb=true 且 skipFrame=false 时应绘制白色底板/阴影，
 * 画布上存在非图标内容的不透明像素。
 */
TEST_F(IconPainterUtilsTest, PaintIconThumbnailWithFrameDrawsPlate)
{
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    IconPainterUtils::PaintIconOpts opts;
    opts.rect = QRectF(16, 16, 64, 64);
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.isThumb = true;
    opts.viewMode = Global::ViewMode::kIconMode;
    opts.skipFrame = false;

    const auto &painted = IconPainterUtils::paintIcon(&painter, solidIcon(Qt::red), opts);
    painter.end();

    ASSERT_TRUE(painted.has_value());
    EXPECT_GT(countPlatePixels(canvas), 0);
}

/*
 * 回归用例：skipFrame=true（AppImage/UAB 路径）时画布上只能有图标
 * 本体，不允许出现底板/阴影像素。
 */
TEST_F(IconPainterUtilsTest, PaintIconThumbnailSkipFrameSuppressesPlate)
{
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    IconPainterUtils::PaintIconOpts opts;
    opts.rect = QRectF(16, 16, 64, 64);
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.isThumb = true;
    opts.viewMode = Global::ViewMode::kIconMode;
    opts.skipFrame = true;

    const auto &painted = IconPainterUtils::paintIcon(&painter, solidIcon(Qt::red), opts);
    painter.end();

    ASSERT_TRUE(painted.has_value());
    EXPECT_EQ(countPlatePixels(canvas), 0);
}

/*
 * 缩略图为空时 paintIcon 必须返回 nullopt，
 * 供调用方回退到 kItemFileIconRole（文件类型图标）。
 */
TEST_F(IconPainterUtilsTest, PaintIconThumbnailNullPixmapReturnsNullopt)
{
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    IconPainterUtils::PaintIconOpts opts;
    opts.rect = QRectF(16, 16, 64, 64);
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.isThumb = true;
    opts.viewMode = Global::ViewMode::kIconMode;
    opts.skipFrame = true;

    const auto &painted = IconPainterUtils::paintIcon(&painter, QIcon(), opts);
    painter.end();

    EXPECT_FALSE(painted.has_value());
}
