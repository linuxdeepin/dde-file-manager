// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_deviceitem.cpp - DeviceItem（含 IconLabel）单元测试
// QWidget 离屏渲染；打开设备/音效等外部副作用全部打桩。

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QtTest>
#include <QLabel>
#include <QProgressBar>
#include <QMouseEvent>
#include <QFrame>
#include <QStandardPaths>

#include <DDesktopServices>
#include <DIconButton>
#include <DGuiApplicationHelper>
#include <DConfig>

#include "stubext.h"

#include "deviceitem.h"
#include "dockutils.h"
#include "global_server_defines.h"

using namespace GlobalServerDefines;
using namespace Dtk::Gui;
using namespace Dtk::Widget;
using namespace Dtk::Core;   // DConfig

namespace {

// initUI 中 nameLabel 先于 sizeLabel 创建，再以文本特征兜底定位
QLabel *findNameLabel(QWidget *host)
{
    const auto labels = host->findChildren<QLabel *>();
    for (QLabel *l : labels)
        if (l->text().contains("UT-Disk"))
            return l;
    return labels.isEmpty() ? nullptr : labels.constFirst();
}

// IconLabel 是 .cpp 内私有类且无 Q_OBJECT（className 恒为 QWidget），
// 以其独有特征定位：48x48 固定尺寸 + 鼠标事件穿透
QWidget *findIconLabel(QWidget *host)
{
    const auto children = host->findChildren<QWidget *>();
    for (QWidget *w : children) {
        if (w->width() == 48 && w->height() == 48
                && w->testAttribute(Qt::WA_TransparentForMouseEvents))
            return w;
    }
    return nullptr;
}

DockItemData makeItemData()
{
    return {
        .id = QString("/org/freedesktop/UDisks2/block_devices/utblk"),
        .backingID = QString("/org/freedesktop/UDisks2/block_devices/utback"),
        .displayName = QString("UT-Disk"),
        .targetUrl = QUrl::fromLocalFile("/media/utblk"),
        .targetFileUrl = QUrl::fromLocalFile("/media/utblk"),
        .iconName = QString("drive-removable-media-usb"),
        .totalSize = 2048ULL * 1024 * 1024,
        .usedSize = 512ULL * 1024 * 1024,
        .sortKey = QString("00x_00UT-Disk"),
        .isProtocolDevice = false,
    };
}

}   // namespace

class DeviceItemTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // DConfig 缺省 → isIntegratedByFilemanager 恒 true，保证鼠标分支确定
        stub.set_lamda(static_cast<DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(
                               &DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *) -> DConfig * {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
    }

    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// 构造 + initUI：布局元素齐备，宽度锁定插件宽度
TEST_F(DeviceItemTest, Construct_BuildsUiWithFixedWidth)
{
    // Arrange / Act
    DeviceItem item(makeItemData());

    // Assert
    EXPECT_EQ(item.width(), kDockPluginWidth);
    EXPECT_FALSE(item.findChildren<QLabel *>().isEmpty());
    EXPECT_FALSE(item.findChildren<QProgressBar *>().isEmpty());
    EXPECT_EQ(item.findChildren<DIconButton *>().count(), 1);
    // IconLabel（.cpp 内私有类）存在（以 48x48 + 鼠标穿透特征识别）
    EXPECT_NE(findIconLabel(&item), nullptr);
    // grab() 触发 IconLabel::paintEvent 离屏绘制
    EXPECT_FALSE(item.grab().isNull());
}

// updateUsage：超界值收敛到总量；文本与进度条同步
TEST_F(DeviceItemTest, UpdateUsage_ClampsAndRefreshesLabelAndProgress)
{
    // Arrange
    DeviceItem item(makeItemData());

    // Act
    item.updateUsage(1024ULL * 1024 * 1024);

    // Assert
    QProgressBar *bar = item.findChild<QProgressBar *>();
    ASSERT_NE(bar, nullptr);
    EXPECT_EQ(bar->value(), 50);   // 1GB / 2GB
    EXPECT_EQ(item.data.usedSize, 1024ULL * 1024 * 1024);

    // Act：超过总量 → 收敛到 100%
    item.updateUsage(9999ULL * 1024 * 1024 * 1024);
    EXPECT_EQ(bar->value(), 100);
    EXPECT_EQ(item.data.usedSize, 2048ULL * 1024 * 1024);
}

// createSeparateLine：静态工厂产出的分割线属性
TEST_F(DeviceItemTest, CreateSeparateLine_PropertiesConfigured)
{
    // Arrange / Act
    QFrame *line = DeviceItem::createSeparateLine(3);

    // Assert
    ASSERT_NE(line, nullptr);
    EXPECT_EQ(line->lineWidth(), 3);
    EXPECT_EQ(line->height(), 3);
    EXPECT_EQ(line->frameStyle(), QFrame::HLine | QFrame::Plain);
    delete line;
}

// mouseReleaseEvent：左键 + 集成开启 → 打开设备（targetUrl）
TEST_F(DeviceItemTest, MouseRelease_LeftButtonIntegrated_OpensTargetUrl)
{
    // Arrange
    DeviceItem item(makeItemData());
    QList<QUrl> shown;
    stub.set_lamda(static_cast<bool (*)(const QUrl &, const QString &)>(&DDesktopServices::showFolder),
                   [&shown](const QUrl &url, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        shown << url;
        return true;
    });
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [] { __DBG_STUB_INVOKE__ return QString("/usr/bin/dde-file-manager"); });

    QMouseEvent left(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5),
                     Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    // Act
    item.mouseReleaseEvent(&left);

    // Assert
    ASSERT_EQ(shown.count(), 1);
    EXPECT_EQ(shown.first(), QUrl::fromLocalFile("/media/utblk"));   // 有可执行 → targetUrl
}

// mouseReleaseEvent：非左键不打开；无 dde-file-manager 可执行时用 targetFileUrl
TEST_F(DeviceItemTest, MouseRelease_RightButtonOrNoExec_NoOpenOrFileUrl)
{
    // Arrange
    DeviceItem item(makeItemData());
    QList<QUrl> shown;
    stub.set_lamda(static_cast<bool (*)(const QUrl &, const QString &)>(&DDesktopServices::showFolder),
                   [&shown](const QUrl &url, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        shown << url;
        return true;
    });

    QMouseEvent right(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5),
                      Qt::RightButton, Qt::NoButton, Qt::NoModifier);

    // Act：右键 → 不打开
    item.mouseReleaseEvent(&right);
    EXPECT_EQ(shown.count(), 0);

    // Act：左键但找不到可执行 → 用 targetFileUrl 分支
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [] { __DBG_STUB_INVOKE__ return QString(); });
    QMouseEvent left(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5),
                     Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    item.mouseReleaseEvent(&left);

    // Assert
    ASSERT_EQ(shown.count(), 1);
    EXPECT_EQ(shown.first(), QUrl::fromLocalFile("/media/utblk"));
}

// resizeEvent / showEvent：名称重排不崩溃且文本保留
TEST_F(DeviceItemTest, ResizeAndShow_RefreshDeviceNameSafely)
{
    // Arrange
    DeviceItem item(makeItemData());

    // Act
    item.resize(260, 70);
    item.show();
    QTest::qWait(10);

    // Assert
    QLabel *name = findNameLabel(&item);
    ASSERT_NE(name, nullptr);
    EXPECT_TRUE(name->text().contains("UT-Disk"));
    item.hide();
}

