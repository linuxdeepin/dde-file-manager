// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconitemeditor.h"
#include "private/iconitemeditor_p.h"
#include "utils/itemdelegatehelper.h"

#include <dfm-base/utils/fileutils.h>

#include <DTextEdit>
#include <QMenu>
#include <QApplication>
#include <QGraphicsOpacityEffect>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

IconItemEditor::IconItemEditor(QWidget *parent)
    : QFrame(parent),
      dd(new IconItemEditorPrivate(this))
{
    dd->init();
}

IconItemEditor::~IconItemEditor()
{
    Q_D(IconItemEditor);

    if (d->tooltip) {
        d->tooltip->hide();
        d->tooltip->deleteLater();
        d->tooltip = nullptr;
    }
}

QString IconItemEditor::text() const
{
    Q_D(const IconItemEditor);
    return d->edit->toPlainText();
}

void IconItemEditor::setText(const QString &text)
{
    Q_D(const IconItemEditor);
    d->edit->setPlainText(text);
    d->edit->setAlignment(Qt::AlignHCenter);
}

void IconItemEditor::select(const QString &part)
{
    Q_D(const IconItemEditor);
    QString org = text();
    if (org.contains(part)) {
        int start = org.indexOf(org);
        if (Q_UNLIKELY(start < 0))
            start = 0;
        int end = start + part.size();
        if (Q_UNLIKELY(end > org.size()))
            end = org.size();

        QTextCursor cursor = d->edit->textCursor();
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        d->edit->setTextCursor(cursor);
    }
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

void IconItemEditor::setMaxHeight(int h)
{
    Q_D(IconItemEditor);
    d->maxHeight = h;
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

bool IconItemEditor::isEditReadOnly() const
{
    Q_D(const IconItemEditor);
    return d->edit->isReadOnly();
}

void IconItemEditor::setCharCountLimit()
{
    Q_D(IconItemEditor);
    d->useCharCountLimit = true;
}

void IconItemEditor::showAlertMessage(const QString &text, int duration)
{
    Q_D(IconItemEditor);

    if (!d->tooltip) {
        d->tooltip = createTooltip();
        d->tooltip->setBackgroundColor(palette().color(backgroundRole()));
        QTimer::singleShot(duration, this, [d] {
            if (d->tooltip) {
                d->tooltip->hide();
                d->tooltip->deleteLater();
                d->tooltip = nullptr;
            }
        });
    }

    if (QLabel *label = qobject_cast<QLabel *>(d->tooltip->getContent())) {
        label->setText(text);
        label->adjustSize();
    }

    QPoint pos = this->mapToGlobal(QPoint(this->width() / 2, this->height()));
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

void IconItemEditor::onEditTextChanged()
{
    Q_D(IconItemEditor);

    if (sender() != d->edit)
        return;

    if (!d->edit || isEditReadOnly())
        return;

    QSignalBlocker blocker(d->edit);

    QString currentText = text();
    if (currentText.isEmpty()) {
        resizeFromEditTextChanged();
        blocker.unblock();
        return;
    }

    QString dstText = FileUtils::preprocessingFileName(currentText);

    bool hasInvalidChar = currentText != dstText;

    int endPos = getTextEdit()->textCursor().position() + (dstText.length() - currentText.length());

    FileUtils::processLength(dstText, endPos, maxCharSize(), d->useCharCountLimit, dstText, endPos);
    if (currentText != dstText) {
        d->edit->setPlainText(dstText);
        QTextCursor cursor = d->edit->textCursor();
        cursor.setPosition(endPos);
        d->edit->setTextCursor(cursor);
        d->edit->setAlignment(Qt::AlignHCenter);
    }

    resizeFromEditTextChanged();

    if (editTextStackCurrentItem() != text()) {
        pushItemToEditTextStack(text());
    }

    if (hasInvalidChar) {
        showAlertMessage(tr("%1 are not allowed").arg("|/\\*:\"'?<>"));
    }
}

void IconItemEditor::resizeFromEditTextChanged()
{
    Q_D(IconItemEditor);
    updateEditorGeometry();
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
        int maxTextHeight = d->maxHeight - (contentsMargins().top() + d->icon->height() + kIconModeIconSpacing);
        if (maxTextHeight < 0) {
            d->edit->setFixedHeight(qMin(fontMetrics().height() * 3 + kIconModeTextPadding * 2, textHeight));
        } else {
            int minHeight = fontMetrics().height() * 1 + kIconModeTextPadding * 2;
            d->edit->setFixedHeight(qMax(qMin(maxTextHeight, textHeight), minHeight));
        }
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
    base = base.arg(palette().color(QPalette::Window).name(QColor::HexArgb))
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

DArrowRectangle *IconItemEditor::createTooltip()
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

