/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "durl.h"
#include "singleton.h"
#include "dfmeventdispatcher.h"
#include "connecttoserverdialog.h"
#include "../views/dfilemanagerwindow.h"
#include "../controllers/searchhistroymanager.h"
#include "../interfaces/dfmsettings.h"
#include "../interfaces/dfmapplication.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QIcon>
#include <QDebug>
#include <QStringList>
#include <QStringListModel>
#include <QScrollBar>
#include <QDir>
#include <QLabel>
#include <QCompleter>
#include <DIconButton>
#include <DListView>
#include <QWindow>

ConnectToServerDialog::ConnectToServerDialog(QWidget *parent) : DDialog(parent)
{
    setWindowTitle(tr("Connect to Server"));
    initUI();
    initConnect();
}

void ConnectToServerDialog::onButtonClicked(const int &index)
{
    // connect to server
    if(index == ConnectButton) {
        QString text = m_serverComboBox->currentText();
        if (text.isEmpty()) {
            close();
            return;
        }

        QWidget *fileWindow = qobject_cast<QWidget *>(parent());

        const QString &currentDir = QDir::currentPath();
        const DUrl &currentUrl = qobject_cast<DFileManagerWindow*>(fileWindow->topLevelWidget())->currentUrl();

        if (currentUrl.isLocalFile()) {
            QDir::setCurrent(currentUrl.toLocalFile());
        }

        DUrl inputUrl = DUrl::fromUserInput(text, false); ///###: here, judge whether the text is a local file path.

        QDir::setCurrent(currentDir);

        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, inputUrl, fileWindow->window());

        //add search history list
        SearchHistroyManager *historyManager = Singleton<SearchHistroyManager>::instance();
        if (!historyManager->toStringList().contains(text)) {
            historyManager->writeIntoSearchHistory(text);
        }
    }
    close();
}

void ConnectToServerDialog::onAddButtonClicked()
{
    QStringList serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();

    const QString &text = m_serverComboBox->currentText();
    if (!text.isEmpty() &&
            !serverList.contains(text)) {
        if(!m_collectionServerView->addItem(text)) {
            qWarning() << "add server failed, server: " << text;
        } else {
            serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();
            const QModelIndex modelIndex = m_collectionServerView->model()->index(serverList.indexOf(text),
                                                                                  0,
                                                                                  m_collectionServerView->currentIndex().parent());
            m_collectionServerView->setCurrentIndex(modelIndex);

            QStringList dataList;
            const QList<QVariant> &serverData = DFMApplication::genericSetting()->value("ConnectServer", "URL").toList();
            for (const QVariant &data : serverData) {
                dataList << data.toString();
            }

            if (!dataList.contains(text)) {
                dataList << text;
                DFMApplication::genericSetting()->setValue("ConnectServer", "URL", dataList);
            }
        }
    }
}

void ConnectToServerDialog::onDelButtonClicked()
{
    const QStringList &serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();
    const QString &text = m_serverComboBox->currentText();
    if (!text.isEmpty() &&
            serverList.contains(text)) {
        if (!m_collectionServerView->removeItem(serverList.indexOf(text))) {
            qWarning() << "remove server failed, server: " << text;
        } else {
            const QList<QVariant> &serverData = DFMApplication::genericSetting()->value("ConnectServer", "URL").toList();

            QStringList dataList;
            for (const QVariant &data : serverData) {
                dataList << data.toString();
            }

            if (dataList.contains(text)) {
                dataList.removeOne(text);
                DFMApplication::genericSetting()->setValue("ConnectServer", "URL", dataList);
            }

            const QModelIndex &currentIndex = m_collectionServerView->currentIndex();
            if (currentIndex.isValid()) {
                m_serverComboBox->setCurrentText(currentIndex.data().toString());
            } else {
                m_serverComboBox->clearEditText();
            }
        }
    }
}

