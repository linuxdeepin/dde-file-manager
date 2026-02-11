// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemeditor.h"
#include "canvasitemdelegate.h"

#include <dfm-base/utils/fileutils.h>

#include <DArrowRectangle>
#include <DStyle>

#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QLabel>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_canvas;

ItemEditor::ItemEditor(QWidget *parent)
    : QFrame(parent)
{
    init();
}

ItemEditor::~ItemEditor()
{
    if (tooltip) {
        tooltip->hide();
        tooltip->deleteLater();
        tooltip = nullptr;
    }
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
    lay->setContentsMargins(0, 0, 0, 0);
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
    textEditor = createEditor();
    textEditor->setParent(this);
    textEditor->installEventFilter(textEditor);   // for draw

    connect(textEditor, &RenameEdit::textChanged, this, &ItemEditor::textChanged, Qt::UniqueConnection);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
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
        // maxheight subtracts the starting position of edit
        // Note: textEditor ->pos(). y() cannot be used directly,
        // because it is a variable rather than a fixed height value
        auto maxTextHeight = maxHeight - layout()->contentsMargins().top();
        if (maxTextHeight <= 0) {
            textEditor->setFixedHeight(qMin(fontMetrics().height() * 3 + CanvasItemDelegate::kTextPadding * 2,
                                            textHeight));
        } else {
            int minHeight = fontMetrics().height() * 1 + CanvasItemDelegate::kTextPadding * 2;
            // insufficient free space, forced display of one line
            if (maxTextHeight < minHeight)
                textEditor->setFixedHeight(minHeight);
            else
                textEditor->setFixedHeight(qMin(maxTextHeight, textHeight));
        }
    }

    // make layout to adjust height.
    adjustSize();
    QFrame::updateGeometry();
}

