/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "filedialogstatusbar.h"

#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWindow>
#include <QMimeDatabase>
#include <QTimer>
#include <QDebug>
#include <QFontMetrics>

DIALOGCORE_USE_NAMESPACE

FileDialogStatusBar::FileDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
    initializeUi();
}

void FileDialogStatusBar::setMode(FileDialogStatusBar::Mode mode)
{
    if (curMode == mode)
        return;

    curMode = mode;

    QString acButton = mode == kSave ? tr("Save", "button") : tr("Open", "button");
    curAcceptButton->setText(acButton);
    curAcceptButton->setObjectName(acButton);

    updateLayout();

    if (titleLabel->text().isEmpty()) {
        QString text = mode == kSave ? tr("Save File", "button") : tr("Open File", "button");
        titleLabel->setText(text);
        titleLabel->setObjectName(text);
    }
}

void FileDialogStatusBar::setComBoxItems(const QStringList &list)
{
    bool visible = filtersComboBox->isVisible();

    filtersComboBox->clear();
    filtersComboBox->addItems(list);

    if (curMode == kOpen) {
        filtersComboBox->setHidden(list.isEmpty());
        filtersLabel->setHidden(list.isEmpty());
    } else {
        if (visible == list.isEmpty()) {
            updateLayout();
        }
    }
}

QComboBox *FileDialogStatusBar::comboBox() const
{
    return filtersComboBox;
}

QLineEdit *FileDialogStatusBar::lineEdit() const
{
    return fileNameEdit;
}

QPushButton *FileDialogStatusBar::acceptButton() const
{
    return curAcceptButton;
}

QPushButton *FileDialogStatusBar::rejectButton() const
{
    return curRejectButton;
}

void FileDialogStatusBar::addLineEdit(QLabel *label, QLineEdit *edit)
{
    customLineEditList << qMakePair(label, edit);
}

