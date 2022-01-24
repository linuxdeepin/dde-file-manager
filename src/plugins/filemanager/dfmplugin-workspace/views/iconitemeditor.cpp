/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "iconitemeditor.h"
#include "private/iconitemeditor_p.h"
#include "utils/itemdelegatehelper.h"

#include <DTextEdit>
#include <QMenu>
#include <QApplication>

DPWORKSPACE_USE_NAMESPACE

IconItemEditor::IconItemEditor(QWidget *parent)
    : QFrame(parent),
      dd(new IconItemEditorPrivate(this))
{
    dd->init();
}

IconItemEditor::~IconItemEditor()
{
}

qreal IconItemEditor::opacity() const
{
    Q_D(const IconItemEditor);

    if (d->opacityEffect)
        return d->opacityEffect->opacity();
    return 1;
}

void IconItemEditor::setOpacity(qreal opacity)
{
    Q_D(IconItemEditor);

    if (opacity - 1.0 >= 0) {
        if (d->opacityEffect) {
            d->opacityEffect->deleteLater();
            d->opacityEffect = nullptr;
        }

        return;
    } else if (!d->opacityEffect) {
        d->opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(d->opacityEffect);
    }

    d->opacityEffect->setOpacity(opacity);
}

void IconItemEditor::setMaxCharSize(int maxSize)
{
    Q_D(IconItemEditor);
    d->maxCharSize = maxSize;
}

int IconItemEditor::maxCharSize() const
{
    Q_D(const IconItemEditor);
    return d->maxCharSize;
}

QSize IconItemEditor::sizeHint() const
{
    Q_D(const IconItemEditor);
    return QSize(width(), d->icon->height() + d->edit->height());
}

QLabel *IconItemEditor::getIconLabel() const
{
    Q_D(const IconItemEditor);
    return d->icon;
}

QTextEdit *IconItemEditor::getTextEdit() const
{
    Q_D(const IconItemEditor);
    return d->edit;
}

void IconItemEditor::showAlertMessage(const QString &text, int duration)
{
    Q_D(IconItemEditor);

    if (!d->tooltip) {
        d->tooltip = new DArrowRectangle(DArrowRectangle::ArrowTop, this);
        d->tooltip->setObjectName("AlertTooltip");

        QLabel *label = new QLabel(d->tooltip);

        label->setWordWrap(true);
        label->setMaximumWidth(500);
        d->tooltip->setContent(label);
        d->tooltip->setBackgroundColor(palette().color(backgroundRole()));
        d->tooltip->setArrowX(15);
        d->tooltip->setArrowHeight(5);

        QTimer::singleShot(duration, d->tooltip, [d] {
            d->tooltip->deleteLater();
            d->tooltip = Q_NULLPTR;
        });
    }

    QLabel *label = qobject_cast<QLabel *>(d->tooltip->getContent());

    if (!label) {
        return;
    }

    label->setText(text);
    label->adjustSize();

    const QPoint &pos = d->edit->mapToGlobal(QPoint(d->edit->width() / 2, d->edit->height()));

    d->tooltip->show(pos.x(), pos.y());
}

