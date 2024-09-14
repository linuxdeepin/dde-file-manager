// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitemeditor.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/fileutils.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

ListItemEditor::ListItemEditor(QWidget *parent)
    : QLineEdit(parent)
{
    init();
}

ListItemEditor::~ListItemEditor()
{
    if (tooltip) {
        tooltip->hide();
        tooltip->deleteLater();
        tooltip = nullptr;
    }
}

void ListItemEditor::select(const QString &part)
{
    QString org = text();
    if (org.contains(part)) {
        int start = org.indexOf(org);
        if (Q_UNLIKELY(start < 0))
            start = 0;
        int end = start + part.size();
        if (Q_UNLIKELY(end > org.size()))
            end = org.size();

        setCursorPosition(end);
        setSelection(0, end);
    }
}

void ListItemEditor::showAlertMessage(const QString &text, int duration)
{
    if (!tooltip) {
        tooltip = createTooltip();
        tooltip->setBackgroundColor(palette().color(backgroundRole()));
        QTimer::singleShot(duration, this, [this] {
            if (tooltip) {
                tooltip->hide();
                tooltip->deleteLater();
                tooltip = nullptr;
            }
        });
    }

    if (QLabel *label = qobject_cast<QLabel *>(tooltip->getContent())) {
        label->setText(text);
        label->adjustSize();
    }

    QPoint pos = this->mapToGlobal(QPoint(this->width() / 2, this->height()));
    tooltip->show(pos.x(), pos.y());
}

DArrowRectangle *ListItemEditor::createTooltip()
{
    auto tooltip = new DArrowRectangle(DArrowRectangle::ArrowTop);
    tooltip->setObjectName("AlertTooltip");

    QLabel *label = new QLabel(tooltip);

    label->setWordWrap(true);
    label->setMaximumWidth(500);
    tooltip->setContent(label);
    tooltip->setArrowX(15);
    tooltip->setArrowHeight(5);
    return tooltip;
}

bool ListItemEditor::event(QEvent *ee)
{
    switch (ee->type()) {
    case QEvent::FocusOut:
        emit inputFocusOut();
        break;
    default:
        break;
    }

    return QLineEdit::event(ee);
}

void ListItemEditor::onEditorTextChanged(const QString &text)
{
    const QString srcText = text;

    QString dstText = FileUtils::preprocessingFileName(text);

    bool hasInvalidChar = text != dstText;

    if (hasInvalidChar) {
        showAlertMessage(tr("%1 are not allowed").arg("|/\\*:\"'?<>"));
    }

    int currPos = this->cursorPosition();
    currPos += dstText.length() - text.length();

    FileUtils::processLength(dstText, currPos, theMaxCharSize, useCharCount, dstText, currPos);
    if (srcText != dstText) {
        QSignalBlocker blocker(this);
        this->setText(dstText);
        this->setCursorPosition(currPos);
    }
}

void ListItemEditor::init()
{
    setObjectName("ListItemDelegate_Editor");

    setFrame(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0, 0, 0, 0);
    connect(this, &ListItemEditor::textChanged, this, &ListItemEditor::onEditorTextChanged, Qt::UniqueConnection);
}
