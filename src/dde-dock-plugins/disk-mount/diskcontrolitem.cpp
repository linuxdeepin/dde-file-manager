/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "diskcontrolitem.h"

#include <DGuiApplicationHelper>
#include <QVBoxLayout>
/*!
 * \class DiskControlItem
 *
 * \brief DiskControlItem is an item on the DiskControlWidget
 */

DWIDGET_USE_NAMESPACE

DiskControlItem::DiskControlItem(QFrame *parent)
    : QFrame(parent),
      unknowIcon(":/icons/resources/unknown.svg"),
      diskIcon(new QPushButton(this)),
      diskName(new QLabel),
      diskCapacity(new QLabel),
      capacityValueBar(new QProgressBar),
      unmountButton(new DIconButton(this))
{
    setObjectName("DiskItem");

    initializeUi();
    initConnection();
}

QString DiskControlItem::tagName() const
{
    return curTagName;
}

void DiskControlItem::setTagName(const QString &tagName)
{
    curTagName = tagName;
}

void DiskControlItem::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
}

void DiskControlItem::showEvent(QShowEvent *e)
{
    QFrame::showEvent(e);
}

void DiskControlItem::initializeUi()
{
    diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    diskName->setTextFormat(Qt::PlainText);

    int colorValue = Dtk::Gui::DGuiApplicationHelper::instance()->themeType()
            == Dtk::Gui::DGuiApplicationHelper::LightType
            ? 0
            : 1;

    QFont f = diskName->font();
    f.setPixelSize(14);
    f.setWeight(QFont::Medium);
    diskName->setFont(f);
    QPalette pal = diskName->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.8));
    diskName->setPalette(pal);

    f = diskCapacity->font();
    f.setPixelSize(12);
    f.setWeight(QFont::Normal);
    diskCapacity->setFont(f);
    pal = diskCapacity->palette();
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorValue, colorValue, colorValue, 0.6));
    diskCapacity->setPalette(pal);

    capacityValueBar->setTextVisible(false);
    capacityValueBar->setFixedHeight(2);

    unmountButton->setFixedSize(20, 20);
    unmountButton->setIconSize({20, 20});
    unmountButton->setFlat(true);

    QVBoxLayout *leftLay = new QVBoxLayout;
    leftLay->addWidget(diskIcon);
    leftLay->setContentsMargins(10, 8, 0, 8);
    leftLay->setSpacing(0);

    QWidget *info = new QWidget(this);
    QVBoxLayout *centLay = new QVBoxLayout;
    QVBoxLayout *subCentLay = new QVBoxLayout;
    subCentLay->setSpacing(2);
    subCentLay->setContentsMargins(0, 0, 0, 0);
    subCentLay->addWidget(diskName);
    subCentLay->addWidget(diskCapacity);
    centLay->addItem(subCentLay);
    centLay->addWidget(capacityValueBar);
    info->setLayout(centLay);
    centLay->setSpacing(5);
    centLay->setContentsMargins(10, 11, 0, 10);

    QVBoxLayout *rigtLay = new QVBoxLayout;
    rigtLay->addWidget(unmountButton);
    rigtLay->setContentsMargins(19, 22, 12, 22);

    QHBoxLayout *mainLay = new QHBoxLayout;
    mainLay->addLayout(leftLay);
    mainLay->addWidget(info);
    mainLay->addLayout(rigtLay);
    mainLay->setContentsMargins(10, 8, 8, 12);
    mainLay->setMargin(0);
    mainLay->setSpacing(0);
    setLayout(mainLay);

    // TODO(zhangs): hide unmountButton according dfmsetting

    diskIcon->setFlat(true);
    // TODO(zhangs): diskIcon->setIcon by requist server get icon name
    diskIcon->setIconSize(QSize(48, 48));
    diskIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    diskIcon->setStyleSheet("padding: 0;");
    diskName->setText(QStringLiteral("OwO")); // blumia: correct text should be set in DiskControlItem::showEvent()
    capacityValueBar->setMinimum(0);
    capacityValueBar->setMaximum(100);

    refreshIcon();
}

void DiskControlItem::initConnection()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DiskControlItem::refreshIcon);
}

void DiskControlItem::refreshIcon()
{
    unmountButton->setIcon(QIcon::fromTheme("dfm_unmount"));
}

QString DiskControlItem::sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}
