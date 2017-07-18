#include <QTextEdit>
#include <QTextBlock>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QMenu>

#include <anchors.h>

#include "fileitem.h"
#include "dfmglobal.h"
#include "app/define.h"

DWIDGET_USE_NAMESPACE

FileIconItem::FileIconItem(QWidget *parent) :
    QFrame(parent)
{
    icon = new QLabel(this);
    edit = new QTextEdit(this);

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

    AnchorsBase::setAnchor(icon, Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    AnchorsBase::setAnchor(edit, Qt::AnchorTop, icon, Qt::AnchorBottom);
    AnchorsBase::setAnchor(edit, Qt::AnchorHorizontalCenter, icon, Qt::AnchorHorizontalCenter);

    AnchorsBase::getAnchorBaseByWidget(edit)->setTopMargin(ICON_MODE_ICON_SPACING);

    setFrameShape(QFrame::NoFrame);
    setFocusProxy(edit);

    connect(edit, &QTextEdit::textChanged, this, [this] {
        QSignalBlocker blocker(edit);
        Q_UNUSED(blocker)

        QString text = edit->toPlainText();
        const QString old_text = text;

        int text_length = text.length();
        int text_line_height = fontMetrics().height();

        text.remove('/');
        text.remove(QChar(0));

        QVector<uint> list = text.toUcs4();
        int cursor_pos = edit->textCursor().position() - text_length + text.length();

        while (text.toUtf8().size() > MAX_FILE_NAME_CHAR_COUNT) {
            list.removeAt(--cursor_pos);

            text = QString::fromUcs4(list.data(), list.size());
        }

        if (text.count() != old_text.count()) {
            edit->setPlainText(text);
        }

        if (editTextStackCurrentItem() != text) {
            pushItemToEditTextStack(text);
        }

        QTextCursor cursor = edit->textCursor();

        cursor.movePosition(QTextCursor::Start);

        do {
            QTextBlockFormat format = cursor.blockFormat();

            format.setLineHeight(text_line_height, QTextBlockFormat::FixedHeight);
            cursor.setBlockFormat(format);
        } while (cursor.movePosition(QTextCursor::NextBlock));

        cursor.setPosition(cursor_pos);

        edit->setTextCursor(cursor);
        edit->setAlignment(Qt::AlignHCenter);

        if (edit->isReadOnly() && edit->isVisible())
            edit->setFixedHeight(edit->document()->size().height());
    });
    connect(edit, &QTextEdit::customContextMenuRequested, this, &FileIconItem::popupEditContentMenu);
}

void FileIconItem::setOpacity(qreal opacity)
{
    if (opacity - 1.0 >= 0) {
        if (opacityEffect) {
            opacityEffect->deleteLater();
            opacityEffect = Q_NULLPTR;
        }

        return;
    } else if(!opacityEffect) {
        opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacityEffect);
    }

    opacityEffect->setOpacity(opacity);
}

QSize FileIconItem::sizeHint() const
{
    return QSize(width(), icon->height() + edit->height());
}

QColor FileIconItem::borderColor() const
{
    return m_borderColor;
}

void FileIconItem::setBorderColor(QColor borderColor)
{
    if (m_borderColor == borderColor)
        return;

    m_borderColor = borderColor;
    emit borderColorChanged(borderColor);

    updateStyleSheet();
}

void FileIconItem::popupEditContentMenu()
{
    QMenu *menu = edit->createStandardContextMenu();

    if (!menu)
        return;

    QAction *undo_action = menu->findChild<QAction*>(QStringLiteral("edit-undo"));
    QAction *redo_action = menu->findChild<QAction*>(QStringLiteral("edit-redo"));

    undo_action->setEnabled(editTextStackCurrentIndex > 0);
    redo_action->setEnabled(editTextStackCurrentIndex < editTextStack.count() - 1);

    disconnect(undo_action, SIGNAL(triggered(bool)));
    disconnect(redo_action, SIGNAL(triggered(bool)));
    connect(undo_action, &QAction::triggered, this, &FileIconItem::editUndo);
    connect(redo_action, &QAction::triggered, this, &FileIconItem::editRedo);

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

bool FileIconItem::event(QEvent *ee)
{
    if (ee->type() == QEvent::DeferredDelete) {
        if (!canDeferredDelete) {
            ee->accept();

            return true;
        }
    } else if(ee->type() == QEvent::Resize) {
        updateEditorGeometry();

        resize(width(), icon->height() + edit->height() + ICON_MODE_ICON_SPACING);
    } else if (ee->type() == QEvent::FontChange) {
        edit->setFont(font());
    }

    return QFrame::event(ee);
}

bool FileIconItem::eventFilter(QObject *obj, QEvent *ee)
{
    switch (ee->type()) {
    case QEvent::Resize:
        if(obj == icon || obj == edit) {
            resize(width(), icon->height() + edit->height() + ICON_MODE_ICON_SPACING);
        }

        break;
    case QEvent::KeyPress: {
        if(obj != edit) {
           return QFrame::eventFilter(obj, ee);
        }

        QKeyEvent *event = static_cast<QKeyEvent*>(ee);

        if (event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return) {
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

        if(!(event->modifiers() & Qt::ShiftModifier)) {
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

    if (edit->isReadOnly()) {
        int text_height = edit->document()->size().height();

        if (edit->isVisible())
            edit->setFixedHeight(text_height);
    } else {
        edit->setFixedHeight(fontMetrics().height() * 3 + TEXT_PADDING * 2);
    }
}

void FileIconItem::updateStyleSheet()
{
    QString base = "FileIconItem[showBackground=true] QTextEdit {background: %1; color: %2; border: 2px solid %3;}";

    base.append("FileIconItem QTextEdit {color: %4}");
    base = base.arg(palette().color(QPalette::Background).name(QColor::HexArgb))
            .arg(palette().color(QPalette::BrightText).name(QColor::HexArgb))
            .arg(m_borderColor.name(QColor::HexArgb))
            .arg(palette().color(QPalette::Text).name(QColor::HexArgb));

    setStyleSheet(base);
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
    if (disableEditTextStack)
        return;

    editTextStack.remove(editTextStackCurrentIndex + 1, editTextStack.count() - editTextStackCurrentIndex - 1);
    editTextStack.push(item);
    ++editTextStackCurrentIndex;
}
