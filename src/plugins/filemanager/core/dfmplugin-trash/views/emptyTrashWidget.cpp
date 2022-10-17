/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "emptyTrashWidget.h"

#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_trash;

EmptyTrashWidget::EmptyTrashWidget(QWidget *parent)
    : QFrame(parent)
{

    this->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *emptyTrashLayout = new QHBoxLayout;
    QLabel *trashLabel = new QLabel(this);

    trashLabel->setText(tr("Trash"));
    QFont f = trashLabel->font();
    f.setPixelSize(17);
    f.setBold(true);
    trashLabel->setFont(f);
    QPushButton *emptyTrashButton = new QPushButton;
    emptyTrashButton->setContentsMargins(0, 0, 0, 0);
    emptyTrashButton->setObjectName("EmptyTrashButton");

    emptyTrashButton->setText(tr("Empty"));
    emptyTrashButton->setToolTip(tr("Empty Trash"));
    emptyTrashButton->setFixedSize({ 86, 36 });
    DPalette pal = DApplicationHelper::instance()->palette(this);
    QPalette buttonPalette = emptyTrashButton->palette();
    buttonPalette.setColor(QPalette::ButtonText, pal.color(DPalette::Active, DPalette::TextWarning));
    emptyTrashButton->setPalette(buttonPalette);

    QObject::connect(emptyTrashButton, &QPushButton::clicked, this, &EmptyTrashWidget::emptyTrash);

    QPalette pa = emptyTrashButton->palette();
    pa.setColor(QPalette::ColorRole::Text, QColor("#FF5736"));
    emptyTrashButton->setPalette(pa);
    emptyTrashLayout->addWidget(trashLabel, 0, Qt::AlignLeft);
    emptyTrashLayout->addWidget(emptyTrashButton, 0, Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(emptyTrashLayout);
    mainLayout->setContentsMargins(10, 8, 10, 8);

    this->setLayout(mainLayout);
}