QString FileDialogStatusBar::getLineEditValue(const QString &text) const
{
    auto iter = std::find_if(customLineEditList.begin(), customLineEditList.end(),
                             [text](const QPair<QLabel *, QLineEdit *> &i) {
                                 return i.first->text() == text;
                             });

    if (iter != customLineEditList.end()) {
        return iter->second->text();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allLineEditsValue() const
{
    QVariantMap map;

    for (auto i : customLineEditList)
        map[i.first->text()] = i.second->text();

    return map;
}

void FileDialogStatusBar::addComboBox(QLabel *label, QComboBox *box)
{
    customComboBoxList << qMakePair(label, box);
}

QString FileDialogStatusBar::getComboBoxValue(const QString &text) const
{
    auto iter = std::find_if(customComboBoxList.begin(), customComboBoxList.end(),
                             [text](const QPair<QLabel *, QComboBox *> &i) {
                                 return i.first->text() == text;
                             });

    if (iter != customComboBoxList.end()) {
        return iter->second->currentText();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allComboBoxsValue() const
{
    QVariantMap map;

    for (auto i : customComboBoxList)
        map[i.first->text()] = i.second->currentText();

    return map;
}

void FileDialogStatusBar::beginAddCustomWidget()
{
    for (auto i : customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    for (auto i : customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    customComboBoxList.clear();
    customLineEditList.clear();
}

void FileDialogStatusBar::endAddCustomWidget()
{
    updateLayout();
}

void FileDialogStatusBar::onWindowTitleChanged(const QString &title)
{
    if (title.isEmpty())
        return;

    QFontMetrics fontWidth(titleLabel->font());
    QString elideTitle = fontWidth.elidedText(title, Qt::ElideMiddle, UISize::kTitleMaxWidth);
    titleLabel->setText(elideTitle);

    titleLabel->setObjectName(title);
}

void FileDialogStatusBar::initializeUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameShape(QFrame::NoFrame);

    titleLabel = new QLabel(this);
    QString labelName = tr("File Name");
    QString labelFilters = tr("Format");
    fileNameLabel = new QLabel(labelName, this);
    filtersLabel = new QLabel(labelFilters, this);

    fileNameLabel->setObjectName(labelName);
    filtersLabel->setObjectName(labelFilters);

    fileNameEdit = new QLineEdit(this);
    filtersComboBox = new QComboBox(this);

    fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    fileNameEdit->installEventFilter(this);
    filtersComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    curAcceptButton = new QPushButton(this);
    curRejectButton = new QPushButton(tr("Cancel", "button"), this);

    curRejectButton->setObjectName(tr("Cancel", "button"));

    curAcceptButton->setMinimumWidth(130);
    curRejectButton->setMinimumWidth(130);

    curAcceptButton->setObjectName("FileDialogStatusBarAcceptButton");

    contentLayout = new QHBoxLayout();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addLayout(contentLayout);
}

void FileDialogStatusBar::updateLayout()
{
    if (curMode == kUnknow)
        return;

    while (contentLayout->count() > 0)
        delete contentLayout->takeAt(0);

    fileNameLabel->hide();
    filtersLabel->hide();

    fileNameEdit->hide();
    filtersComboBox->hide();

    contentLayout->addSpacing(10);

    int widgetCount = customComboBoxList.count() + customLineEditList.count();

    if (curMode == kSave) {
        ++widgetCount;
    }

    if (filtersComboBox->count() > 0)
        ++widgetCount;

    if (widgetCount <= 1) {
        int addedWidgetCount = 1;

        if (!customLineEditList.isEmpty()) {
            contentLayout->addWidget(customLineEditList.first().first);
            contentLayout->addSpacing(10);
            contentLayout->addWidget(customLineEditList.first().second, 1);
        } else if (!customComboBoxList.isEmpty()) {
            contentLayout->addWidget(customComboBoxList.first().first);
            contentLayout->addSpacing(10);
            contentLayout->addWidget(customComboBoxList.first().second, 1);
        } else {
            addedWidgetCount = 0;
        }

        if (curMode == kOpen) {
            if (addedWidgetCount == 0) {
                contentLayout->addWidget(filtersLabel);
                contentLayout->addSpacing(10);
                contentLayout->addWidget(filtersComboBox, 1);
            }

            contentLayout->addSpacing(10);
            contentLayout->addStretch();
            contentLayout->addWidget(curRejectButton);
            contentLayout->addWidget(curAcceptButton);

            if (filtersComboBox->count() > 0) {
                filtersLabel->show();
                filtersComboBox->show();
            }

            return;
        }

        if (filtersComboBox->count() <= 0) {
            if (addedWidgetCount == 0) {
                contentLayout->addWidget(fileNameLabel);
                contentLayout->addSpacing(10);
                contentLayout->addWidget(fileNameEdit);
            }

            contentLayout->addSpacing(10);
            contentLayout->addWidget(curRejectButton);
            contentLayout->addWidget(curAcceptButton);

            fileNameLabel->show();
            fileNameEdit->show();

            return;
        }
    }

    QVBoxLayout *labelLayout = new QVBoxLayout();
    QVBoxLayout *centerLayout = new QVBoxLayout();

    if (curMode == kSave) {
        labelLayout->addWidget(fileNameLabel);
        centerLayout->addWidget(fileNameEdit);
        fileNameLabel->show();
        fileNameEdit->show();
    }

    for (auto i : customLineEditList) {
        labelLayout->addWidget(i.first);
        centerLayout->addWidget(i.second);
    }

    if (filtersComboBox->count() > 0) {
        labelLayout->addWidget(filtersLabel);
        centerLayout->addWidget(filtersComboBox);
        filtersLabel->show();
        filtersComboBox->show();
    }

    for (auto i : customComboBoxList) {
        labelLayout->addWidget(i.first);
        centerLayout->addWidget(i.second);
    }

    QVBoxLayout *buttonLayout = new QVBoxLayout();

    buttonLayout->addStretch();
    buttonLayout->addWidget(curRejectButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    buttonLayout->addWidget(curAcceptButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    contentLayout->addLayout(labelLayout);
    contentLayout->addSpacing(10);
    contentLayout->addLayout(centerLayout);
    contentLayout->addSpacing(10);
    contentLayout->addLayout(buttonLayout);
}

void FileDialogStatusBar::showEvent(QShowEvent *event)
{
    const QString &title = window()->windowTitle();

    if (!title.isEmpty()) {
        titleLabel->setText(title);

        titleLabel->setObjectName(title);
    }
    connect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    if (fileNameEdit->isVisible())
        fileNameEdit->setFocus();

    return QFrame::showEvent(event);
}

void FileDialogStatusBar::hideEvent(QHideEvent *event)
{
    disconnect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    return QFrame::hideEvent(event);
}

bool FileDialogStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != fileNameEdit)
        return false;

    if (event->type() == QEvent::FocusIn) {
        QTimer::singleShot(10, this, [this]() {
            QMimeDatabase db;
            const QString &name = fileNameEdit->text();
            const QString &suffix = db.suffixForFileName(name);

            if (suffix.isEmpty())
                fileNameEdit->selectAll();
            else
                fileNameEdit->setSelection(0, name.length() - suffix.length() - 1);
        });
    } else if (event->type() == QEvent::Show) {
        QTimer::singleShot(500, this, [this]() {
            fileNameEdit->setFocus();
        });
    }

    return false;
}
