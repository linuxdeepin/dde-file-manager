/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