// updateDeviceName：协议设备 smb 别名替换 + ftp scheme 别名替换
TEST_F(DeviceItemTest, UpdateDeviceName_ProtocolAliasesApplied)
{
    // Arrange：smb 分支 —— targetFileUrl 字符串可被 parseSmbInfo 解析
    // （名称刻意极短，规避未布局 nameLabel 宽度下 elidedText 的中间省略）
    DockItemData smbData = makeItemData();
    smbData.isProtocolDevice = true;
    smbData.id = "smb-share";
    smbData.displayName = "on nas1";
    smbData.targetFileUrl = QUrl(",server=nas1,share=photos");
    DeviceItem smbItem(smbData);

    stub.set_lamda(&device_utils::protocolDeviceAlias,
                   [](const QString &, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QStringLiteral("别名");
    });

    // Act
    smbItem.updateDeviceName();

    // Assert：host "nas1" 被别名替换
    QLabel *name = findNameLabel(&smbItem);
    ASSERT_NE(name, nullptr);
    EXPECT_TRUE(name->text().contains("别名"));
    EXPECT_FALSE(name->text().contains("nas1"));

    // Arrange：ftp 分支 —— id 匹配 schemeList
    DockItemData ftpData = makeItemData();
    ftpData.isProtocolDevice = true;
    ftpData.id = "ftp://f.ut/pub";
    ftpData.displayName = "f.ut";
    ftpData.targetFileUrl = QUrl("ftp://f.ut/pub");
    DeviceItem ftpItem(ftpData);

    // Act
    ftpItem.updateDeviceName();

    // Assert：host "f.ut" 被同一别名桩替换
    QLabel *ftpName = findNameLabel(&ftpItem);
    ASSERT_NE(ftpName, nullptr);
    EXPECT_TRUE(ftpName->text().contains("别名"));
    EXPECT_FALSE(ftpName->text().contains("f.ut"));
}

// updateDeviceName：非协议设备名称原样
TEST_F(DeviceItemTest, UpdateDeviceName_BlockDevice_KeepsName)
{
    // Arrange
    DeviceItem item(makeItemData());

    // Act
    item.updateDeviceName();

    // Assert
    QLabel *name = findNameLabel(&item);
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->text(), QString("UT-Disk"));
}

// 弹出按钮：点击 → requestEject(backingID)
TEST_F(DeviceItemTest, EjectButton_Click_EmitsRequestEject)
{
    // Arrange
    DeviceItem item(makeItemData());
    item.show();
    QSignalSpy spy(&item, &DeviceItem::requestEject);
    DIconButton *btn = item.findChild<DIconButton *>();
    ASSERT_NE(btn, nullptr);

    // Act
    btn->click();

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().toString(),
              QString("/org/freedesktop/UDisks2/block_devices/utback"));
}

// 主题切换 lambda：标签调色板按信号参数取对应主题色（与初始主题无关）
TEST_F(DeviceItemTest, ThemeChange_Lambda_UpdatesLabelPalette)
{
    // Arrange
    DeviceItem item(makeItemData());
    QLabel *name = findNameLabel(&item);
    ASSERT_NE(name, nullptr);

    // Act：切浅色（lambda 直接消费信号参数）
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
    const QColor light = name->palette().color(QPalette::WindowText);

    // Act：切深色
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);
    const QColor dark = name->palette().color(QPalette::WindowText);

    // Assert
    EXPECT_EQ(light, QColor::fromRgbF(0, 0, 0, 0.8));
    EXPECT_EQ(dark, QColor::fromRgbF(1, 1, 1, 0.8));
}

// setTextColor：静态工具按主题与 alpha 设置颜色（DarkType=白、LightType=黑）
TEST_F(DeviceItemTest, SetTextColor_ThemeTypes_SelectBaseColor)
{
    // Arrange
    QLabel label;

    // Act：深色主题 → 白色基底
    DeviceItem::setTextColor(&label, DGuiApplicationHelper::DarkType, 0.8);

    // Assert
    EXPECT_EQ(label.palette().color(QPalette::WindowText), QColor::fromRgbF(1, 1, 1, 0.8));

    // Act：浅色主题 → 黑色基底
    DeviceItem::setTextColor(&label, DGuiApplicationHelper::LightType, 0.6);
    EXPECT_EQ(label.palette().color(QPalette::WindowText), QColor::fromRgbF(0, 0, 0, 0.6));
}
