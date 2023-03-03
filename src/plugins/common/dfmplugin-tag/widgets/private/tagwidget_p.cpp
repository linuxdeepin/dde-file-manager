// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagwidget_p.h"
#include "widgets/tagwidget.h"
#include "widgets/tagcolorlistwidget.h"
#include "widgets/tagcrumbedit.h"

#include <QLabel>
#include <QVBoxLayout>

using namespace dfmplugin_tag;

TagWidgetPrivate::TagWidgetPrivate(TagWidget *qq, const QUrl &url)
    : url(url),
      q(qq)
{
}

TagWidgetPrivate::~TagWidgetPrivate()
{
}

void TagWidgetPrivate::initializeUI()
{
    mainLayout = new QVBoxLayout(q);
    q->setLayout(mainLayout);

    QString name = tr("Tag");
    tagLable = new QLabel(name, q);
    tagLable->setObjectName(name);

    mainLayout->addWidget(tagLable);
    tagLeftLable = new QLabel(name, q);
    tagLeftLable->setObjectName(name);
    tagLeftLable->setHidden(true);

    colorListWidget = new TagColorListWidget(q);
    colorListWidget->setMaximumHeight(20);
    colorListWidget->setObjectName("tagActionWidget");
    colorListWidget->setToolTipVisible(false);

    QHBoxLayout *tagColorListLayout = new QHBoxLayout;
    tagColorListLayout->addWidget(tagLeftLable);
    tagColorListLayout->addWidget(colorListWidget);
    mainLayout->addLayout(tagColorListLayout);

    crumbEdit = new TagCrumbEdit(q);
    crumbEdit->setMaximumHeight(100);
    crumbEdit->setObjectName("tagCrumbEdit");

    crumbEdit->setFrameShape(QFrame::Shape::NoFrame);
    crumbEdit->viewport()->setBackgroundRole(QPalette::NoRole);
    mainLayout->addWidget(crumbEdit);

    mainLayout->setContentsMargins(10, 10, 10, 10);
}
