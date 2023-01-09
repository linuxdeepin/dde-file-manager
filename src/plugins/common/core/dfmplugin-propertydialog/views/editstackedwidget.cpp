/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "editstackedwidget.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/elidetextlayout.h"

#include <dfm-framework/dpf.h>

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QFileInfo>
#include <QLabel>
#include <QPainterPath>

Q_DECLARE_METATYPE(QList<QUrl> *)

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

const int kTextLineHeight = 18;

NameTextEdit::NameTextEdit(const QString &text, QWidget *parent)
    : QTextEdit(text, parent)
{
    setObjectName("NameTextEdit");
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setFixedSize(200, 60);
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QTextEdit::textChanged, this, &NameTextEdit::slotTextChanged);
}

NameTextEdit::~NameTextEdit()
{
}

bool NameTextEdit::isCanceled() const
{
    return isCancel;
}

void NameTextEdit::setIsCanceled(bool isCanceled)
{
    isCancel = isCanceled;
}

void NameTextEdit::setPlainText(const QString &text)
{
    QTextEdit::setPlainText(text);
    setAlignment(Qt::AlignCenter);
}

void NameTextEdit::slotTextChanged()
{
    QSignalBlocker blocker(this);
    Q_UNUSED(blocker)

    QString text = this->toPlainText();
    const QString old_text = text;

    int text_length = text.length();

    text.remove('/');
    text.remove(QChar(0));

    QVector<uint> list = text.toUcs4();
    int cursor_pos = this->textCursor().position() - text_length + text.length();

    while (text.toLocal8Bit().count() > NAME_MAX) {
        text.chop(1);
    }

    if (text.count() != old_text.count()) {
        this->setText(text);
    }

    QTextCursor cursor = this->textCursor();

    cursor.movePosition(QTextCursor::Start);

    do {
        QTextBlockFormat format = cursor.blockFormat();

        format.setLineHeight(kTextLineHeight, QTextBlockFormat::FixedHeight);
        cursor.setBlockFormat(format);
    } while (cursor.movePosition(QTextCursor::NextBlock));

    cursor.setPosition(cursor_pos);

    this->setTextCursor(cursor);
    this->setAlignment(Qt::AlignHCenter);

    if (this->isReadOnly())
        this->setFixedHeight(static_cast<int>(this->document()->size().height()));
}

void NameTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextEdit::focusOutEvent(event);
}

void NameTextEdit::keyPressEvent(QKeyEvent *event)
{
    QEvent::Type ty = event->type();
    Q_UNUSED(ty)
    Qt::KeyboardModifiers modifiers = event->modifiers();
    Q_UNUSED(modifiers)
    if (event->key() == Qt::Key_Escape) {
        setIsCanceled(true);
        emit editFinished();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        setIsCanceled(false);
        emit editFinished();
    }
    QTextEdit::keyPressEvent(event);
}

EditStackedWidget::EditStackedWidget(QWidget *parent)
    : QStackedWidget(parent), textShowFrame(nullptr)
{
    initUI();
}

EditStackedWidget::~EditStackedWidget()
{
}

void EditStackedWidget::initUI()
{
    QFrame *editFrame = new QFrame;
    fileNameEdit = new NameTextEdit(QString(""), editFrame);
    QHBoxLayout *editLayout = new QHBoxLayout;
    editLayout->addStretch();
    editLayout->addWidget(fileNameEdit);
    editLayout->addStretch();
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editFrame->setLayout(editLayout);

    this->addWidget(editFrame);

    connect(fileNameEdit, &NameTextEdit::editFinished, this, &EditStackedWidget::showTextShowFrame);
}

