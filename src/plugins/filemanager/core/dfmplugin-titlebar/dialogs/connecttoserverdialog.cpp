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
#include "collectiondelegate.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/windowutils.h"

#include <DIconButton>
#include <DListView>
#include <DGuiApplicationHelper>

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
#include <QSpacerItem>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

DFMGLOBAL_USE_NAMESPACE

static constexpr char kConnectServer[] { "ConnectServer" };
static constexpr char kUrl[] { "URL" };
static constexpr char kprotocolIPRegExp[] { R"(((smb)|(ftp)|(sftp))(://)((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3})" };

ConnectToServerDialog::ConnectToServerDialog(const QUrl &url, QWidget *parent)
    : DDialog(parent), currentUrl(url)
{
    setWindowTitle(tr("Connect to Server"));
    initializeUi();
    initConnect();

    protocolIPRegExp.setPattern(kprotocolIPRegExp);
    protocolIPRegExp.setCaseSensitivity(Qt::CaseInsensitive);
}

void ConnectToServerDialog::collectionOperate()
{
    if (isAddState)
        onAddButtonClicked();
    else
        onDelButtonClicked();
}

void ConnectToServerDialog::onButtonClicked(const int &index)
{
    // connect to server
    if (index == kConnectButton) {
        QString text { QString("%1%2").arg(schemeComboBox->currentText()).arg(serverComboBox->currentText()) };
        if (serverComboBox->currentText().isEmpty()) {
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
        if (!SearchHistroyManager::instance()->getSearchHistroy().contains(text)) {
            SearchHistroyManager::instance()->writeIntoSearchHistory(text);
        } else {   // to make sure it is the recent one
            SearchHistroyManager::instance()->removeSearchHistory(text);
            SearchHistroyManager::instance()->writeIntoSearchHistory(text);
        }

        if (protocolIPRegExp.exactMatch(text))
            SearchHistroyManager::instance()->writeIntoIPHistory(text);
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

        SearchHistroyManager::instance()->clearHistory(supportedSchemes);
    }
}

void ConnectToServerDialog::doDeleteCollection(const QString &text, int row)
{
    const QStringList &serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();
    bool removeSuccess = true;
    if (serverList.isEmpty())
        removeSuccess = false;
    else if (row >= 0 && row < collectionServerView->model()->rowCount()) {
        QModelIndex curIndex = collectionServerView->model()->index(row, 0);
        if (!collectionServerView->removeItem(curIndex.row())) {
            removeSuccess = false;
        }
    } else if (!text.isEmpty()) {
        if (!collectionServerView->removeItem(serverList.indexOf(text))) {
            removeSuccess = false;
            qWarning() << "remove server failed, server: " << text;
        }
    }
    if (removeSuccess) {
        QStringList serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
        if (serverData.contains(text)) {
            serverData.removeOne(text);
            Application::genericSetting()->setValue(kConnectServer, kUrl, serverData);
        }
    }
    upateUiState();
}

void ConnectToServerDialog::onCurrentInputChanged(const QString &text)
{
    if (text == serverComboBox->itemText(serverComboBox->count() - 1)) {
        QSignalBlocker blocker(serverComboBox);
        Q_UNUSED(blocker)
        serverComboBox->clear();
        serverComboBox->addItem(tr("Clear History"));
        serverComboBox->clearEditText();
        serverComboBox->completer()->setModel(new QStringListModel());
        SearchHistroyManager::instance()->clearHistory(supportedSchemes);
        Application::appObtuselySetting()->sync();
    }

    QUrl url(text);
    const QString &scheme = url.scheme();
    if (supportedSchemes.contains(schemeWithSlash(scheme))) {
        QString temText = text;
        serverComboBox->setEditText(temText.remove(schemeWithSlash(scheme)));
        schemeComboBox->setCurrentText(schemeWithSlash(scheme));
    }

    upateUiState();
}

void ConnectToServerDialog::onCollectionViewClicked(const QModelIndex &index)
{
    const QString &history = index.data().toString();
    if (history != schemeComboBox->currentText() + serverComboBox->currentText()) {
        QUrl histroyUrl(history);
        schemeComboBox->setCurrentText(schemeWithSlash(histroyUrl.scheme()));
        int checkedIndex = serverComboBox->findText(history);
        if (checkedIndex >= 0)
            serverComboBox->setCurrentIndex(checkedIndex);
        serverComboBox->setCurrentText(history);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/light/icons/collect_cancel.svg").scaled(16, 16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/dark/icons/collect_cancel.svg").scaled(16, 16)));
        isAddState = false;
        theAddButton->setToolTip(tr("Unfavorite"));
    }
    collectionServerView->model()->index(index.row(), 0);
    collectionServerView->setCurrentIndex(index);
}

