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
#include "connecttoserverdialog.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include "dfm-base/utils/windowutils.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

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
#include <QWindow>
#include <DIconButton>
#include <DListView>

DFMBASE_USE_NAMESPACE
DPTITLEBAR_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

ConnectToServerDialog::ConnectToServerDialog(const QUrl &url, QWidget *parent)
    : DDialog(parent), currentUrl(url)
{
    setWindowTitle(tr("Connect to Server"));
    initializeUi();
    initConnect();
}

void ConnectToServerDialog::onButtonClicked(const int &index)
{
    // connect to server
    if (index == kConnectButton) {
        QString text { serverComboBox->currentText() };
        if (text.isEmpty()) {
            close();
            return;
        }

        const QString &currentDir = QDir::currentPath();

        if (currentUrl.isLocalFile())
            QDir::setCurrent(currentUrl.toLocalFile());

        QWidget *fileWindow = qobject_cast<QWidget *>(parent());
        TitleBarHelper::handlePressed(fileWindow, text);
        QDir::setCurrent(currentDir);

        // add search history list
        if (!SearchHistroyManager::instance()->toStringList().contains(text))
            SearchHistroyManager::instance()->writeIntoSearchHistory(text);
    }
    close();
}

void ConnectToServerDialog::onCurrentTextChanged(const QString &string)
{
    if (string == serverComboBox->itemText(serverComboBox->count() - 1)) {
        QSignalBlocker blocker(serverComboBox);
        Q_UNUSED(blocker)

        serverComboBox->clear();
        serverComboBox->addItem(tr("Clear History"));
        serverComboBox->clearEditText();
        serverComboBox->completer()->setModel(new QStringListModel());

        SearchHistroyManager::instance()->clearHistory();
    }
}