void EditStackedWidget::initTextShowFrame(QString fileName)
{
    QRect rect(QPoint(0, 0), QSize(200, 66));
    QStringList labelTexts;
    ElideTextLayout layout(fileName);
    layout.layout(rect, Qt::ElideMiddle, nullptr, Qt::NoBrush, &labelTexts);

    int textHeight = 0;

    if (textShowFrame) {
        delete textShowFrame;
        textShowFrame = new QFrame(this);
    } else {
        textShowFrame = new QFrame(this);
    }

    nameEditIcon = new DIconButton(textShowFrame);
    nameEditIcon->setObjectName(QString("EditButton"));
    nameEditIcon->setIcon(QIcon::fromTheme("edit-rename"));
    nameEditIcon->setIconSize({ 24, 24 });
    nameEditIcon->setFixedSize(24, 24);
    nameEditIcon->setFlat(true);

    connect(nameEditIcon, &QPushButton::clicked, this, &EditStackedWidget::renameFile);

    QVBoxLayout *textShowLayout = new QVBoxLayout;
    for (const auto &labelText : labelTexts) {
        DLabel *fileNameLabel = new DLabel(labelText, textShowFrame);
        fileNameLabel->setAlignment(Qt::AlignHCenter);
        textHeight += fileNameLabel->fontInfo().pixelSize() + 10;

        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addStretch(1);
        hLayout->addWidget(fileNameLabel);

        if (labelText == labelTexts.last()) {
            hLayout->addSpacing(2);
            hLayout->addWidget(nameEditIcon);
        } else if (fileNameLabel->fontMetrics().horizontalAdvance(labelText) > (rect.width() - 10)) {
            fileNameLabel->setFixedWidth(rect.width());
        }

        textShowLayout->addLayout(hLayout);
        hLayout->addStretch(1);
    }

    textShowLayout->setContentsMargins(0, 0, 0, 0);
    textShowLayout->setSpacing(0);
    textShowFrame->setLayout(textShowLayout);
    textShowLayout->addStretch(1);

    textShowFrame->setFixedHeight(textHeight + 15);

    if (this->count() == 1) {
        this->addWidget(textShowFrame);
    } else {
        this->insertWidget(1, textShowFrame);
    }
    this->setCurrentIndex(1);
    this->setFixedHeight(textShowFrame->height());
}

void EditStackedWidget::renameFile()
{
    QFileInfo info(filerUrl.path());
    fileNameEdit->setPlainText(info.fileName());
    this->setCurrentIndex(0);
    fileNameEdit->setFixedHeight(textShowFrame->height());
    fileNameEdit->setFocus();

    fileNameEdit->selectAll();
    int endPos = fileNameEdit->toPlainText().length();

    QTextCursor cursor = fileNameEdit->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    fileNameEdit->setTextCursor(cursor);
}

void EditStackedWidget::showTextShowFrame()
{
    QString newName = fileNameEdit->toPlainText();

    if (newName.trimmed().isEmpty()) {
        fileNameEdit->setIsCanceled(true);
    }

    if (fileNameEdit->isCanceled())
        initTextShowFrame(newName);
    else {
        QUrl oldUrl = filerUrl;
        QList<QUrl> urls {};
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << oldUrl, &urls);
        if (ok && !urls.isEmpty())
            oldUrl = urls.first();

        QUrl newUrl = QUrl::fromLocalFile(QFileInfo(oldUrl.path()).absolutePath() + "/" + newName);

        if (oldUrl == newUrl) {
            this->setCurrentIndex(1);
            return;
        }

        initTextShowFrame(newName);
        dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, this->topLevelWidget()->winId(), oldUrl, newUrl, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
        filerUrl = newUrl;
        emit selectUrlRenamed(newUrl);
    }
}

void EditStackedWidget::selectFile(const QUrl &url)
{
    filerUrl = url;
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (!info.isNull()) {
        initTextShowFrame(info->displayOf(DisPlayInfoType::kFileDisplayName));
        if (!info->canAttributes(CanableInfoType::kCanRename)) {
            nameEditIcon->hide();
        } else {
            nameEditIcon->show();
        }
    }
}

void EditStackedWidget::mouseProcess(QMouseEvent *event)
{
    if (fileNameEdit->isVisible()) {
        if (event->button() != Qt::RightButton) {
            fileNameEdit->setIsCanceled(fileNameEdit->toPlainText().isEmpty());
            emit fileNameEdit->editFinished();
        }
    }
}
