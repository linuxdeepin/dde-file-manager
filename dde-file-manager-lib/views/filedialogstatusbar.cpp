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

    m_fileNameEdit->setFixedHeight(24);
    m_filtersComboBox->setFixedHeight(24);

    m_fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fileNameEdit->installEventFilter(this);
    m_filtersComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_acceptButton = new QPushButton(this);
    m_rejectButton = new QPushButton(tr("Cancel"), this);

    m_acceptButton->setFixedSize(100, 24);
    m_acceptButton->setObjectName("FileDialogStatusBarAcceptButton");
    m_rejectButton->setFixedSize(100, 24);

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

    if (m_mode == Open) {
        m_contentLayout->addWidget(m_filtersLabel);
        m_contentLayout->addSpacing(10);
        m_contentLayout->addWidget(m_filtersComboBox, 1);
        m_contentLayout->addSpacing(10);
        m_contentLayout->addStretch();
        m_contentLayout->addWidget(m_rejectButton);
        m_contentLayout->addWidget(m_acceptButton);

        if (m_filtersComboBox->count() > 0) {
            m_filtersLabel->show();
            m_filtersComboBox->show();
        }

        return;
    }

    if (m_filtersComboBox->count() <= 0) {
        m_contentLayout->addWidget(m_fileNameLabel);
        m_contentLayout->addSpacing(10);
        m_contentLayout->addWidget(m_fileNameEdit);
        m_contentLayout->addSpacing(10);
        m_contentLayout->addWidget(m_rejectButton);
        m_contentLayout->addWidget(m_acceptButton);

        m_fileNameLabel->show();
        m_fileNameEdit->show();

        return;
    }

    QVBoxLayout *label_layout = new QVBoxLayout();

    label_layout->addWidget(m_fileNameLabel);
    label_layout->addWidget(m_filtersLabel);

    QVBoxLayout *center_layout = new QVBoxLayout();

    center_layout->addWidget(m_fileNameEdit);
    center_layout->addWidget(m_filtersComboBox);

    QVBoxLayout *button_layout = new QVBoxLayout();

    button_layout->addWidget(m_rejectButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    button_layout->addWidget(m_acceptButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_contentLayout->addLayout(label_layout);
    m_contentLayout->addSpacing(10);
    m_contentLayout->addLayout(center_layout);
    m_contentLayout->addSpacing(10);
    m_contentLayout->addLayout(button_layout);

    m_fileNameLabel->show();
    m_filtersLabel->show();

    m_fileNameEdit->show();
    m_filtersComboBox->show();
}

void FileDialogStatusBar::onWindowTitleChanged(const QString &title)
{
    if (title.isEmpty())
        return;

    m_titleLabel->setText(title);
}
