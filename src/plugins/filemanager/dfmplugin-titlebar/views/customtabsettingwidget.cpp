// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customtabsettingwidget.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <DLabel>
#include <DToolButton>
#include <DSettingsOption>
#include <DStyle>

#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>

#include <functional>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace dfmplugin_titlebar;

CustomTabSettingWidget::CustomTabSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void CustomTabSettingWidget::setOption(QObject *opt)
{
    auto option = qobject_cast<DSettingsOption *>(opt);
    const auto &itemList = option->value().toStringList();
    addItemBtn->setEnabled(itemList.size() < 4);

    for (const auto &item : itemList) {
        addCustomItem(option, item);
    }

    using namespace std::placeholders;
    connect(addItemBtn, &DToolButton::clicked, this, std::bind(&CustomTabSettingWidget::handleAddCustomItem, this, option));
    connect(option, &DSettingsOption::valueChanged, this, &CustomTabSettingWidget::handleOptionChanged);
}

void CustomTabSettingWidget::initUI()
{
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setVerticalSpacing(10);

    addItemBtn = new DToolButton(this);
    addItemBtn->setToolTip(tr("Add Directory"));
    addItemBtn->setIconSize({ 16, 16 });
    addItemBtn->setIcon(DStyle::standardIcon(style(), DStyle::SP_IncreaseElement));
    addItemBtn->setEnabled(true);

    mainLayout->addWidget(new QLabel(tr("Custom Directory"), this), 0, 0);
    mainLayout->addWidget(addItemBtn, 0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 0);
}

void CustomTabSettingWidget::addCustomItem(DSettingsOption *opt, const QUrl &url)
{
    DToolButton *delBtn = new DToolButton(this);
    delBtn->setIcon(QIcon::fromTheme("trash"));
    delBtn->setIconSize({ 16, 16 });
    delBtn->setFixedSize(32, 32);
    delBtn->setObjectName("DeleteButton");
    connect(delBtn, &DToolButton::clicked, this,
            [=] {
                if (removeRow(delBtn)) {
                    auto itemList = opt->value().toStringList();
                    itemList.removeOne(url.toString());
                    opt->setValue(itemList);
                    addItemBtn->setEnabled(itemList.size() < 4);
                }
            });

    DLabel *pathLabel = new DLabel(this);
    pathLabel->setElideMode(Qt::ElideMiddle);
    pathLabel->setToolTip(url.path());
    pathLabel->setText(url.path());

    int row = mainLayout->rowCount();
    mainLayout->addWidget(pathLabel, row, 0);
    mainLayout->addWidget(delBtn, row, 1, Qt::AlignRight | Qt::AlignVCenter);
}

QUrl CustomTabSettingWidget::selectCustomDirectory()
{
    const auto &url = QFileDialog::getExistingDirectoryUrl(this);
    if (!url.isValid())
        return {};

    if (!ProtocolUtils::isLocalFile(url)) {
        DialogManagerInstance->showErrorDialog(tr("Invalid Directory"),
                                               tr("This directory does not support pinning"));
        return {};
    }

    return url;
}

void CustomTabSettingWidget::handleAddCustomItem(Dtk::Core::DSettingsOption *opt)
{
    const auto &url = selectCustomDirectory();
    if (!url.isValid())
        return;

    addCustomItem(opt, url);
    auto itemList = opt->value().toStringList();
    itemList.append(url.toString());
    opt->setValue(itemList);

    if (itemList.size() > 3)
        addItemBtn->setEnabled(false);
}

void CustomTabSettingWidget::handleOptionChanged(const QVariant &value)
{
    auto opt = qobject_cast<DSettingsOption *>(sender());
    if (!opt)
        return;

    clearCustomItems();
    const auto &itemList = value.toStringList();
    if (!itemList.isEmpty()) {
        for (const auto &item : itemList) {
            addCustomItem(opt, item);
        }
    }
    addItemBtn->setEnabled(itemList.size() < 4);
}

void CustomTabSettingWidget::clearCustomItems()
{
    for (int i = mainLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = mainLayout->itemAt(i);
        if (!item) continue;

        auto w = item->widget();
        if (w && w->objectName() == "DeleteButton")
            removeRow(w);
    }
}

bool CustomTabSettingWidget::removeRow(QWidget *w)
{
    int index = mainLayout->indexOf(w);   // 获取控件在布局中的索引
    if (index == -1)
        return false;

    int row, column, rowSpan, columnSpan;
    mainLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);

    for (int i = mainLayout->count() - 1; i >= 0; i--) {
        int r, c, rowSpan, colSpan;
        QLayoutItem *item = mainLayout->itemAt(i);
        if (!item) continue;

        // 获取当前项的位置信息
        mainLayout->getItemPosition(i, &r, &c, &rowSpan, &colSpan);
        if (r == row) {
            item = mainLayout->takeAt(i);
            QWidget *widget = item->widget();
            if (widget)
                delete widget;
            delete item;   // 删除布局项本身
        }
    }

    // 重置该行的属性
    mainLayout->setRowMinimumHeight(row, 0);
    mainLayout->setRowStretch(row, 0);
    return true;
}
