// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connecttoserverdialog.h"
#include "collectiondelegate.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/event/event.h>

#include <DIconButton>
#include <DListView>
#include <DGuiApplicationHelper>
#include <DComboBox>
#include <DLineEdit>
#include <DLabel>
#include <DFrame>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QStringList>
#include <QStringListModel>
#include <QScrollBar>
#include <QDir>
#include <QCompleter>
#include <QWindow>
#include <QSpacerItem>
#include <QIcon>
#include <QCoreApplication>
#include <QMouseEvent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr char kConnectServer[] { "ConnectServer" };
static constexpr char kUrl[] { "URL" };
static const int kMaxHistoryItems = 10;
static constexpr char kGBKCharset[] { "gbk" };
static constexpr char kUTF8CharSet[] { "utf8" };
static constexpr char kUTF8CharSet2[] { "utf-8" };

enum CharsetOption {
    kDefault,
    kUtf8,
    kGbk,
};

ConnectToServerDialog::ConnectToServerDialog(const QUrl &url, QWidget *parent)
    : DDialog(parent), currentUrl(url)
{
    setWindowTitle(tr("Connect to Server"));
    initializeUi();
    initConnect();
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
        if (serverComboBox->currentText().isEmpty()) {
            fmWarning() << "Connect attempt with empty server address";
            close();
            return;
        }

        QString url = getCurrentUrlString();
        fmInfo() << "Attempting to connect to server:" << url;
        updateCollections(url, false);

        const QString &currentDir = QDir::currentPath();
        if (currentUrl.isLocalFile())
            QDir::setCurrent(currentUrl.toLocalFile());
        QDir::setCurrent(currentDir);

        // add search history list
        SearchHistroyManager::instance()->addIPHistoryCache(url);

        QWidget *fileWindow = qobject_cast<QWidget *>(parent());
        TitleBarHelper::handleJumpToPressed(fileWindow, url);
    }
    close();
}

void ConnectToServerDialog::doDeleteCollection(const QString &text, int row)
{
    QString deletedItem = text;
    if (row >= 0)
        deletedItem = model->data(model->index(row), CollectionModel::kUrlRole).toString();

    QStringList collections = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
    collections.removeAll(deletedItem);
    Application::genericSetting()->setValue(kConnectServer, kUrl, collections);

    model->setStringList(collections);
    updateUiState();
}

void ConnectToServerDialog::onCurrentInputChanged(const QString &server)
{
    int index = serverComboBox->findText(server);
    if (index < 0)
        return updateUiState();

    QVariant customData = serverComboBox->itemData(index);
    // clear history
    if (customData.isValid() && customData.toBool()) {
        QSignalBlocker blocker(serverComboBox);
        Q_UNUSED(blocker)
        serverComboBox->clear();
        // After clearing history, show a disabled "No history" placeholder
        serverComboBox->addItem(tr("No history"));
        serverComboBox->model()->setData(serverComboBox->model()->index(serverComboBox->count() - 1, 0), 0, Qt::UserRole - 1);
        serverComboBox->setCurrentIndex(-1);
        serverComboBox->clearEditText();
        serverComboBox->completer()->setModel(new QStringListModel());
        SearchHistroyManager::instance()->clearHistory(supportedSchemes);
        SearchHistroyManager::instance()->clearIPHistory();
        Application::appObtuselySetting()->sync();
    } else {
        if (server.startsWith("ftp") && customData.isValid()) {
            int charsetOpt = customData.toInt();
            charsetComboBox->setCurrentIndex(charsetOpt);
        }

        if (server.contains("://")) {
            QString scheme = server.section("://", 0, 0);
            if (!scheme.isEmpty()) {
                serverComboBox->setEditText(server.section("//", -1));
                schemeComboBox->setCurrentText(scheme + "://");
            }
        }
    }

    updateUiState();
}

