/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "tageditor.h"
#include "utils/tagmanager.h"

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

TagEditor::TagEditor(QWidget *const parent)
    : DArrowRectangle { DArrowRectangle::ArrowTop, parent }
{
    initializeWidgets();
    initializeParameters();
    initializeLayout();
    initializeConnect();

    installEventFilter(this);

    setWindowFlags(Qt::Tool);
}

void TagEditor::setFocusOutSelfClosing(bool value) noexcept
{
    bool excepted { !value };
    flagForShown.compare_exchange_strong(excepted, value, std::memory_order_release);
}

void TagEditor::setFilesForTagging(const QList<QUrl> &files)
{
    this->files = files;
}

void TagEditor::setDefaultCrumbs(const QStringList &list)
{
    isSettingDefault = true;

    for (const QString &crumb : list)
        crumbEdit->appendCrumb(crumb);

    isSettingDefault = false;
}

void TagEditor::onFocusOut()
{
    if (flagForShown.load(std::memory_order_acquire)) {
        processTags();
        close();
    }
}

void TagEditor::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape: {
        this->processTags();
        event->accept();
        this->close();
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        QObject::disconnect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);
        processTags();
        event->accept();
        close();
        break;
    }
    default:
        break;
    }

    DArrowRectangle::keyPressEvent(event);
}

void TagEditor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void TagEditor::initializeWidgets()
{
    crumbEdit = new DCrumbEdit;
    promptLabel = new QLabel(tr("Input tag info, such as work, family. A comma is used between two tags."));
    totalLayout = new QVBoxLayout;
    backgroundFrame = new QFrame;
}

void TagEditor::initializeParameters()
{
    crumbEdit->setFixedSize(140, 40);
    crumbEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setCrumbReadOnly(true);
    crumbEdit->setCrumbRadius(2);

    promptLabel->setFixedWidth(140);
    promptLabel->setWordWrap(true);
    backgroundFrame->setContentsMargins(QMargins { 0, 0, 0, 0 });

    setMargin(0);
    setFixedWidth(140);
    setFocusPolicy(Qt::StrongFocus);
    setBorderColor(QColor { "#ffffff" });
    setBackgroundColor(QColor { "#ffffff" });
    setWindowFlags(Qt::FramelessWindowHint);
}

void TagEditor::initializeLayout()
{
    totalLayout->addStretch(1);
    totalLayout->addWidget(crumbEdit);
    totalLayout->addSpacing(8);
    totalLayout->addWidget(promptLabel);
    totalLayout->addStretch(1);

    backgroundFrame->setLayout(totalLayout);
    setContent(backgroundFrame);
}

void TagEditor::initializeConnect()
{
    QObject::connect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);

    QObject::connect(crumbEdit, &DCrumbEdit::crumbListChanged, this, [=] {
        if (!isSettingDefault)
            processTags();
    });
}

void TagEditor::processTags()
{
    QList<QString> tags = crumbEdit->crumbList();
    QList<QUrl> tempFiles = files;

    TagManager::instance()->setTagsForFiles(tags, tempFiles);
}
