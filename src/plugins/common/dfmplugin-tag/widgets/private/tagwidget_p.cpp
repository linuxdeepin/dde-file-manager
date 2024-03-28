// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagwidget_p.h"
#include "widgets/tagwidget.h"
#include "widgets/tagcolorlistwidget.h"
#include "widgets/tagcrumbedit.h"

#include <dfm-base/utils/universalutils.h>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QVBoxLayout>

static constexpr int kTagWidgetHeight { 114 };

DWIDGET_USE_NAMESPACE
DTK_USE_NAMESPACE

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
    q->setFrameShape(QFrame::NoFrame);
    mainLayout = new QVBoxLayout(q);
    q->setLayout(mainLayout);
    QString name = tr("Tag");
    tagLable = new DLabel(name, q);
    DFontSizeManager::instance()->bind(tagLable, DFontSizeManager::SizeType::T6, QFont::DemiBold);
    tagLable->setObjectName(name);

    colorListWidget = new TagColorListWidget(q, TagColorListWidget::kProperty);
    colorListWidget->setMaximumHeight(30);
    colorListWidget->setObjectName("tagActionWidget");
    colorListWidget->setToolTipVisible(false);

    crumbEdit = new TagCrumbEdit(q);
    crumbEdit->setObjectName("tagCrumbEdit");
    crumbEdit->setFrameShape(QFrame::Shape::NoFrame);
    crumbEdit->viewport()->setBackgroundRole(QPalette::NoRole);
    crumbEdit->setFocusPolicy(Qt::ClickFocus);

    tagColorListLayout = new QVBoxLayout;
    tagColorListLayout->addWidget(tagLable, 0, Qt::AlignLeft);
    tagColorListLayout->addWidget(colorListWidget, 0, Qt::AlignLeft);
    tagColorListLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addLayout(tagColorListLayout);

    mainLayout->addWidget(crumbEdit);

    mainLayout->setAlignment(Qt::AlignVCenter);
#ifdef DTKWIDGET_CLASS_DSizeMode
    initUiForSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &TagWidgetPrivate::initUiForSizeMode);
#else
    mainLayout->setContentsMargins(10, 10, 10, 10);
    crumbEdit->setMaximumHeight(100);
    q->setFixedHeight(kTagWidgetHeight);
#endif
}

void TagWidgetPrivate::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    mainLayout->setContentsMargins(DSizeModeHelper::element(5, 10), 6, 10, 10);
    crumbEdit->setMaximumHeight(DSizeModeHelper::element(50, 50));
    colorListWidget->setFixedWidth(214);
    q->setFixedHeight(DSizeModeHelper::element(150, kTagWidgetHeight));
#endif
}