void ConnectToServerDialog::initializeUi()
{
    if (WindowUtils::isWayLand()) {
        // 设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        // this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setFixedSize(476, 380);

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Connect", "button"));
    addButton(buttonTexts[kCannelButton], false);
    addButton(buttonTexts[kConnectButton], true, DDialog::ButtonRecommend);

    QFrame *contentFrame = new QFrame(this);
    serverComboBox = new QComboBox();
    theAddButton = new DIconButton(nullptr);
    theDelButton = new DIconButton(nullptr);
    QLabel *collectionLabel = new QLabel(tr("My Favorites:"));
    collectionServerView = new DListView();

    theAddButton->setFixedSize(44, 44);
    theDelButton->setFixedSize(44, 44);
    //collectionLabel->setFixedSize(98, 20);

    theAddButton->setIcon(QIcon::fromTheme("dfm_add_server"));
    theAddButton->setIconSize({ 44, 44 });
    theAddButton->setFlat(true);
    theDelButton->setIcon(QIcon::fromTheme("dfm_del_server"));
    theDelButton->setIconSize({ 44, 44 });
    theDelButton->setFlat(true);

    QHBoxLayout *comboButtonLayout = new QHBoxLayout();
    comboButtonLayout->addWidget(serverComboBox, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(6);
    comboButtonLayout->addWidget(theAddButton, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(2);
    comboButtonLayout->addWidget(theDelButton, 0, Qt::AlignVCenter);
    comboButtonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->addLayout(comboButtonLayout);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(collectionLabel, 0, Qt::AlignVCenter);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(collectionServerView, 0, Qt::AlignVCenter);
    contentLayout->setContentsMargins(5, 0, 0, 0);

    contentFrame->setLayout(contentLayout);
    addContent(contentFrame);

    const QStringList &stringList = SearchHistroyManager::instance()->toStringList();

    QCompleter *completer = new QCompleter(stringList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setMaxVisibleItems(10);

    serverComboBox->setEditable(true);
    serverComboBox->addItems(stringList);
    serverComboBox->insertItem(serverComboBox->count(), tr("Clear History"));
    serverComboBox->setEditable(true);
    serverComboBox->setMaxVisibleItems(10);
    serverComboBox->setCompleter(completer);
    serverComboBox->clearEditText();

    collectionServerView->setViewportMargins(0, 0, collectionServerView->verticalScrollBar()->sizeHint().width(), 0);
    collectionServerView->setVerticalScrollMode(DListView::ScrollPerPixel);
    collectionServerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    collectionServerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    collectionServerView->setResizeMode(DListView::Fixed);
    collectionServerView->setAlternatingRowColors(true);
    collectionServerView->setUniformItemSizes(true);
    collectionServerView->setItemSize({ collectionServerView->width(), 36 });
    collectionServerView->setItemMargins({ 0, 0, 0, 0 });
    collectionServerView->setItemSpacing(1);
    collectionServerView->setSelectionBehavior(QAbstractItemView::SelectRows);
    collectionServerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    collectionServerView->setFrameShape(QFrame::Shape::NoFrame);

    QStringList dataList;
    const QList<QVariant> &serverData = Application::genericSetting()->value("ConnectServer", "URL").toList();
    for (const QVariant &data : serverData) {
        dataList << data.toString();
    }

    QStringListModel *listModel = new QStringListModel(this);
    collectionServerView->setModel(listModel);
    listModel->setStringList(dataList);

    setContentsMargins(0, 0, 0, 0);
}

void ConnectToServerDialog::initConnect()
{
    //QComboBox clear history
    connect(serverComboBox, &QComboBox::currentTextChanged, this, &ConnectToServerDialog::onCurrentTextChanged);

    connect(theAddButton, &DIconButton::clicked, this, &ConnectToServerDialog::onAddButtonClicked);
    connect(theDelButton, &DIconButton::clicked, this, &ConnectToServerDialog::onDelButtonClicked);
    connect(collectionServerView, &DListView::clicked, this, [this](const QModelIndex &index) {
        if (index.data().toString() != serverComboBox->currentText()) {
            serverComboBox->setCurrentText(index.data().toString());
        }
    });
}

void ConnectToServerDialog::onAddButtonClicked()
{
    QStringList serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();

    const QString &text = serverComboBox->currentText();
    if (!text.isEmpty() && !serverList.contains(text)) {
        if (!collectionServerView->addItem(text)) {
            qWarning() << "add server failed, server: " << text;
        } else {
            serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();
            const QModelIndex modelIndex = collectionServerView->model()->index(serverList.indexOf(text),
                                                                                0,
                                                                                collectionServerView->currentIndex().parent());
            collectionServerView->setCurrentIndex(modelIndex);

            QStringList dataList;
            const QList<QVariant> &serverData = Application::genericSetting()->value("ConnectServer", "URL").toList();
            for (const QVariant &data : serverData) {
                dataList << data.toString();
            }

            if (!dataList.contains(text)) {
                dataList << text;
                Application::genericSetting()->setValue("ConnectServer", "URL", dataList);
            }
        }
    }
}

void ConnectToServerDialog::onDelButtonClicked()
{
    const QStringList &serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();
    const QString &text = serverComboBox->currentText();
    if (!text.isEmpty() && serverList.contains(text)) {
        if (!collectionServerView->removeItem(serverList.indexOf(text))) {
            qWarning() << "remove server failed, server: " << text;
        } else {
            const QList<QVariant> &serverData = Application::genericSetting()->value("ConnectServer", "URL").toList();

            QStringList dataList;
            for (const QVariant &data : serverData) {
                dataList << data.toString();
            }

            if (dataList.contains(text)) {
                dataList.removeOne(text);
                Application::genericSetting()->setValue("ConnectServer", "URL", dataList);
            }

            const QModelIndex &currentIndex = collectionServerView->currentIndex();
            if (currentIndex.isValid()) {
                serverComboBox->setCurrentText(currentIndex.data().toString());
            } else {
                serverComboBox->clearEditText();
            }
        }
    }
}