void ConnectToServerDialog::onCompleterActivated(const QString &text)
{
    const QString &scheme = QUrl::fromUserInput(text).scheme();
    if (!scheme.isEmpty())
        schemeComboBox->setCurrentText(schemeWithSlash(scheme));
}

void ConnectToServerDialog::initializeUi()
{
    if (WindowUtils::isWayLand()) {
        // Hide the maximize and minimize buttons
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
    schemeComboBox = new QComboBox();
    supportedSchemes << QString("%1://").arg(Scheme::kSmb)
                     << QString("%1://").arg(Scheme::kFtp)
                     << QString("%1://").arg(Scheme::kSFtp);
    schemeComboBox->addItems(supportedSchemes);
    theAddButton = new DIconButton(nullptr);
    QLabel *collectionLabel = new QLabel(tr("My Favorites"));
    collectionServerView = new DListView(contentFrame);
    delegate = new CollectionDelegate(collectionServerView);
    connect(delegate, &CollectionDelegate::removeItemManually, [this](const QString &text, int row) {
        doDeleteCollection(text, row);
    });
    collectionServerView->setItemDelegate(delegate);

    theAddButton->setFixedSize(44, 44);
    collectionLabel->setFixedSize(98, 20);

    theAddButton->setIcon(QIcon::fromTheme("dfm_add_server"));
    theAddButton->setIconSize({ 44, 44 });
    theAddButton->setFlat(true);

    QHBoxLayout *comboButtonLayout = new QHBoxLayout();
    comboButtonLayout->addWidget(schemeComboBox, 0, Qt::AlignVCenter);
    comboButtonLayout->addWidget(serverComboBox, 1, Qt::AlignVCenter);

    comboButtonLayout->addSpacing(6);
    comboButtonLayout->addWidget(theAddButton, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(2);
    comboButtonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->addLayout(comboButtonLayout);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(collectionLabel, 0, Qt::AlignVCenter);
    contentLayout->addStretch(10);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(collectionServerView, 0, Qt::AlignVCenter);
    contentLayout->setContentsMargins(5, 0, 0, 0);

    contentFrame->setLayout(contentLayout);
    addContent(contentFrame);

    QStringList historyList = SearchHistroyManager::instance()->getSearchHistroy();
    for (const QString &data : historyList) {
        QUrl url(data);
        if (!url.isValid() || !supportedSchemes.contains(schemeWithSlash(url.scheme())))
            historyList.removeOne(data);
    }

    static int maxHistoryItems = 10;
    while (historyList.count() > maxHistoryItems - 1)   // maxHistoryItems - 1 : leave a space for "Clear History"
        historyList.takeFirst();   // Keep the latest `maxHistoryItems`

    QString recentAccessOne = historyList.count() > 0 ? historyList.last() : QString();
    historyList.removeDuplicates();

    serverComboBox->setEditable(true);
    serverComboBox->addItems(historyList);
    serverComboBox->setEditable(true);
    serverComboBox->setMaxVisibleItems(maxHistoryItems);
    serverComboBox->clearEditText();

    if (!recentAccessOne.isEmpty()) {   // Recent accessed url must be selected in initial state.
        QUrl recentAccessUrl(recentAccessOne);
        const QString &scheme = recentAccessUrl.scheme();
        if (!scheme.isEmpty()) {
            int checkedIndex = serverComboBox->findText(recentAccessOne);
            if (checkedIndex >= 0)
                serverComboBox->setCurrentIndex(checkedIndex);

            serverComboBox->setEditText(recentAccessOne.remove(schemeWithSlash(scheme)));
            schemeComboBox->setCurrentText(schemeWithSlash(scheme));
        }
    }

    completer = new QCompleter(historyList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setMaxVisibleItems(maxHistoryItems);

    serverComboBox->setCompleter(completer);
    serverComboBox->insertItem(serverComboBox->count(), tr("Clear History"));
    if (serverComboBox->count() == 1)
        serverComboBox->clearEditText();   // To avoid show "Clear History"

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

    QStringListModel *listModel = new QStringListModel(this);
    collectionServerView->setModel(listModel);

    const QStringList &serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
    listModel->setStringList(serverData);

    setContentsMargins(0, 0, 0, 0);
    upateUiState();

    const bool hasCollections = collectionServerView->count() > 0;
    QHBoxLayout *centerNotesLayout = new QHBoxLayout();
    centerNotes = new QLabel(tr("No favorites yet"), this);
    centerNotesLayout->addWidget(centerNotes, 0, Qt::AlignHCenter);
    centerNotes->setVisible(false);
    contentLayout->addLayout(centerNotesLayout, Qt::AlignVCenter);
    centerNotes->setVisible(!hasCollections);
    collectionServerView->setVisible(hasCollections);
}

void ConnectToServerDialog::initConnect()
{
    connect(serverComboBox, &QComboBox::currentTextChanged, this, &ConnectToServerDialog::onCurrentInputChanged);
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(onCompleterActivated(const QString &)));
    connect(schemeComboBox, &QComboBox::currentTextChanged, this, [=](const QString &string) {
        Q_UNUSED(string)
        upateUiState();
    });

    connect(theAddButton, &DTK_WIDGET_NAMESPACE::DIconButton::clicked, this, &ConnectToServerDialog::collectionOperate);
    connect(collectionServerView, &DListView::clicked, this, &ConnectToServerDialog::onCollectionViewClicked);
}

void ConnectToServerDialog::onAddButtonClicked()
{
    QStringList serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();
    if (serverComboBox->currentText().trimmed().isEmpty())
        return;
    const QString &text = schemeComboBox->currentText() + serverComboBox->currentText();
    if (!text.isEmpty() && !serverList.contains(text)) {
        if (!collectionServerView->addItem(text)) {
            qWarning() << "add server failed, server: " << text;
        } else {
            serverList = static_cast<QStringListModel *>(collectionServerView->model())->stringList();
            const QModelIndex modelIndex = collectionServerView->model()->index(serverList.indexOf(text),
                                                                                0,
                                                                                collectionServerView->currentIndex().parent());
            collectionServerView->setCurrentIndex(modelIndex);
            QStringList serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
            if (!serverData.contains(text)) {
                serverData << text;
                Application::genericSetting()->setValue(kConnectServer, kUrl, serverData);
            }
        }
    }

    upateUiState();
}

void ConnectToServerDialog::onDelButtonClicked()
{
    const QString &text = schemeComboBox->currentText() + serverComboBox->currentText();
    doDeleteCollection(text);
}

void ConnectToServerDialog::upateUiState()
{
    const QStringList &serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
    const QString &text = schemeComboBox->currentText() + serverComboBox->currentText();
    if (serverData.contains(text)) {
        //Current text is already collected, collection button display the cancel icon.
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/light/icons/collect_cancel.svg").scaled(16, 16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/dark/icons/collect_cancel.svg").scaled(16, 16)));
        isAddState = false;
        theAddButton->setToolTip(tr("Unfavorite"));
    } else {   //Current text is not collected, collection button display the collection icon.
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/light/icons/collect.svg").scaled(16, 16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            theAddButton->setIcon(QIcon(QPixmap(":icons/deepin/builtin/dark/icons/collect.svg").scaled(16, 16)));
        isAddState = true;
        theAddButton->setToolTip(tr("Favorite"));
    }
    // Display the notice info in the center when there is no any collections.
    bool hasCollections = collectionServerView->count() > 0;
    if (centerNotes)
        centerNotes->setVisible(!hasCollections);
    if (collectionServerView)
        collectionServerView->setVisible(hasCollections);
}

QString ConnectToServerDialog::schemeWithSlash(const QString &scheme) const
{
    return scheme + "://";
}
