/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "filedialogstatusbar.h"
#include "dfmglobal.h"

#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWindow>
#include <QMimeDatabase>
#include <QTimer>
#include <QDebug>

FileDialogStatusBar::FileDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameShape(QFrame::NoFrame);

    m_titleLabel = new QLabel(this);

    m_fileNameLabel = new QLabel(tr("File Name"), this);
    m_filtersLabel = new QLabel(tr("Format"), this);

    m_fileNameLabel->setObjectName("FileDialogStatusBarContentLabel");
    m_filtersLabel->setObjectName("FileDialogStatusBarContentLabel");

    m_fileNameEdit = new QLineEdit(this);
    m_filtersComboBox = new QComboBox(this);

    m_fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fileNameEdit->installEventFilter(this);
    m_filtersComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_acceptButton = new QPushButton(this);
    m_rejectButton = new QPushButton(tr("Cancel"), this);

    m_acceptButton->setMinimumWidth(130);
    m_rejectButton->setMinimumWidth(130);

    m_acceptButton->setObjectName("FileDialogStatusBarAcceptButton");

    m_contentLayout = new QHBoxLayout();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addLayout(m_contentLayout);
}

void FileDialogStatusBar::setMode(FileDialogStatusBar::Mode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;

    m_acceptButton->setText(mode == Save ? tr("Save") : tr("Open"));

    updateLayout();

    if (m_titleLabel->text().isEmpty())
        m_titleLabel->setText(mode == Save ? tr("Save File") : tr("Open File"));
}

void FileDialogStatusBar::setComBoxItems(const QStringList &list)
{
    bool visible = m_filtersComboBox->isVisible();

    m_filtersComboBox->clear();
    m_filtersComboBox->addItems(list);

    if (m_mode == Open) {
        m_filtersComboBox->setHidden(list.isEmpty());
        m_filtersLabel->setHidden(list.isEmpty());
    } else {
        if (visible == list.isEmpty()) {
            updateLayout();
        }
    }
}

QComboBox *FileDialogStatusBar::comboBox() const
{
    return m_filtersComboBox;
}

QLineEdit *FileDialogStatusBar::lineEdit() const
{
    return m_fileNameEdit;
}

QPushButton *FileDialogStatusBar::acceptButton() const
{
    return m_acceptButton;
}

QPushButton *FileDialogStatusBar::rejectButton() const
{
    return m_rejectButton;
}

void FileDialogStatusBar::addLineEdit(QLabel *label, QLineEdit *edit)
{
    label->setObjectName("FileDialogStatusBarContentLabel");
    m_customLineEditList << qMakePair(label, edit);
}

