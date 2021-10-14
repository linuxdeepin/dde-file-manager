/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QBoxLayout>
#include <QTextEdit>
#include <QTextBlock>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QToolTip>

#include <danchors.h>
#include <DTextEdit>
#include <DArrowRectangle>
#include <DThemeManager>

#include "fileitem.h"
#include "dfmglobal.h"
#include "app/define.h"
#include <private/qtextedit_p.h>

DWIDGET_USE_NAMESPACE

class CanSetDragTextEdit : public DTextEdit
{
public:
    explicit CanSetDragTextEdit(QWidget *parent = nullptr);
    explicit CanSetDragTextEdit(const QString& text, QWidget* parent = nullptr);
    //set QTextEdit can drag
    void setDragEnabled(const bool &bdrag);
};

class FileIconItemPrivate{
public:
    FileIconItemPrivate(){}
    ~FileIconItemPrivate()
    {
        if (tooltip){
            tooltip->deleteLater();
        }
    }

    DArrowRectangle *tooltip {nullptr};
    QString validText;
};

FileIconItem::FileIconItem(QWidget *parent) :
    QFrame(parent)
  , d_ptr(new FileIconItemPrivate())
{
    icon = new QLabel(this);
    edit = new CanSetDragTextEdit(this);

    icon->setAlignment(Qt::AlignCenter);
    icon->setFrameShape(QFrame::NoFrame);
    icon->installEventFilter(this);

    edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);
    edit->installEventFilter(this);
    edit->setAcceptRichText(false);
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setAcceptDrops(false);
    static_cast<CanSetDragTextEdit *>(edit)->setDragEnabled(false);

    auto vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    vlayout->addWidget(icon, 0, Qt::AlignTop | Qt::AlignHCenter);
    vlayout->addSpacing(ICON_MODE_ICON_SPACING);
    vlayout->addWidget(edit, 0, Qt::AlignTop | Qt::AlignHCenter);

    setFrameShape(QFrame::NoFrame);
    setFocusProxy(edit);

    connect(edit, &QTextEdit::customContextMenuRequested, this, &FileIconItem::popupEditContentMenu);
}

FileIconItem::~FileIconItem()
{

}

qreal FileIconItem::opacity() const
{
    if (opacityEffect)
        return opacityEffect->opacity();

    return 1;
}

void FileIconItem::setOpacity(qreal opacity)
{
    if (opacity - 1.0 >= 0) {
        if (opacityEffect) {
            opacityEffect->deleteLater();
            opacityEffect = Q_NULLPTR;
        }

        return;
    } else if (!opacityEffect) {
        opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacityEffect);
    }

    opacityEffect->setOpacity(opacity);
}

void FileIconItem::setMaxCharSize(int maxSize)
{
    m_maxCharSize = maxSize;
}

int FileIconItem::maxCharSize()
{
    return m_maxCharSize;
}

QSize FileIconItem::sizeHint() const
{
    return QSize(width(), icon->height() + edit->height());
}