void IconItemEditor::popupEditContentMenu()
{
    Q_D(IconItemEditor);
    QMenu *menu = d->edit->createStandardContextMenu();

    if (!menu || d->edit->isReadOnly()) {
        return;
    }

    QAction *undoAction = menu->findChild<QAction *>(QStringLiteral("edit-undo"));
    QAction *redoAction = menu->findChild<QAction *>(QStringLiteral("edit-redo"));

    if (undoAction) {
        undoAction->setEnabled(d->editTextStackCurrentIndex > 0);
        disconnect(undoAction, SIGNAL(triggered(bool)));
        connect(undoAction, &QAction::triggered, this, &IconItemEditor::editUndo);
    }
    if (redoAction) {
        redoAction->setEnabled(d->editTextStackCurrentIndex < d->editTextStack.count() - 1);
        disconnect(redoAction, SIGNAL(triggered(bool)));
        connect(redoAction, &QAction::triggered, this, &IconItemEditor::editRedo);
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void IconItemEditor::editUndo()
{
    Q_D(IconItemEditor);
    d->disableEditTextStack = true;
    QTextCursor cursor = d->edit->textCursor();
    d->edit->setPlainText(editTextStackBack());
    d->edit->setTextCursor(cursor);
}

void IconItemEditor::editRedo()
{
    Q_D(IconItemEditor);
    d->disableEditTextStack = true;
    QTextCursor cursor = d->edit->textCursor();
    d->edit->setPlainText(editTextStackAdvance());
    d->edit->setTextCursor(cursor);
}

void IconItemEditor::doLineEditTextChanged()
{
    Q_D(IconItemEditor);
    QSignalBlocker blocker(d->edit);
    Q_UNUSED(blocker)

    const QString srcText = d->edit->toPlainText();
    // Todo(yanghao):preprocessingFileName
    QString dstText = srcText /* DFMGlobal::preprocessingFileName(srcText)*/;

    if (srcText != dstText) {
        // 修改文件的命名规则
        showAlertMessage(QObject::tr("%1 are not allowed").arg("|/\\*:\"'?<>"));
        d->edit->setPlainText(dstText);
    } else {
        return;
    }

    QVector<uint> list = dstText.toUcs4();
    int cursorPos = d->edit->textCursor().position() - srcText.length() + dstText.length();

    while (dstText.toLocal8Bit().size() > d->maxCharSize) {
        list.removeAt(--cursorPos);

        dstText = QString::fromUcs4(list.data(), list.size());
    }

    while (dstText.toLocal8Bit().size() > d->maxCharSize) {
        dstText.chop(1);
    }

    if (editTextStackCurrentItem() != dstText) {
        pushItemToEditTextStack(dstText);
    }

    QTextCursor cursor = d->edit->textCursor();

    cursor.movePosition(QTextCursor::Start);

    do {
        QTextBlockFormat format = cursor.blockFormat();

        format.setLineHeight(fontMetrics().height(), QTextBlockFormat::FixedHeight);
        cursor.setBlockFormat(format);
    } while (cursor.movePosition(QTextCursor::NextBlock));

    cursor.setPosition(cursorPos);

    d->edit->setTextCursor(cursor);
    d->edit->setAlignment(Qt::AlignHCenter);

    if (d->edit->isVisible()) {
        updateEditorGeometry();
    }
}

void IconItemEditor::resizeFromEditTextChanged()
{
    Q_D(IconItemEditor);
    //根据字符串的长度调整大小调整(之前的逻辑)
    updateEditorGeometry();
    //调整大小后，重新设置水平居中，否则会左对齐。
    if (d->edit) {
        d->edit->setAlignment(Qt::AlignHCenter);
    }
}

void IconItemEditor::updateEditorGeometry()
{
    Q_D(IconItemEditor);

    d->edit->setFixedWidth(width());
    int textHeight = static_cast<int>(d->edit->document()->size().height());

    if (d->edit->isReadOnly()) {
        if (d->edit->isVisible()) {
            d->edit->setFixedHeight(textHeight);
        }
    } else {
        d->edit->setFixedHeight(qMin(fontMetrics().height() * 3 + kIconModeTextPadding * 2, textHeight));
    }
}

bool IconItemEditor::event(QEvent *ee)
{
    Q_D(IconItemEditor);
    if (ee->type() == QEvent::DeferredDelete) {
        if (!d->canDeferredDelete) {
            ee->accept();

            return true;
        }
    } else if (ee->type() == QEvent::Resize) {
        updateEditorGeometry();
        int marginsHeight = contentsMargins().top();
        resize(width(), d->icon->height() + d->edit->height() + kIconModeIconSpacing + marginsHeight);
    } else if (ee->type() == QEvent::FontChange) {
        d->edit->setFont(font());
    }

    return QFrame::event(ee);
}

bool IconItemEditor::eventFilter(QObject *obj, QEvent *ee)
{
    Q_D(IconItemEditor);
    switch (ee->type()) {
    case QEvent::Resize:
        if (obj == d->icon || obj == d->edit) {
            int marginsHeight = contentsMargins().top();
            //计算高度时需加上marginsHeight，否则文字会显示不全
            resize(width(), d->icon->height() + d->edit->height() + kIconModeIconSpacing + marginsHeight);
        }

        break;
    case QEvent::KeyPress: {
        if (obj != d->edit) {
            return QFrame::eventFilter(obj, ee);
        }

        QKeyEvent *event = static_cast<QKeyEvent *>(ee);

        if (event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return && event->key() != Qt::Key_Tab) {
            if (event == QKeySequence::Undo) {
                editUndo();
            } else if (event == QKeySequence::Redo) {
                editRedo();
            } else {
                return QFrame::eventFilter(obj, ee);
            }

            d->disableEditTextStack = false;
            ee->accept();

            return true;
        }

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            ee->accept();
            parentWidget()->setFocus();

            return true;
        } else {
            event->accept();
            return false;
        }
    }
    case QEvent::FocusOut:
        if (obj == d->edit && qApp->focusWidget() != d->edit) {
            emit inputFocusOut();
        }

        break;
    case QEvent::Show:
        updateEditorGeometry();

        break;
    default:
        break;
    }

    return QFrame::eventFilter(obj, ee);
}

void IconItemEditor::updateStyleSheet()
{
    QString base = "IconItemEditor[showBackground=true] QTextEdit {background: %1; color: %2;}";

    base.append("IconItemEditor QTextEdit {color: %3}");
    base = base.arg(palette().color(QPalette::Background).name(QColor::HexArgb))
                   .arg(palette().color(QPalette::BrightText).name(QColor::HexArgb))
                   .arg(palette().color(QPalette::Text).name(QColor::HexArgb));

    // WARNING: setStyleSheet will clean margins!!!!!!
    auto saveContent = contentsMargins();
    setStyleSheet(base);
    setContentsMargins(saveContent);
}

QString IconItemEditor::editTextStackCurrentItem() const
{
    Q_D(const IconItemEditor);

    return d->editTextStack.value(d->editTextStackCurrentIndex);
}

QString IconItemEditor::editTextStackBack()
{
    Q_D(IconItemEditor);
    d->editTextStackCurrentIndex = qMax(0, d->editTextStackCurrentIndex - 1);
    const QString &text = editTextStackCurrentItem();
    return text;
}

QString IconItemEditor::editTextStackAdvance()
{
    Q_D(IconItemEditor);
    d->editTextStackCurrentIndex = qMin(d->editTextStack.count() - 1, d->editTextStackCurrentIndex + 1);
    const QString &text = editTextStackCurrentItem();

    return text;
}

void IconItemEditor::pushItemToEditTextStack(const QString &item)
{
    Q_D(IconItemEditor);
    if (d->disableEditTextStack) {
        return;
    }

    d->editTextStack.remove(d->editTextStackCurrentIndex + 1, d->editTextStack.count() - d->editTextStackCurrentIndex - 1);
    d->editTextStack.push(item);
    ++d->editTextStackCurrentIndex;
}