void ConnectToServerDialog::onCollectionViewClicked(const QModelIndex &index)
{
    QString urlStr = index.data(CollectionModel::kUrlRole).toString();
    QUrl url(urlStr);
    QString host = url.host();
    QString path = url.path();
    const QString &scheme = url.scheme();
    const QString &params = url.query().toLower();
    int port = url.port();
    QRegularExpression charsetRegx(R"(charset=([^&]*))");
    QString charset;
    auto match = charsetRegx.match(params);
    if (match.hasMatch())
        charset = match.captured(1);

    if (port != -1)
        host += QString(":%1").arg(port);
    serverComboBox->setCurrentText(host + path);
    schemeComboBox->setCurrentText(scheme + "://");

    CharsetOption opt = kDefault;
    if (charset == kGBKCharset)
        opt = kGbk;
    else if (charset == kUTF8CharSet || charset == kUTF8CharSet2)
        opt = kUtf8;
    else
        opt = kDefault;

    charsetComboBox->setCurrentIndex(opt);

    collectionServerView->model()->index(index.row(), 0);
    collectionServerView->setCurrentIndex(index);
}

void ConnectToServerDialog::onCompleterActivated(const QString &text)
{
    const QString &scheme = QUrl::fromUserInput(text).scheme();
    if (!scheme.isEmpty())
        schemeComboBox->setCurrentText(schemeWithSlash(scheme));
}

void ConnectToServerDialog::initIfWayland()
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
}

void ConnectToServerDialog::updateAddButtonState(bool collected)
{
    const QString &iconName = collected ? "dfm_server_uncollect" : "dfm_server_collect";
    theAddButton->setIcon(QIcon::fromTheme(iconName));
    theAddButton->setIconSize({ 16, 16 });
    theAddButton->setToolTip(collected ? tr("Unfavorite") : tr("Favorite"));
    isAddState = !collected;

    theAddButton->setDisabled(serverComboBox->lineEdit()->text().isEmpty());
}

void ConnectToServerDialog::initServerDatas()
{
    QStringList hosts;
    const QStringList expectedSchemes { "smb", "ftp", "sftp" };

    auto processUrl = [this, &hosts, &expectedSchemes](const QString &urlStr, CharsetOption opt = kDefault) {
        QString processedUrl = urlStr.toLower().replace("/?", "?");
        while (processedUrl.endsWith("/")) {
            processedUrl.chop(1);
        }

        QUrl url(processedUrl);
        if (!expectedSchemes.contains(url.scheme()) || url.host().isEmpty()) {
            fmDebug() << "Skipping invalid URL in history:" << urlStr;
            return;
        }

        processedUrl = processedUrl.section('?', 0, 0);
        if (!hosts.contains(processedUrl)) {
            hosts.prepend(processedUrl);
            serverComboBox->insertItem(0, processedUrl, opt);
        }
    };

    const QList<IPHistroyData> ipHistoryData = SearchHistroyManager::instance()->getIPHistory();
    for (const auto &ipData : ipHistoryData) {
        if (ipData.isRecentlyAccessed()) {
            const QString ipStr = QString("%1://%2").arg(ipData.accessedType, ipData.ipData);
            processUrl(ipStr);
        }
    }

    const QStringList searchList = SearchHistroyManager::instance()->getSearchHistroy();
    for (int i = searchList.count() - 1; i >= 0 && hosts.count() < kMaxHistoryItems - 1; --i) {
        QString urlStr = searchList.at(i);
        CharsetOption opt = kDefault;
        const QString &query = QUrl(urlStr).query();
        if (!query.isEmpty()) {
            const static QRegularExpression charsetRegx(R"(charset=([^&]*))");
            const auto match = charsetRegx.match(query);
            if (match.hasMatch()) {
                const QString charset = match.captured(1);
                if (charset == kGBKCharset)
                    opt = kGbk;
                else if (charset == kUTF8CharSet || charset == kUTF8CharSet2)
                    opt = kUtf8;
            }
        }
        processUrl(urlStr, opt);
    }

    completer->setModel(new QStringListModel(hosts, completer));
    if (!hosts.isEmpty()) {
        // Add a clear-history action as the last item (customData true)
        serverComboBox->addItem(tr("Clear History"), true);
        onCurrentInputChanged(hosts.last());
    } else {
        // No history: show a disabled placeholder so dropdown shows a hint
        serverComboBox->addItem(tr("No history"));
        serverComboBox->model()->setData(serverComboBox->model()->index(serverComboBox->count() - 1, 0), 0, Qt::UserRole - 1);
        serverComboBox->setCurrentIndex(-1);
        serverComboBox->clearEditText();
    }
}

