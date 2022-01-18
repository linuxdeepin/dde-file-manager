/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "itemeditor.h"
#include "canvasitemdelegate.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QLabel>
DWIDGET_USE_NAMESPACE
DSB_D_USE_NAMESPACE

ItemEditor::ItemEditor(QWidget *parent) : QFrame(parent)
{
    init();
}

ItemEditor::~ItemEditor()
{

}

void ItemEditor::setBaseGeometry(const QRect &base, const QSize &itemSize, const QMargins &margin)
{
    delete layout();

    // move editor to the grid.
    move(base.topLeft());
    setFixedWidth(base.width());
    // minimum height is virtualRect's height.
    setMinimumHeight(base.height());

    auto lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);

    // add grid margin ,icon height ,icon space, text padding
    lay->setContentsMargins(margin);

    // the textEditor is on text painting area.
    lay->addWidget(textEditor, 0, Qt::AlignTop | Qt::AlignHCenter);

    itemSizeHint = itemSize;
    updateGeometry();
}

void ItemEditor::init()
{
    setFrameShape(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);
    textEditor = creatEditor();
    textEditor->setParent(this);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(textEditor, Qt::AlignTop | Qt::AlignHCenter);

    // the editor is real focus widget.
    setFocusProxy(textEditor);
}

void ItemEditor::updateGeometry()
{
    //! the textEditor->document()->textWidth will changed after textEditor->setFixedWidth although it was setted before.
    textEditor->setFixedWidth(width());

    //! so need to reset it.
    textEditor->document()->setTextWidth(itemSizeHint.width());

    int textHeight = static_cast<int>(textEditor->document()->size().height());
    if (textEditor->isReadOnly()) {
        textEditor->setFixedHeight(textHeight);
    } else {
        textHeight = qMin(fontMetrics().height() * 3 + CanvasItemDelegate::kTextPadding * 2, textHeight);
        textEditor->setFixedHeight(textHeight);
    }

    QFrame::updateGeometry();
}

QString ItemEditor::text() const
{
    return textEditor->toPlainText();
}

void ItemEditor::setText(const QString &text)
{
    textEditor->setPlainText(text);
    textEditor->setAlignment(Qt::AlignHCenter);
    updateGeometry();
}

void ItemEditor::setItemSizeHint(QSize size)
{
    itemSizeHint = size;
    updateGeometry();
}

void ItemEditor::select(const QString &part)
{
    QString org = text();
    if (org.contains(part)) {
        int start = org.indexOf(org);
        if (Q_UNLIKELY(start < 0))
            start = 0;
        int end = start + part.size();
        if (Q_UNLIKELY(end > org.size()))
            end = org.size();

        QTextCursor cursor = textEditor->textCursor();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        textEditor->setTextCursor(cursor);
    }
}

void ItemEditor::setOpacity(qreal opacity)
{
    if (opacity - 1.0 >= 0) {
        if (opacityEffect) {
            opacityEffect->deleteLater();
            opacityEffect = Q_NULLPTR;
        }
        return;
    }

    if (!opacityEffect) {
        opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacityEffect);
    }

    opacityEffect->setOpacity(opacity);
}

RenameEdit *ItemEditor::creatEditor()
{
    auto edit = new RenameEdit();
    edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);
    edit->setAcceptRichText(false);
    edit->setAcceptDrops(false);

    return edit;
}

void RenameEdit::undo()
{
    qInfo() << __FUNCTION__;
}

void RenameEdit::redo()
{
    qInfo() << __FUNCTION__;
}

void RenameEdit::contextMenuEvent(QContextMenuEvent *e)
{
    // this menu only popup when preesed on viewport.
    // and pressing on the point that out of viewport but in RenameEdit will popoup other menu.
    e->accept();

    if (isReadOnly())
        return;

    QMenu *menu = createStandardContextMenu();
    if (!menu)
        return;

    // rewrite redo and undo, do not use them provided by QTextEdit.
    QAction *undoAction = menu->findChild<QAction *>(QStringLiteral("edit-undo"));
    QAction *redoAction = menu->findChild<QAction *>(QStringLiteral("edit-redo"));

    if (undoAction) {
        //undoAction->setEnabled(editTextStackCurrentIndex > 0);
        disconnect(undoAction, SIGNAL(triggered(bool)), nullptr, nullptr);
        connect(undoAction, &QAction::triggered, this, &RenameEdit::undo);
    }

    if (redoAction) {
        //redoAction->setEnabled(editTextStackCurrentIndex < editTextStack.count() - 1);
        QObject::disconnect(redoAction, SIGNAL(triggered(bool)), nullptr, nullptr);
        connect(redoAction, &QAction::triggered, this, &RenameEdit::redo);
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void RenameEdit::focusOutEvent(QFocusEvent *e)
{
    // do not emit when focus out by showing menu.
    if (qApp->focusWidget() != this)
        QMetaObject::invokeMethod(parent(), "inputFocusOut", Qt::QueuedConnection);
    DTextEdit::focusOutEvent(e);
}

void RenameEdit::keyPressEvent(QKeyEvent *e)
{
    // rewrite redo and undo, do not use them provided by QTextEdit.
    if (e == QKeySequence::Undo) {
        undo();
        return;
    } else if (e == QKeySequence::Redo) {
        redo();
        return;
    }

    DTextEdit::keyPressEvent(e);
}
