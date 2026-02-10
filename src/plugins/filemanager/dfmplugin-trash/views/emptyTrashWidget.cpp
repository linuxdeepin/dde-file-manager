// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emptyTrashWidget.h"

#include <DHorizontalLine>
#include <DPaletteHelper>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_trash;

EmptyTrashWidget::EmptyTrashWidget(QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(42);
    this->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *emptyTrashLayout = new QHBoxLayout;
    QLabel *trashLabel = new QLabel(this);

    trashLabel->setText(tr("Trash"));
    QFont f = trashLabel->font();
    f.setPixelSize(16);
    trashLabel->setFont(f);
    QPushButton *emptyTrashButton = new QPushButton;
    emptyTrashButton->setContentsMargins(0, 0, 0, 0);
    emptyTrashButton->setObjectName("EmptyTrashButton");

    emptyTrashButton->setText(tr("Empty"));
    emptyTrashButton->setToolTip(tr("Empty Trash"));
    emptyTrashButton->setFixedSize({ 86, 30 });
    DPalette pal = DPaletteHelper::instance()->palette(this);
    QPalette buttonPalette = emptyTrashButton->palette();
    buttonPalette.setColor(QPalette::ButtonText, pal.color(DPalette::Active, DPalette::TextWarning));
    emptyTrashButton->setPalette(buttonPalette);

    QObject::connect(emptyTrashButton, &QPushButton::clicked, this, &EmptyTrashWidget::emptyTrash);

    QPalette pa = emptyTrashButton->palette();
    pa.setColor(QPalette::ColorRole::Text, QColor("#FF5736"));
    emptyTrashButton->setPalette(pa);
    emptyTrashLayout->addSpacing(11);
    emptyTrashLayout->addWidget(trashLabel, 0, Qt::AlignLeft);
    emptyTrashLayout->addWidget(emptyTrashButton, 0, Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(emptyTrashLayout);
    mainLayout->setContentsMargins(10, 6, 10, 6);

    this->setLayout(mainLayout);
}