QString FileDialogStatusBar::getLineEditValue(const QString &text) const
{
    auto iter = std::find_if(m_customLineEditList.begin(), m_customLineEditList.end(),
    [text](const QPair<QLabel *, QLineEdit *> &i) {
        return i.first->text() == text;
    });

    if (iter != m_customLineEditList.end()) {
        return iter->second->text();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allLineEditsValue() const
{
    QVariantMap map;

    for (auto i : m_customLineEditList)
        map[i.first->text()] = i.second->text();

    return map;
}

void FileDialogStatusBar::addComboBox(QLabel *label, QComboBox *box)
{
    label->setObjectName("FileDialogStatusBarContentLabel");
    m_customComboBoxList << qMakePair(label, box);
}

QString FileDialogStatusBar::getComboBoxValue(const QString &text) const
{
    auto iter = std::find_if(m_customComboBoxList.begin(), m_customComboBoxList.end(),
    [text](const QPair<QLabel *, QComboBox *> &i) {
        return i.first->text() == text;
    });

    if (iter != m_customComboBoxList.end()) {
        return iter->second->currentText();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allComboBoxsValue() const
{
    QVariantMap map;

    for (auto i : m_customComboBoxList)
        map[i.first->text()] = i.second->currentText();

    return map;
}

void FileDialogStatusBar::beginAddCustomWidget()
{
    for (auto i : m_customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    for (auto i : m_customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    m_customComboBoxList.clear();
    m_customLineEditList.clear();
}

void FileDialogStatusBar::endAddCustomWidget()
{
    updateLayout();
}

void FileDialogStatusBar::showEvent(QShowEvent *event)
{
    const QString &title = window()->windowTitle();

    if (!title.isEmpty())
        m_titleLabel->setText(title);

    connect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    if (m_fileNameEdit->isVisible())
        m_fileNameEdit->setFocus();

    return QFrame::showEvent(event);
}

void FileDialogStatusBar::hideEvent(QHideEvent *event)
{
    disconnect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    return QFrame::hideEvent(event);
}

bool FileDialogStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_fileNameEdit)
        return false;

    if (event->type() == QEvent::FocusIn) {
        TIMER_SINGLESHOT_OBJECT(this, 10, {
            QMimeDatabase db;
            const QString &name = m_fileNameEdit->text();
            const QString &suffix = db.suffixForFileName(name);

            if (suffix.isEmpty())
                m_fileNameEdit->selectAll();
            else
                m_fileNameEdit->setSelection(0, name.length() - suffix.length() - 1);
        }, this)
    } else if (event->type() == QEvent::Show) {
        TIMER_SINGLESHOT_OBJECT(this, 500, m_fileNameEdit->setFocus(), this);
    }

    return false;
}

void FileDialogStatusBar::updateLayout()
{
    if (m_mode == Unknow)
        return;

    while (m_contentLayout->count() > 0)
        delete m_contentLayout->takeAt(0);

    m_fileNameLabel->hide();
    m_filtersLabel->hide();

    m_fileNameEdit->hide();
    m_filtersComboBox->hide();

    m_contentLayout->addSpacing(10);

    int widget_count = m_customComboBoxList.count() + m_customLineEditList.count();

    if (m_mode == Save) {
        ++widget_count;
    }

    if (m_filtersComboBox->count() > 0)
        ++widget_count;

    if (widget_count <= 1) {
        int added_widget_count = 1;

        if (!m_customLineEditList.isEmpty()) {
            m_contentLayout->addWidget(m_customLineEditList.first().first);
            m_contentLayout->addSpacing(10);
            m_contentLayout->addWidget(m_customLineEditList.first().second, 1);
        } else if (!m_customComboBoxList.isEmpty()) {
            m_contentLayout->addWidget(m_customComboBoxList.first().first);
            m_contentLayout->addSpacing(10);
            m_contentLayout->addWidget(m_customComboBoxList.first().second, 1);
        } else {
            added_widget_count = 0;
        }

        if (m_mode == Open) {
            if (added_widget_count == 0) {
                m_contentLayout->addWidget(m_filtersLabel);
                m_contentLayout->addSpacing(10);
                m_contentLayout->addWidget(m_filtersComboBox, 1);
            }

            m_contentLayout->addSpacing(10);
            m_contentLayout->addStretch();
            m_contentLayout->addWidget(m_rejectButton);
            m_contentLayout->addWidget(m_acceptButton);

            if (m_filtersComboBox->count() > 0) {
                m_filtersLabel->show();
                m_filtersComboBox->show();
                ++widget_count;
            }

            return;
        }

        if (m_filtersComboBox->count() <= 0) {
            if (added_widget_count == 0) {
                m_contentLayout->addWidget(m_fileNameLabel);
                m_contentLayout->addSpacing(10);
                m_contentLayout->addWidget(m_fileNameEdit);
            }

            m_contentLayout->addSpacing(10);
            m_contentLayout->addWidget(m_rejectButton);
            m_contentLayout->addWidget(m_acceptButton);

            m_fileNameLabel->show();
            m_fileNameEdit->show();

            return;
        }
    }

    QVBoxLayout *label_layout = new QVBoxLayout();
    QVBoxLayout *center_layout = new QVBoxLayout();

    if (m_mode == Save) {
        label_layout->addWidget(m_fileNameLabel);
        center_layout->addWidget(m_fileNameEdit);
        m_fileNameLabel->show();
        m_fileNameEdit->show();
    }

    for (auto i : m_customLineEditList) {
        label_layout->addWidget(i.first);
        center_layout->addWidget(i.second);
    }

    if (m_filtersComboBox->count() > 0) {
        label_layout->addWidget(m_filtersLabel);
        center_layout->addWidget(m_filtersComboBox);
        m_filtersLabel->show();
        m_filtersComboBox->show();
    }

    for (auto i : m_customComboBoxList) {
        label_layout->addWidget(i.first);
        center_layout->addWidget(i.second);
    }

    QVBoxLayout *button_layout = new QVBoxLayout();

    button_layout->addStretch();
    button_layout->addWidget(m_rejectButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    button_layout->addWidget(m_acceptButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_contentLayout->addLayout(label_layout);
    m_contentLayout->addSpacing(10);
    m_contentLayout->addLayout(center_layout);
    m_contentLayout->addSpacing(10);
    m_contentLayout->addLayout(button_layout);
}

void FileDialogStatusBar::onWindowTitleChanged(const QString &title)
{
    if (title.isEmpty())
        return;

    m_titleLabel->setText(title);
}
