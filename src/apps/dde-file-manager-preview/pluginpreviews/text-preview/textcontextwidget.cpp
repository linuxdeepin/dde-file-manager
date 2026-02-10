// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textcontextwidget.h"
#include "textbrowseredit.h"

#include <DPlainTextEdit>

#include <QVBoxLayout>

using namespace plugin_filepreview;
DWIDGET_USE_NAMESPACE

TextContextWidget::TextContextWidget(QWidget *parent)
    : QWidget(parent)
    , editWidget(new TextBrowserEdit(this))
{
    DPlainTextEdit *titleWidget = new DPlainTextEdit(this);
    titleWidget->setFixedHeight(30);
    titleWidget->setFrameStyle(QFrame::NoFrame);
    titleWidget->setReadOnly(true);
    titleWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(titleWidget);
    mainLay->addWidget(editWidget);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
}

TextBrowserEdit *TextContextWidget::textBrowserEdit() const
{
    return editWidget;
}