void ItemEditor::showAlertMessage(const QString &text, int duration)
{
    if (!tooltip) {
        tooltip = createTooltip();
        tooltip->setBackgroundColor(palette().color(backgroundRole()));
        QTimer::singleShot(duration, this, [this] {
            if (tooltip) {
                tooltip->setParent(nullptr);
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

    if (!this->window())
        return;

    QPoint pos = textEditor->mapTo(this->window(), QPoint(textEditor->width() / 2, textEditor->height()));
    tooltip->setParent(this->window());
    tooltip->show(pos.x(), pos.y());
}

DArrowRectangle *ItemEditor::createTooltip()
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

RenameEdit *ItemEditor::createEditor()
{
    auto edit = new RenameEdit();
    edit->setWordWrapMode(QTextOption::WrapAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);
    edit->setAcceptRichText(false);
    edit->setAcceptDrops(false);

    return edit;
}

void ItemEditor::textChanged()
{
    if (sender() != textEditor)
        return;

    if (textEditor->isReadOnly())
        return;

    // block signal to prevent signal from being sent again.
    QSignalBlocker blocker(textEditor);

    const QString curText = textEditor->toPlainText();

    if (curText.isEmpty()) {
        blocker.unblock();
        updateGeometry();
        return;
    }

    QString dstText = DFMBASE_NAMESPACE::FileUtils::preprocessingFileName(curText);
    bool hasInvalidChar = dstText.size() != curText.size();
    int endPos = textEditor->textCursor().position() + (dstText.length() - curText.length());

    DFMBASE_NAMESPACE::FileUtils::processLength(dstText, endPos, maximumLength(), useCharCount, dstText, endPos);
    if (curText != dstText) {
        textEditor->setPlainText(dstText);
        QTextCursor cursor = textEditor->textCursor();
        cursor.setPosition(endPos);
        textEditor->setTextCursor(cursor);
        textEditor->setAlignment(Qt::AlignHCenter);
    }

    // push stack.
    if (textEditor->stackCurrent() != dstText)
        textEditor->pushStatck(dstText);

    blocker.unblock();
    updateGeometry();

    if (hasInvalidChar)
        showAlertMessage(tr("%1 are not allowed").arg("|/\\*:\"'?<>"));
}

RenameEdit::RenameEdit(QWidget *parent)
    : DTextEdit(parent)
{
    adjustStyle();
}

void RenameEdit::undo()
{
    // make signal textChanged to be invalid to push stack.
    enableStack = false;

    QTextCursor cursor = textCursor();
    setPlainText(stackBack());
    setTextCursor(cursor);
    setAlignment(Qt::AlignHCenter);
    enableStack = true;

    // update edit height by ItemEditor::updateGeometry
    QMetaObject::invokeMethod(parent(), "updateGeometry");
}

void RenameEdit::redo()
{
    // make signal textChanged to be invalid.
    enableStack = false;

    QTextCursor cursor = textCursor();
    setPlainText(stackAdvance());
    setTextCursor(cursor);
    setAlignment(Qt::AlignHCenter);
    enableStack = true;

    // update edit height by ItemEditor::updateGeometry
    QMetaObject::invokeMethod(parent(), "updateGeometry");
}

QString RenameEdit::stackCurrent() const
{
    return textStack.value(stackCurrentIndex);
}

QString RenameEdit::stackBack()
{
    stackCurrentIndex = qMax(0, stackCurrentIndex - 1);
    const QString &text = stackCurrent();
    return text;
}

QString RenameEdit::stackAdvance()
{
    stackCurrentIndex = qMin(textStack.count() - 1, stackCurrentIndex + 1);
    const QString &text = stackCurrent();
    return text;
}

void RenameEdit::adjustStyle()
{
    // Set margins for text and edit boxes
    document()->setDocumentMargin(CanvasItemDelegate::kTextPadding);

    // In DTextEdit, FrameRadius will be set to viewportMargins, which will reduce the available width of text
    // Modify FrameRadius to 0
    DTK_WIDGET_NAMESPACE::DStyle::setFrameRadius(this, 0);
}

bool RenameEdit::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Paint && obj == this) {
        // Get the original radius value
        const int oldFrameRadius = DStyle::pixelMetric(style(), DStyle::PM_FrameRadius, nullptr, this);

        // ths basic value which should be a fixed value written as 8 in dtk
        const int frameRadius = DStyle::pixelMetric(style(), DStyle::PM_FrameRadius);

        // Set frameRadius to draw radius rect as background
        DStyle::setFrameRadius(this, frameRadius);

        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);

        QStyleOptionFrame panel;
        initStyleOption(&panel);
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

        DStyle::setFrameRadius(this, oldFrameRadius);
        return true;
    }

    return DTextEdit::eventFilter(obj, e);
}

void RenameEdit::pushStatck(const QString &item)
{
    if (!enableStack)
        return;

    textStack.remove(stackCurrentIndex + 1, textStack.count() - stackCurrentIndex - 1);
    textStack.push(item);
    ++stackCurrentIndex;
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
        undoAction->setEnabled(stackCurrentIndex > 0);
        disconnect(undoAction, SIGNAL(triggered(bool)), nullptr, nullptr);
        connect(undoAction, &QAction::triggered, this, &RenameEdit::undo);
    }

    if (redoAction) {
        redoAction->setEnabled(stackCurrentIndex < textStack.count() - 1);
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
        e->accept();
        return;
    } else if (e == QKeySequence::Redo) {
        redo();
        e->accept();
        return;
    }

    // key to commit
    // it can edit next or previous item if ingore Qt::Key_Tab or Qt::Key_Backtab.
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        e->accept();
        QMetaObject::invokeMethod(parent(), "inputFocusOut", Qt::QueuedConnection);
        return;
    default:
        break;
    }

    DTextEdit::keyPressEvent(e);
}

void RenameEdit::showEvent(QShowEvent *e)
{
    DTextEdit::showEvent(e);

    // the editor will be not active when opening the file and then pressing it immediately.
    if (!isActiveWindow())
        activateWindow();
}
