// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "durl.h"
#include "singleton.h"
#include "dfmeventdispatcher.h"
#include "connecttoserverdialog.h"
#include "collectiondelegate.h"
#include "dguiapplicationhelper.h"
#include "../views/dfilemanagerwindow.h"
#include "../views/dtoolbar.h"
#include "../shutil/fileutils.h"
#include "../controllers/searchhistroymanager.h"
#include "../interfaces/dfmsettings.h"
#include "../interfaces/dfmapplication.h"
#include "gvfs/networkmanager.h"

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
#include <DListView>
#include <QWindow>
#include <QRegExpValidator>
#include <QListWidget>

static constexpr char kConnectServer[] { "ConnectServer" };
static constexpr char kUrl[] { "URL" };
static int kMaxHistoryItems = 10;

ConnectToServerDialog::ConnectToServerDialog(QWidget *parent) : DDialog(parent)
{
    setWindowTitle(tr("Connect to Server"));
    initUI();
    initConnect();
}

ConnectToServerDialog::~ConnectToServerDialog()
{
    if(m_serverComboBox){
        delete m_serverComboBox;
        m_serverComboBox = nullptr;
    }
    if(m_schemeComboBox){
        delete m_schemeComboBox;
        m_schemeComboBox = nullptr;
    }
    if(m_addButton){
        delete m_addButton;
        m_addButton = nullptr;
    }
    if(m_collectionServerView){
        delete m_collectionServerView;
        m_collectionServerView = nullptr;
    }
}

void ConnectToServerDialog::onButtonClicked(const int &index)
{
    // connect to server
    if(index == ConnectButton) {
        QString text = m_schemeComboBox->currentText() + m_serverComboBox->currentText();//smb://x.x.x.x
        if (m_serverComboBox->currentText().isEmpty()) {
            close();
            return;
        }
        QWidget *fileWindow = qobject_cast<QWidget *>(parent());

        const QString &currentDir = QDir::currentPath();
        const DUrl &currentUrl = fileWindow ? qobject_cast<DFileManagerWindow*>(fileWindow->topLevelWidget())->currentUrl():
                                             DUrl(text) ;

        if (currentUrl.isLocalFile()) {
            QDir::setCurrent(currentUrl.toLocalFile());
        }

        const DUrl &inputUrl = DUrl::fromUserInput(text, false); ///###: here, judge whether the text is a local file path.

        QDir::setCurrent(currentDir);
        if(fileWindow){
            NetworkManager::NetworkNodes.remove(inputUrl);
            DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, inputUrl, fileWindow->window());
        }
    }
    close();
}

void ConnectToServerDialog::collectionOperate()
{
    if(m_isAddState)
        onAddButtonClicked();
    else
        onDelButtonClicked();
}

void ConnectToServerDialog::onAddButtonClicked()
{
    QStringList serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();
    const QString &text = m_schemeComboBox->currentText() + m_serverComboBox->currentText();
    if (!text.isEmpty() && !serverList.contains(text)) {
        if(!m_collectionServerView->addItem(text)) {
            qWarning() << "add server failed, server: " << text;
        } else {
            serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();
            const QModelIndex modelIndex = m_collectionServerView->model()->index(serverList.indexOf(text),
                                                                                  0,
                                                                                  m_collectionServerView->currentIndex().parent());
            m_collectionServerView->setCurrentIndex(modelIndex);
            QStringList serverData = DFMApplication::genericSetting()->value(kConnectServer, kUrl).toStringList();

            if (!serverData.contains(text)) {
                serverData << text;
                DFMApplication::genericSetting()->setValue(kConnectServer, kUrl, serverData);
            }
        }
    }
    upateState();
}

void ConnectToServerDialog::onDelButtonClicked()
{
    const QString &text = m_schemeComboBox->currentText() + m_serverComboBox->currentText();
    doDeleteCollection(text);
}

void ConnectToServerDialog::upateState()
{
    //检查是否已经收藏
    const QList<QVariant> &serverData = DFMApplication::genericSetting()->value(kConnectServer, kUrl).toList();
    QStringList dataList;
    for (const QVariant &data : serverData) {
        dataList << data.toString();
    }
    const QString &text = m_schemeComboBox->currentText() + m_serverComboBox->currentText();
    if (dataList.contains(text)) {
        //已经收藏，显示取消收藏
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CancelIcon,true)).scaled(16,16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CancelIcon,false)).scaled(16,16)));
        m_isAddState = false;
        m_addButton->setToolTip(tr("Unfavorite"));
    }else {//没有收藏，显示收藏
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CollectionIcon,true)).scaled(16,16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CollectionIcon,false)).scaled(16,16)));
        m_isAddState = true;
        m_addButton->setToolTip(tr("Favorite"));
    }
    bool hasCollections = m_collectionServerView->count() > 0;
    if(m_centerNotes)
        m_centerNotes->setVisible(!hasCollections);
    if(m_collectionServerView)
        m_collectionServerView->setVisible(hasCollections);
}