void FileIconItem::popupEditContentMenu()
{
    QMenu *menu = edit->createStandardContextMenu();

    if (!menu || edit->isReadOnly()) {
        return;
    }

    QAction *undo_action = menu->findChild<QAction *>(QStringLiteral("edit-undo"));
    QAction *redo_action = menu->findChild<QAction *>(QStringLiteral("edit-redo"));

    if (undo_action) {
        undo_action->setEnabled(editTextStackCurrentIndex > 0);
        disconnect(undo_action, SIGNAL(triggered(bool)));
        connect(undo_action, &QAction::triggered, this, &FileIconItem::editUndo);
    }
    if (redo_action) {
        redo_action->setEnabled(editTextStackCurrentIndex < editTextStack.count() - 1);
        disconnect(redo_action, SIGNAL(triggered(bool)));
        connect(redo_action, &QAction::triggered, this, &FileIconItem::editRedo);
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void FileIconItem::editUndo()
{
    disableEditTextStack = true;
    QTextCursor cursor = edit->textCursor();
    edit->setPlainText(editTextStackBack());
    edit->setTextCursor(cursor);
}

void FileIconItem::editRedo()
{
    disableEditTextStack = true;
    QTextCursor cursor = edit->textCursor();
    edit->setPlainText(editTextStackAdvance());
    edit->setTextCursor(cursor);
}

void FileIconItem::doLineEditTextChanged()
{
    QSignalBlocker blocker(edit);
    Q_UNUSED(blocker)

    const QString srcText = edit->toPlainText();
    QString dstText = DFMGlobal::preprocessingFileName(srcText);

    if (srcText != dstText){
        // 修改文件的命名规则
        showAlertMessage(tr("%1 are not allowed").arg("|[/\\*:\"']?<>"));
        edit->setPlainText(dstText);
    } else {
        return;
    }

    QVector<uint> list = dstText.toUcs4();
    int cursor_pos = edit->textCursor().position() - srcText.length() + dstText.length();

    while (dstText.toLocal8Bit().size() > m_maxCharSize) {
        list.removeAt(--cursor_pos);

        dstText = QString::fromUcs4(list.data(), list.size());
    }

    while (dstText.toLocal8Bit().size() > m_maxCharSize) {
        dstText.chop(1);
    }

    if (editTextStackCurrentItem() != dstText) {
        pushItemToEditTextStack(dstText);
    }

    QTextCursor cursor = edit->textCursor();

    cursor.movePosition(QTextCursor::Start);

    do {
        QTextBlockFormat format = cursor.blockFormat();

        format.setLineHeight(fontMetrics().height(), QTextBlockFormat::FixedHeight);
        cursor.setBlockFormat(format);
    } while (cursor.movePosition(QTextCursor::NextBlock));

    cursor.setPosition(cursor_pos);

    edit->setTextCursor(cursor);
    edit->setAlignment(Qt::AlignHCenter);

    if (edit->isVisible()) {
        updateEditorGeometry();
    }
}

//提供外部调用的槽函数
void FileIconItem::resizeFromEditTextChanged()
{
    //根据字符串的长度调整大小调整(之前的逻辑)
    updateEditorGeometry();
    //调整大小后，重新设置水平居中，否则会左对齐。
    if (edit) {
        edit->setAlignment(Qt::AlignHCenter);
    }
}

void FileIconItem::showAlertMessage(const QString &text, int duration)
{
    Q_D(FileIconItem);

    if (!d->tooltip){
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

    const QPoint &pos = edit->mapToGlobal(QPoint(edit->width()/2, edit->height()));

    d->tooltip->show(pos.x(), pos.y());
}

bool FileIconItem::event(QEvent *ee)
{
    if (ee->type() == QEvent::DeferredDelete) {
        if (!canDeferredDelete) {
            ee->accept();

            return true;
        }
    } else if (ee->type() == QEvent::Resize) {
        updateEditorGeometry();
        int marginsHeight = contentsMargins().top();
        resize(width(), icon->height() + edit->height() + ICON_MODE_ICON_SPACING + marginsHeight);
    } else if (ee->type() == QEvent::FontChange) {
        edit->setFont(font());
    }

    return QFrame::event(ee);
}

bool FileIconItem::eventFilter(QObject *obj, QEvent *ee)
{
    switch (ee->type()) {
    case QEvent::Resize:
        if (obj == icon || obj == edit) {
            int marginsHeight = contentsMargins().top();
            //计算高度时需加上marginsHeight，否则文字会显示不全
            resize(width(), icon->height() + edit->height() + ICON_MODE_ICON_SPACING + marginsHeight);
        }

        break;
    case QEvent::KeyPress: {
        if (obj != edit) {
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

            disableEditTextStack = false;
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

        break;
    }
    case QEvent::FocusOut:
        if (obj == edit && qApp->focusWidget() != edit) {
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

void FileIconItem::updateEditorGeometry()
{
    edit->setFixedWidth(width());
    int text_height = static_cast<int>(edit->document()->size().height());

    if (edit->isReadOnly()) {
        if (edit->isVisible()) {
            edit->setFixedHeight(text_height);
        }
    } else {
        edit->setFixedHeight(qMin(fontMetrics().height() * 3 + TEXT_PADDING * 2, text_height));
    }
}

void FileIconItem::updateStyleSheet()
{
    QString base = "FileIconItem[showBackground=true] QTextEdit {background: %1; color: %2;}";

    base.append("FileIconItem QTextEdit {color: %3}");
    base = base.arg(palette().color(QPalette::Background).name(QColor::HexArgb))
            .arg(palette().color(QPalette::BrightText).name(QColor::HexArgb))
            .arg(palette().color(QPalette::Text).name(QColor::HexArgb));

    // WARNING: setStyleSheet will clean margins!!!!!!
    auto saveContent = contentsMargins();
    setStyleSheet(base);
    setContentsMargins(saveContent);
}

QString FileIconItem::editTextStackCurrentItem() const
{
    return editTextStack.value(editTextStackCurrentIndex);
}

QString FileIconItem::editTextStackBack()
{
    editTextStackCurrentIndex = qMax(0, editTextStackCurrentIndex - 1);
    const QString &text = editTextStackCurrentItem();

    return text;
}

QString FileIconItem::editTextStackAdvance()
{
    editTextStackCurrentIndex = qMin(editTextStack.count() - 1, editTextStackCurrentIndex + 1);
    const QString &text = editTextStackCurrentItem();

    return text;
}

void FileIconItem::pushItemToEditTextStack(const QString &item)
{
    if (disableEditTextStack) {
        return;
    }

    editTextStack.remove(editTextStackCurrentIndex + 1, editTextStack.count() - editTextStackCurrentIndex - 1);
    editTextStack.push(item);
    ++editTextStackCurrentIndex;
}

CanSetDragTextEdit::CanSetDragTextEdit(QWidget *parent) :
    DTextEdit (parent)
{

}

CanSetDragTextEdit::CanSetDragTextEdit(const QString &text, QWidget *parent) :
    DTextEdit (text, parent)
{

}

void CanSetDragTextEdit::setDragEnabled(const bool &bdrag)
{
    QTextEditPrivate *dd = reinterpret_cast<QTextEditPrivate *>(qGetPtrHelper(d_ptr));
    dd->control->setDragEnabled(bdrag);
}
