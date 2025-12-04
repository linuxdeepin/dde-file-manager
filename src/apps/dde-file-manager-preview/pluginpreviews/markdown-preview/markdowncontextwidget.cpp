// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdowncontextwidget.h"
#include "markdownbrowser.h"

#include <DPlainTextEdit>

#include <QVBoxLayout>

using namespace plugin_filepreview;
DWIDGET_USE_NAMESPACE

MarkdownContextWidget::MarkdownContextWidget(QWidget *parent)
    : QWidget(parent)
    , m_browserWidget(new MarkdownBrowser(this))
{
    DPlainTextEdit *titleWidget = new DPlainTextEdit(this);
    titleWidget->setFixedHeight(30);
    titleWidget->setFrameStyle(QFrame::NoFrame);
    titleWidget->setReadOnly(true);
    titleWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(titleWidget);
    mainLay->addWidget(m_browserWidget);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
}

MarkdownBrowser *MarkdownContextWidget::markdownBrowser() const
{
    return m_browserWidget;
}