void ConnectToServerDialog::doDeleteCollection(const QString &text, int row)
{
    const QStringList &serverList = static_cast<QStringListModel *>(m_collectionServerView->model())->stringList();
    bool removeSuccess = true;
    if(serverList.isEmpty())
        removeSuccess = false;
    else if(row >=0 && row < m_collectionServerView->model()->rowCount()){
        QModelIndex curIndex = m_collectionServerView->model()->index(row,0);
        if(!m_collectionServerView->removeItem(curIndex.row())){
            removeSuccess = false;
        }
    }else if (!text.isEmpty()) {
        if (!m_collectionServerView->removeItem(serverList.indexOf(text))) {
            removeSuccess = false;
            qWarning() << "remove server failed, server: " << text;
        }
    }
    if(removeSuccess) {
        const QList<QVariant> &serverData = DFMApplication::genericSetting()->value(kConnectServer, kUrl).toList();

        QStringList dataList;
        for (const QVariant &data : serverData) {
            dataList << data.toString();
        }

        if (dataList.contains(text)) {
            dataList.removeOne(text);
            DFMApplication::genericSetting()->setValue(kConnectServer, kUrl, dataList);
        }
    }
    upateState();
}

void ConnectToServerDialog::onCompleterActivated(const QString &text)
{
    const QString &scheme = QUrl::fromUserInput(text).scheme();
    if (!scheme.isEmpty())
        m_schemeComboBox->setCurrentText(schemeWithSlash(scheme));
}

void ConnectToServerDialog::onCurrentInputChanged(const QString &text)
{
    if (text == m_serverComboBox->itemText(m_serverComboBox->count() - 1)) {
        QSignalBlocker blocker(m_serverComboBox);
        Q_UNUSED(blocker)
        m_serverComboBox->clear();
        m_serverComboBox->addItem(tr("Clear History"));
        m_serverComboBox->clearEditText();
        m_serverComboBox->completer()->setModel(new QStringListModel());
        Singleton<SearchHistroyManager>::instance()->clearHistory(m_supportedSchemes);
        DFMApplication::appObtuselySetting()->sync();
    }

    QUrl url = QUrl::fromUserInput(text);
    const QString &scheme = url.scheme();
    if (m_supportedSchemes.contains(schemeWithSlash(scheme))) {
        m_serverComboBox->setEditText(url.toString(QUrl::RemoveScheme).mid(2));
        m_schemeComboBox->setCurrentText(schemeWithSlash(scheme));
    }

    upateState();
}

void ConnectToServerDialog::onCollectionViewClicked(const QModelIndex &index)
{
    QString history = index.data().toString();
    if (history != m_schemeComboBox->currentText() + m_serverComboBox->currentText()) {
        DUrl histroyUrl(history);
        m_schemeComboBox->setCurrentText(schemeWithSlash(histroyUrl.scheme()));
        int checkedIndex = m_serverComboBox->findText(history);
        if (checkedIndex >= 0)
            m_serverComboBox->setCurrentIndex(checkedIndex);

        m_serverComboBox->setCurrentText(history);

        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CancelIcon,true)).scaled(16, 16)));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CancelIcon,false)).scaled(16, 16)));
        m_isAddState = false;
        m_addButton->setToolTip(tr("Unfavorite"));
    }
    m_collectionServerView->model()->index(index.row(), 0);
    m_collectionServerView->setCurrentIndex(index);
}

QString ConnectToServerDialog::schemeWithSlash(const QString &scheme) const
{
    return QString("%1://").arg(scheme);
}

QString ConnectToServerDialog::collectionIcon(CollectionIconType icon, bool light) const
{
    if (icon == CollectionIconType::CancelIcon)
        return QString(":icons/deepin/builtin/%1/icons/collect_cancel.svg").arg(light?"light":"dark");
    else
        return QString(":icons/deepin/builtin/%1/icons/collect.svg").arg(light?"light":"dark");
}