void ConnectToServerDialog::initUI()
{
    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setFixedSize(476, 380);

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Connect","button"));
    addButton(buttonTexts[CannelButton], false);
    addButton(buttonTexts[ConnectButton], true, DDialog::ButtonRecommend);

    QFrame *contentFrame = new QFrame(this);
    m_serverComboBox = new QComboBox();
    m_addButton = new DIconButton(nullptr);
    m_delButton = new DIconButton(nullptr);
    QLabel * collectionLabel = new QLabel(tr("My Favorites:"));
    m_collectionServerView = new DListView();

    m_addButton->setFixedSize(44, 44);
    m_delButton->setFixedSize(44, 44);
    //collectionLabel->setFixedSize(98, 20);

    m_addButton->setIcon(QIcon::fromTheme("dfm_add_server"));
    m_addButton->setIconSize({44, 44});
    m_addButton->setFlat(true);
    m_delButton->setIcon(QIcon::fromTheme("dfm_del_server"));
    m_delButton->setIconSize({44, 44});
    m_delButton->setFlat(true);

    QHBoxLayout *comboButtonLayout = new QHBoxLayout();
    comboButtonLayout->addWidget(m_serverComboBox, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(6);
    comboButtonLayout->addWidget(m_addButton, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(2);
    comboButtonLayout->addWidget(m_delButton, 0, Qt::AlignVCenter);
    comboButtonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->addLayout(comboButtonLayout);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(collectionLabel, 0, Qt::AlignVCenter);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_collectionServerView, 0, Qt::AlignVCenter);
    contentLayout->setContentsMargins(5, 0, 0, 0);

    contentFrame->setLayout(contentLayout);
    addContent(contentFrame);

    const QStringList &stringList = Singleton<SearchHistroyManager>::instance()->toStringList();

    QCompleter *completer = new QCompleter(stringList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setMaxVisibleItems(10);

    m_serverComboBox->setEditable(true);
    m_serverComboBox->addItems(stringList);
    m_serverComboBox->insertItem(m_serverComboBox->count(), tr("Clear History"));
    m_serverComboBox->setEditable(true);
    m_serverComboBox->setMaxVisibleItems(10);
    m_serverComboBox->setCompleter(completer);
    m_serverComboBox->clearEditText();

    m_collectionServerView->setViewportMargins(0, 0, m_collectionServerView->verticalScrollBar()->sizeHint().width(), 0);
    m_collectionServerView->setVerticalScrollMode(DListView::ScrollPerPixel);
    m_collectionServerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_collectionServerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_collectionServerView->setResizeMode(DListView::Fixed);
    m_collectionServerView->setAlternatingRowColors(true);
    m_collectionServerView->setUniformItemSizes(true);
    m_collectionServerView->setItemSize({m_collectionServerView->width(), 36});
    m_collectionServerView->setItemMargins({0, 0, 0, 0});
    m_collectionServerView->setItemSpacing(1);
    m_collectionServerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_collectionServerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_collectionServerView->setFrameShape(QFrame::Shape::NoFrame);

    QStringList dataList;
    const QList<QVariant> &serverData = DFMApplication::genericSetting()->value("ConnectServer", "URL").toList();
    for (const QVariant &data : serverData) {
        dataList << data.toString();
    }

    QStringListModel *listModel = new QStringListModel(this);
    m_collectionServerView->setModel(listModel);
    listModel->setStringList(dataList);

    setContentsMargins(0,0,0,0);
}

void ConnectToServerDialog::initConnect()
{
    //QComboBox clear history
    connect(m_serverComboBox, &QComboBox::currentTextChanged, this, [=](const QString &string){
        if (string == m_serverComboBox->itemText(m_serverComboBox->count() - 1)) {
            QSignalBlocker blocker(m_serverComboBox);
            Q_UNUSED(blocker)

            m_serverComboBox->clear();
            m_serverComboBox->addItem(tr("Clear History"));
            m_serverComboBox->clearEditText();
            m_serverComboBox->completer()->setModel(new QStringListModel());

            Singleton<SearchHistroyManager>::instance()->clearHistory();
        }
    });

    connect(m_addButton, &DIconButton::clicked, this, &ConnectToServerDialog::onAddButtonClicked);
    connect(m_delButton, &DIconButton::clicked, this, &ConnectToServerDialog::onDelButtonClicked);
    connect(m_collectionServerView, &DListView::clicked, this, [=](const QModelIndex &index){
        if (index.data().toString() != m_serverComboBox->currentText()) {
            m_serverComboBox->setCurrentText(index.data().toString());
        }
    });
}