QStringList ConnectToServerDialog::updateCollections(const QString &newUrlStr, bool insertWhenNoExist)
{
    QStringList collections = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
    if (collections.contains(newUrlStr))
        return collections;

    // if only charset changed, remove old item
    bool needUpdate = false;
    QUrl newUrl = QUrl::fromUserInput(newUrlStr);

    if (newUrl.host().isEmpty()) {
        fmWarning() << "Invalid URL provided for collection:" << newUrlStr;
        DialogManagerInstance->showErrorDialog(tr("Error"),
                                               tr("Unable to favorite illegitimate url!"));
        return collections;
    }

    for (const auto &collection : collections) {
        QUrl old(collection);

        QString oldSharePath = old.path();
        QString newSharePath = newUrl.path();

        if (old.scheme() == newUrl.scheme() && old.host() == newUrl.host()
            && newSharePath == oldSharePath
            && collection != newUrlStr) {
            collections.removeAll(collection);
            needUpdate = true;
            break;
        }
    }

    if (needUpdate || insertWhenNoExist)
        collections.append(newUrlStr);

    Application::genericSetting()->setValue(kConnectServer, kUrl, collections);
    return collections;
}

QString ConnectToServerDialog::getCurrentUrlString()
{
    QString url = schemeComboBox->currentText() + serverComboBox->currentText();
    if (!url.startsWith("ftp") || charsetComboBox->currentIndex() == kDefault)
        return url;

    if (url.contains(QRegularExpression(R"([?&]charset=)"))) {
        fmDebug() << "User provided charset parameter in URL:" << url;
        return url;
    }

    url += (url.contains("?")) ? "&charset=" : "?charset=";
    switch (charsetComboBox->currentIndex()) {
    case kUtf8:
        url += kUTF8CharSet;
        break;
    case kGbk:
        url += kGBKCharset;
        break;
    default:
        url.remove("?charset=");
        break;
    }
    return url;
}