void ConnectToServerDialog::initUI()
{
    if (DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setFixedSize(476, 380);

    m_supportedSchemes << schemeWithSlash(SMB_SCHEME)
                       << schemeWithSlash(FTP_SCHEME)
                       << schemeWithSlash(SFTP_SCHEME);

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Connect","button"));
    addButton(buttonTexts[CannelButton], false);
    addButton(buttonTexts[ConnectButton], true, DDialog::ButtonRecommend);

    QFrame *contentFrame = new QFrame(this);
    m_serverComboBox = new QComboBox();
    m_schemeComboBox = new QComboBox();
    m_addButton = new QPushButton();
    m_addButton->setToolTip(tr("Favorite"));
    m_collectionLabel = new QLabel(tr("My Favorites"));
    m_collectionServerView = new DListView();
    m_delegate = new CollectionDelegate(m_collectionServerView);
    connect (m_delegate,&CollectionDelegate::removeItemManually,[this](const QString& text,int row){
        doDeleteCollection(text,row);
    });
    m_collectionServerView->setItemDelegate(m_delegate);
    m_addButton->setFixedSize(36, 36);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CollectionIcon,true)).scaled(16,16)));
    else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        m_addButton->setIcon(QIcon(QPixmap(collectionIcon(CollectionIcon,false)).scaled(16,16)));

    m_addButton->setIconSize({36, 36});

    QHBoxLayout *comboButtonLayout = new QHBoxLayout();
    comboButtonLayout->addWidget(m_schemeComboBox, 0, Qt::AlignVCenter);
    comboButtonLayout->addWidget(m_serverComboBox, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(6);
    comboButtonLayout->addWidget(m_addButton, 0, Qt::AlignVCenter);
    comboButtonLayout->addSpacing(2);
    comboButtonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->addLayout(comboButtonLayout);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_collectionLabel, 0, Qt::AlignVCenter);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_collectionServerView, 0, Qt::AlignVCenter);
    contentLayout->setContentsMargins(5, 0, 0, 0);

    contentFrame->setLayout(contentLayout);
    addContent(contentFrame);
    QStringList historyList = Singleton<SearchHistroyManager>::instance()->toStringList();
    for (const QString &data : historyList) {
        QUrl url(data);
        if (!url.isValid() || !m_supportedSchemes.contains(schemeWithSlash(url.scheme())))
            historyList.removeOne(data);
    }
    while (historyList.count() > kMaxHistoryItems - 1)   // Max_HISTORY_ITEM - 1 : leave a space for "Clear History"
        historyList.removeFirst();   // Keep the latest `maxHistoryItems`

    QString recentAccessOne = historyList.count() > 0 ? historyList.last() : QString();
    historyList.removeDuplicates();
    m_completer = new QCompleter(historyList,this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setMaxVisibleItems(kMaxHistoryItems);

    m_serverComboBox->setEditable(true);
    m_serverComboBox->setMaxVisibleItems(kMaxHistoryItems);

    m_serverComboBox->addItems(historyList);
    m_serverComboBox->insertItem(m_serverComboBox->count(), tr("Clear History"));
    m_serverComboBox->setCompleter(m_completer);
    m_serverComboBox->clearEditText();

    m_schemeComboBox->addItems(m_supportedSchemes);
    m_schemeComboBox->setFixedWidth(100);

    if (!recentAccessOne.isEmpty()) {   // Recent accessed url must be selected in initial state.
        QUrl recentAccessUrl(recentAccessOne);
        const QString &scheme = recentAccessUrl.scheme();
        if (!scheme.isEmpty()) {
            int checkedIndex = m_serverComboBox->findText(recentAccessOne);
            if (checkedIndex >= 0)
                m_serverComboBox->setCurrentIndex(checkedIndex);

            m_serverComboBox->setEditText(recentAccessUrl.toString(QUrl::RemoveScheme).mid(2));
            m_schemeComboBox->setCurrentText(schemeWithSlash(scheme));
        }
    }

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

    const QStringList &serverData = DFMApplication::genericSetting()->value(kConnectServer, kUrl).toStringList();
    QStringListModel *listModel = new QStringListModel(this);
    m_collectionServerView->setModel(listModel);
    listModel->setStringList(serverData);

    setContentsMargins(0,0,0,0);
    upateState();

    const bool hasCollections = m_collectionServerView->count() > 0;
    QHBoxLayout *centerNotesLayout = new QHBoxLayout();
    m_centerNotes = new QLabel(tr("No favorites yet"),this);
    centerNotesLayout->addWidget(m_centerNotes,0,Qt::AlignHCenter);
    m_centerNotes->setVisible(false);
    contentLayout->addLayout(centerNotesLayout,Qt::AlignVCenter);
    m_centerNotes->setVisible(!hasCollections);
    m_collectionServerView->setVisible(hasCollections);
}

void ConnectToServerDialog::initConnect()
{
    connect(m_serverComboBox, &QComboBox::currentTextChanged, this, &ConnectToServerDialog::onCurrentInputChanged);

    typedef void (QCompleter::*QCompleTextSignal)(const QString&);
    connect(m_completer, static_cast<QCompleTextSignal>(&QCompleter::activated), this, &ConnectToServerDialog::onCompleterActivated);

    connect(m_schemeComboBox, &QComboBox::currentTextChanged,this,[=](const QString &string){
        Q_UNUSED(string)
        upateState();
    });

    connect(m_addButton, &QPushButton::clicked, this, &ConnectToServerDialog::collectionOperate);

    connect(m_collectionServerView, &DListView::clicked, this, &ConnectToServerDialog::onCollectionViewClicked);
}
