// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithwidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/mimetype/mimesappsmanager.h>

#include <DRadioButton>
#include <DFontSizeManager>

#include <QCheckBox>
#include <QVBoxLayout>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_utils;

OpenWithWidget::OpenWithWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    MimesAppsManager::instance()->initMimeTypeApps();
    initUI();
}

void OpenWithWidget::selectFileUrl(const QUrl &url)
{
    currentFileUrl = url;
}

void OpenWithWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("Open with")));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6, QFont::DemiBold);

    setExpand(false);

    openWithListWidget = new QListWidget(this);
    openWithListWidget->setSpacing(8);
    openWithListWidget->setObjectName("OpenWithListWidget");
    openWithListWidget->setFrameShape(QFrame::HLine);
    openWithListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    openWithListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    openWithListWidget->setFixedWidth(300);
    DFontSizeManager::instance()->bind(openWithListWidget, DFontSizeManager::SizeType::T7, QFont::Normal);

    openWithBtnGroup = new QButtonGroup(openWithListWidget);

    this->setContent(openWithListWidget);

    connect(openWithBtnGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(openWithBtnChecked(QAbstractButton *)));
    connect(this, &OpenWithWidget::expandChange, this, &OpenWithWidget::slotExpandChange);
}

void OpenWithWidget::openWithBtnChecked(QAbstractButton *btn)
{
    if (btn) {
        MimesAppsManager::instance()->setDefautlAppForTypeByGio(btn->property("mimeTypeName").toString(),
                                                                btn->property("appPath").toString());
    }
}

void OpenWithWidget::slotExpandChange(bool state)
{
    if (state && currentFileUrl.isValid()) {
        FileInfoPointer fileinfo = InfoFactory::create<FileInfo>(currentFileUrl);
        if (fileinfo.isNull())
            return;

        openWithListWidget->clear();

        QMimeType mimeType = fileinfo->fileMimeType();
        const QString &defaultApp = MimesAppsManager::instance()->getDefaultAppByMimeType(mimeType);
        const QStringList &recommendApps = MimesAppsManager::instance()->getRecommendedAppsByQio(mimeType);
        foreach (const QString &appFile, recommendApps) {
            if (!QFile::exists(appFile)) {
                continue;
            }
            const DesktopFile &desktopInfo = MimesAppsManager::instance()->DesktopObjs.value(appFile);

            QListWidgetItem *item = new QListWidgetItem;

            DRadioButton *itemBox = new DRadioButton(desktopInfo.desktopDisplayName());
            itemBox->setObjectName("OpenWithItem");
            itemBox->setIcon(QIcon::fromTheme(desktopInfo.desktopIcon()));
            itemBox->setIconSize(QSize(16, 16));
            itemBox->setProperty("appPath", appFile);
            itemBox->setProperty("mimeTypeName", mimeType.name());
            itemBox->setFocusPolicy(Qt::NoFocus);
            openWithBtnGroup->addButton(itemBox);
            item->setData(Qt::UserRole, desktopInfo.desktopName());
            item->setFlags(Qt::NoItemFlags);
            openWithListWidget->addItem(item);
            openWithListWidget->setItemWidget(item, itemBox);

            if (!defaultApp.isEmpty() && appFile.endsWith(defaultApp)) {
                itemBox->setChecked(true);
            }
        }

        int listHeight = 0;
        int count = openWithListWidget->count();
        for (int i = 0; i < count; i++) {
            QListWidgetItem *item = openWithListWidget->item(i);
            int h = openWithListWidget->itemWidget(item)->height();
            item->setSizeHint(QSize(item->sizeHint().width(), h));
            // 乘以2是因为item与item之间有两个spacing
            listHeight += h + openWithListWidget->spacing() * 2;
        }
        // 加上最后一个spacing
        listHeight += openWithListWidget->spacing();

        // 修复UI-BUG-48789 自动设置listwidget的高度，使得根据内容延展其面板的长度
        if (count < 1) {
            // 当没有打开方式时，设置一个固定大小
            openWithListWidget->setFixedHeight(30);
        } else {
            openWithListWidget->setFixedHeight(listHeight);
        }
    }
}