void ConnectToServerDialog::initializeUi()
{
    // init DDialog buttons
    {
        QStringList buttonTexts;
        buttonTexts.append(tr("Cancel", "button"));
        buttonTexts.append(tr("Connect", "button"));
        addButton(buttonTexts[kCannelButton], false);
        addButton(buttonTexts[kConnectButton], true, DDialog::ButtonRecommend);
    }

    QGridLayout *addressLayout = new QGridLayout();
    // init address area
    {
        schemeComboBox = new DComboBox(this);
        supportedSchemes << QString("%1://").arg(Scheme::kSmb)
                         << QString("%1://").arg(Scheme::kFtp)
                         << QString("%1://").arg(Scheme::kSFtp)
                         << QString("%1://").arg(Scheme::kDav)
                         << QString("%1://").arg(Scheme::kDavs)
                         << QString("%1://").arg(Scheme::kNfs);
        schemeComboBox->addItems(supportedSchemes);
        schemeComboBox->setFixedWidth(100);

        completer = new QCompleter(this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setMaxVisibleItems(kMaxHistoryItems);

        serverComboBox = new DComboBox(this);
        serverComboBox->setEditable(true);
        serverComboBox->setMaxVisibleItems(kMaxHistoryItems);
        serverComboBox->clearEditText();
        serverComboBox->setCompleter(completer);

        theAddButton = new DIconButton(this);
        theAddButton->setMaximumSize(38, 38);
        theAddButton->setFlat(false);
        theAddButton->setIconSize({ 16, 16 });

        addressLayout->addWidget(schemeComboBox, 0, 0);
        addressLayout->addWidget(serverComboBox, 0, 1);
        addressLayout->addWidget(theAddButton, 0, 2);
    }

    // init charset area
    {
        charsetLabel = new DLabel(tr("Charset Encoding"));
        charsetLabel->setVisible(false);
        charsetLabel->setContentsMargins(8, 0, 0, 0);

        charsetComboBox = new DComboBox();
        charsetComboBox->addItems({ tr("Default"), "UTF-8", "GBK" });
        charsetComboBox->setVisible(false);

        addressLayout->addWidget(charsetLabel, 1, 0, Qt::AlignLeft);
        addressLayout->addWidget(charsetComboBox, 1, 1);
    }

    // init collection area
    {
        collectionServerView = new DListView();
        collectionServerView->setVerticalScrollMode(DListView::ScrollPerPixel);
        collectionServerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        collectionServerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        collectionServerView->setResizeMode(DListView::Fixed);
        collectionServerView->setUniformItemSizes(true);
        collectionServerView->setItemSize({ collectionServerView->width(), 36 });
        collectionServerView->setItemMargins({ 0, 0, 0, 0 });
        collectionServerView->setItemSpacing(1);
        collectionServerView->setSelectionBehavior(QAbstractItemView::SelectRows);
        collectionServerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        collectionServerView->setFrameShape(QFrame::Shape::NoFrame);

        delegate = new CollectionDelegate(collectionServerView);
        collectionServerView->setItemDelegate(delegate);

        model = new CollectionModel(this);
        const QStringList &serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
        model->setStringList(serverData);
        collectionServerView->setModel(model);
    }

    // init collection empty area
    {
        QVBoxLayout *emptyLayout = new QVBoxLayout();
        emptyFrame = new DFrame();
        emptyFrame->setLineWidth(0);
        emptyFrame->setBackgroundRole(DPalette::ItemBackground);
        emptyFrame->setMaximumHeight(295);
        emptyFrame->setLayout(emptyLayout);

        noFavoritesLabel = new DLabel();
        noFavoritesLabel->setContentsMargins(120, 40, 0, 0);
        noFavoritesLabel->setMaximumHeight(200);

        DLabel *centerNotes = new DLabel();
        centerNotes->setMaximumHeight(30);
        centerNotes->setText(tr("No favorites yet"));
        centerNotes->setStyleSheet("color:gray;");

        emptyLayout->addWidget(noFavoritesLabel, Qt::AlignHCenter);
        emptyLayout->addSpacing(5);
        emptyLayout->addWidget(centerNotes, 0, Qt::AlignHCenter | Qt::AlignTop);
        emptyLayout->setSpacing(0);
    }

    // do layout
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addSpacing(8);
        DLabel *collectionLabel = new DLabel(tr("My Favorites"));
        layout->addWidget(collectionLabel);

        QVBoxLayout *contentLayout = new QVBoxLayout();
        contentLayout->addLayout(addressLayout);
        contentLayout->addSpacing(5);
        contentLayout->addLayout(layout);
        contentLayout->addSpacing(5);
        contentLayout->addWidget(collectionServerView);
        contentLayout->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout *centerNotesLayout = new QHBoxLayout();
        centerNotesLayout->addWidget(emptyFrame);
        contentLayout->addLayout(centerNotesLayout);

        QFrame *contentFrame = new QFrame(this);
        contentFrame->setLayout(contentLayout);
        addContent(contentFrame);
    }

#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(theAddButton), AcName::kAcComputerTitleBarAddBtn);

    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(collectionServerView), AcName::kAcComputerTitleBarCollectionServerView);

#endif

    initServerDatas();
    initIfWayland();
    setContentsMargins(0, 0, 0, 0);
    updateUiState();
    initUiForSizeMode();
    updateTheme();
}

