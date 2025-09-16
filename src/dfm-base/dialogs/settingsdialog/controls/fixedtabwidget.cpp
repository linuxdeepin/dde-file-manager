// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fixedtabwidget.h"
#include "aliascombobox.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <DToolButton>
#include <DSettingsOption>

#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFileDialog>

#include <functional>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FixedTabWidget::FixedTabWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void FixedTabWidget::setOption(QObject *opt)
{
    auto option = qobject_cast<DSettingsOption *>(opt);
    const auto &itemList = option->value().toStringList();
    if (itemList.size() > 4)
        updateAddItemLabel(false);

    auto defItem = option->defaultValue().toString();
    if (!itemList.isEmpty())
        defItem = itemList.first();

    int index = defItemCB->findData(defItem);
    if (index == -1) {
        index = defItemCB->count() - 1;
        defItemCB->setItemData(index, defItem);
        defItemCB->setItemAlias(index, tr("Specify directory %1").arg(QUrl(defItem).path()));
    }
    lastDefIndex = index;
    defItemCB->setCurrentIndex(index);

    for (int i = 1; i < itemList.size(); ++i) {
        addCustomItem(option, itemList[i]);
    }

    using namespace std::placeholders;
    connect(defItemCB, &AliasComboBox::currentIndexChanged, this, std::bind(&FixedTabWidget::handleDefaultChanged, this, option, _1));
    connect(addItemLabel, &QLabel::linkActivated, this, std::bind(&FixedTabWidget::handleAddCustomItem, this, option));
    connect(option, &DSettingsOption::valueChanged, this, &FixedTabWidget::handleOptionChanged);
}

void FixedTabWidget::initUI()
{
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setColumnStretch(0, 2);
    mainLayout->setColumnStretch(1, 3);
    mainLayout->setVerticalSpacing(10);

    defItemCB = new AliasComboBox(this);
    defItemCB->addItem(tr("Computer"), "computer:///");
    defItemCB->addItem(tr("Home"), "standard://home");
    defItemCB->addItem(tr("Desktop"), "standard://desktop");
    defItemCB->addItem(tr("Videos"), "standard://videos");
    defItemCB->addItem(tr("Music"), "standard://music");
    defItemCB->addItem(tr("Pictures"), "standard://pictures");
    defItemCB->addItem(tr("Documents"), "standard://documents");
    defItemCB->addItem(tr("Downloads"), "standard://downloads");
    defItemCB->addItem(tr("Specify directory"));

    addItemLabel = new QLabel(this);
    addItemLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    updateAddItemLabel(true);

    mainLayout->addWidget(new QLabel(tr("Display when launching a new window:"), this), 0, 0, 1, 2);
    mainLayout->addWidget(new QLabel(tr("Default Directory"), this), 1, 0);
    mainLayout->addWidget(defItemCB, 1, 1);
    mainLayout->addWidget(new QLabel(tr("Custom Directory"), this), 2, 0);
    mainLayout->addWidget(addItemLabel, 2, 1);
}

void FixedTabWidget::addCustomItem(DSettingsOption *opt, const QUrl &url)
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
                    for (int i = 1; i < itemList.size(); ++i) {
                        if (url.toString() == itemList[i]) {
                            itemList.removeAt(i);
                            break;
                        }
                    }
                    opt->setValue(itemList);
                    updateAddItemLabel(itemList.size() < 5);
                }
            });

    QLabel *pathLabel = new QLabel(this);
    const auto &elideText = pathLabel->fontMetrics().elidedText(url.path(), Qt::ElideMiddle, 200);
    if (elideText != url)
        pathLabel->setToolTip(url.path());
    pathLabel->setText(elideText);

    int row = mainLayout->rowCount();
    mainLayout->addWidget(pathLabel, row, 0);
    mainLayout->addWidget(delBtn, row, 1, Qt::AlignRight | Qt::AlignVCenter);
}

QUrl FixedTabWidget::selectCustomDirectory()
{
    const auto &url = QFileDialog::getExistingDirectoryUrl(this);
    if (!ProtocolUtils::isLocalFile(url)) {
        DialogManagerInstance->showErrorDialog(tr("Invalid Directory"),
                                               tr("The directory is unavailable for this operation. "
                                                  "Please select a local directory and try again."));
        return {};
    }

    return url;
}

void FixedTabWidget::handleAddCustomItem(DSettingsOption *opt)
{
    const auto &url = selectCustomDirectory();
    if (!url.isValid())
        return;

    addCustomItem(opt, url);
    auto itemList = opt->value().toStringList();
    itemList.append(url.toString());
    opt->setValue(itemList);

    if (itemList.size() > 4)
        updateAddItemLabel(false);
}

void FixedTabWidget::handleDefaultChanged(DSettingsOption *opt, int index)
{
    if (index == defItemCB->count() - 1) {
        const auto &url = selectCustomDirectory();
        if (!url.isValid()) {
            defItemCB->setCurrentIndex(lastDefIndex);
            return;
        }

        defItemCB->setItemData(index, url.toString());
        defItemCB->setItemAlias(index, tr("Specify directory %1").arg(url.path()));
    }

    lastDefIndex = index;
    auto itemList = opt->value().toStringList();
    itemList.replace(0, defItemCB->itemData(index).toString());
    opt->setValue(itemList);
}

void FixedTabWidget::handleOptionChanged(const QVariant &value)
{
    auto opt = qobject_cast<DSettingsOption *>(sender());
    if (!opt)
        return;

    clearCustomItems();
    const auto &itemList = value.toStringList();
    if (!itemList.isEmpty()) {
        int index = defItemCB->findData(itemList.first());
        if (index == -1) {
            index = defItemCB->count() - 1;
            defItemCB->setItemData(index, itemList.first());
            defItemCB->setItemAlias(index, tr("Specify directory %1").arg(itemList.first()));
        }
        lastDefIndex = index;
        defItemCB->setCurrentIndex(index);

        for (int i = 1; i < itemList.size(); ++i) {
            addCustomItem(opt, itemList[i]);
        }
    }
    updateAddItemLabel(itemList.size() < 5);
}

void FixedTabWidget::updateAddItemLabel(bool enable)
{
    QString color = enable ? "#0082fa" : "#c7ddf7";
    QString format = "<a href='Add'><span style=' text-decoration: none; color:%1;'>%2</span></a>";

    addItemLabel->setText(format.arg(color, tr("Add Directory")));
    addItemLabel->setEnabled(enable);
}

void FixedTabWidget::clearCustomItems()
{
    for (int i = mainLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = mainLayout->itemAt(i);
        if (!item) continue;

        auto w = item->widget();
        if (w && w->objectName() == "DeleteButton")
            removeRow(w);
    }
}

bool FixedTabWidget::removeRow(QWidget *w)
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