void ConnectToServerDialog::initConnect()
{
    connect(serverComboBox, &DComboBox::currentTextChanged, this, &ConnectToServerDialog::onCurrentInputChanged);
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(onCompleterActivated(const QString &)));
    connect(delegate, &CollectionDelegate::removeItemManually, this, &ConnectToServerDialog::doDeleteCollection);
    connect(theAddButton, &DTK_WIDGET_NAMESPACE::DIconButton::clicked, this, &ConnectToServerDialog::collectionOperate);
    connect(collectionServerView, &DListView::clicked, this, &ConnectToServerDialog::onCollectionViewClicked);
    connect(schemeComboBox, &DComboBox::currentTextChanged, this, [=](const QString &) { updateUiState(); });
    connect(charsetComboBox, &DComboBox::currentTextChanged, this, [=] { updateUiState(); });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ConnectToServerDialog::updateTheme);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &ConnectToServerDialog::initUiForSizeMode);
#endif
}

void ConnectToServerDialog::onAddButtonClicked()
{
    const QString &currScheme = schemeComboBox->currentText();
    const QString &currHost = serverComboBox->currentText();
    if (currScheme.isEmpty() || currHost.isEmpty()) {
        fmWarning() << "Cannot add to favorites: scheme or host is empty";
        return;
    }

    const QString &fullUrlStr = getCurrentUrlString();
    fmInfo() << "Adding server to favorites:" << fullUrlStr;

    const QStringList &collections = updateCollections(fullUrlStr, true);
    model->setStringList(collections);

    updateUiState();
}

void ConnectToServerDialog::onDelButtonClicked()
{
    const QString &text = getCurrentUrlString();
    doDeleteCollection(text);
}

void ConnectToServerDialog::updateUiState()
{
    const QStringList &serverData = Application::genericSetting()->value(kConnectServer, kUrl).toStringList();
    const QString &url = getCurrentUrlString();
    updateAddButtonState(serverData.contains(url));

    // Display the notice info in the center when there is no any collections.
    bool hasCollections = collectionServerView->count() > 0;
    emptyFrame->setVisible(!hasCollections);
    collectionServerView->setVisible(hasCollections);

    // update charset visiable
    charsetLabel->setHidden(!url.startsWith("ftp"));
    charsetComboBox->setHidden(!url.startsWith("ftp"));

    // highlight current selection
    const QString &currUrlStr = getCurrentUrlString();
    int row = model->findItem(currUrlStr);
    collectionServerView->setCurrentIndex(model->index(row));
    getButton(kConnectButton)->setEnabled(!serverComboBox->currentText().isEmpty());

    // 更新鼠标hover状态 - 当鼠标移动到收藏列表视图上时,需要更新hover效果
    // 通过发送一个MouseMove事件来触发hover状态的更新
    if (hasCollections && collectionServerView->isVisible()) {
        QPoint globalPos = QCursor::pos();
        QPoint viewPos = collectionServerView->viewport()->mapFromGlobal(globalPos);

        if (collectionServerView->viewport()->rect().contains(viewPos)) {
            QMouseEvent *mouseEvent = new QMouseEvent(QEvent::MouseMove,
                                                      viewPos,
                                                      globalPos,
                                                      Qt::NoButton,
                                                      Qt::NoButton,
                                                      Qt::NoModifier);
            QCoreApplication::postEvent(collectionServerView->viewport(), mouseEvent);
        }
    }
}

QString ConnectToServerDialog::schemeWithSlash(const QString &scheme) const
{
    return scheme + "://";
}

void ConnectToServerDialog::updateTheme()
{
    const auto &noFavoritesIcon = DDciIcon::fromTheme("no_favorites_yet");
    const auto themeType = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
            ? DDciIcon::Light
            : DDciIcon::Dark;
    noFavoritesLabel->setPixmap(noFavoritesIcon.pixmap(qApp->devicePixelRatio(), 145, themeType));
}

void ConnectToServerDialog::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedSize(430, DSizeModeHelper::element(440, 490));
    int size = serverComboBox->height();
    theAddButton->setFixedSize(size, size);
#else
    setFixedSize(430, 490);
#endif
}
