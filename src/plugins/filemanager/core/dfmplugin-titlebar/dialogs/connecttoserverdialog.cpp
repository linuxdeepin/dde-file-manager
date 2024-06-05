// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

DFMGLOBAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr char kConnectServer[] { "ConnectServer" };
static constexpr char kUrl[] { "URL" };
static constexpr char kprotocolIPRegExp[] { R"(((smb)|(ftp)|(sftp))(://)((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3})" };
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

    protocolIPRegExp.setPattern(kprotocolIPRegExp);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    protocolIPRegExp.setCaseSensitivity(Qt::CaseInsensitive);
#else
    protocolIPRegExp.setPatternOptions(protocolIPRegExp.patternOptions() | QRegularExpression::CaseInsensitiveOption);
#endif
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
            close();
            return;
        }

        QString url = getCurrentUrlString();
        fmInfo() << "try connecting" << url;
        updateCollections(url, false);

        const QString &currentDir = QDir::currentPath();
        if (dfmbase::FileUtils::isLocalFile(currentUrl))
            QDir::setCurrent(currentUrl.toLocalFile());
        QDir::setCurrent(currentDir);

        QWidget *fileWindow = qobject_cast<QWidget *>(parent());
        TitleBarHelper::handlePressed(fileWindow, url);

        // add search history list
        SearchHistroyManager::instance()->writeIntoSearchHistory(url);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (protocolIPRegExp.exactMatch(url))
#else
        if (protocolIPRegExp.match(url).hasMatch())
#endif
            SearchHistroyManager::instance()->writeIntoIPHistory(url);
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
    int found = serverComboBox->findText(server);
    if (found >= 0 && server.startsWith("ftp")) {
        QVariant customData = serverComboBox->itemData(found);
        if (customData.isValid()) {
            int charsetOpt = customData.toInt();
            charsetComboBox->setCurrentIndex(charsetOpt);
        }
    }

    if (server == serverComboBox->itemText(serverComboBox->count() - 1)) {
        QSignalBlocker blocker(serverComboBox);
        Q_UNUSED(blocker)
        serverComboBox->clear();
        serverComboBox->addItem(tr("Clear History"));
        serverComboBox->clearEditText();
        serverComboBox->completer()->setModel(new QStringListModel());
        SearchHistroyManager::instance()->clearHistory(supportedSchemes);
        Application::appObtuselySetting()->sync();
    }

    if (server.contains("://")) {
        QString scheme = server.section("://", 0, 0);
        if (!scheme.isEmpty()) {
            serverComboBox->setEditText(server.section("//", -1));
            schemeComboBox->setCurrentText(scheme + "://");
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
    const static QStringList expectedSchemes { "smb", "ftp", "sftp" };
    QStringList searchList = SearchHistroyManager::instance()->getSearchHistroy();
    for (int i = searchList.count() - 1; i >= 0 && hosts.count() < kMaxHistoryItems - 1; --i) {
        QString urlStr = searchList.at(i);
        urlStr = urlStr.toLower();   // ....charset=UTF8 ==> charset=utf8
        urlStr.replace("/?", "?");   // ftp://1.2.3.4/?charset=xxx ==> ..3.4?charset=xxx
        while (urlStr.endsWith("/"))   // ftp://1.2.3.4/ ==> ftp://1.2.3.4
            urlStr.chop(1);

        QUrl url(urlStr);
        if (!expectedSchemes.contains(url.scheme()) || url.host().isEmpty())
            continue;

        CharsetOption opt = kDefault;
        const QString &query = url.query();
        if (!query.isEmpty()) {
            const static QRegularExpression charsetRegx(R"(charset=([^&]*))");
            auto match = charsetRegx.match(query);
            if (match.hasMatch()) {
                QString charset = match.captured(1);
                if (charset == kGBKCharset)
                    opt = kGbk;
                else if (charset == kUTF8CharSet || charset == kUTF8CharSet2)
                    opt = kUtf8;
            }
        }

        urlStr = urlStr.mid(0, urlStr.indexOf("?"));   // ftp://1.2.3.4?charset=xxx ==> ftp://1.2.3.4
        if (!hosts.contains(urlStr)) {
            hosts.insert(0, urlStr);
            serverComboBox->insertItem(0, urlStr, opt);
        }
    }
    completer->setModel(new QStringListModel(hosts));

    // select latest one
    if (hosts.count() > 0)
        onCurrentInputChanged(hosts.last());
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
        fmWarning() << "invalid url" << newUrlStr << "refuse to collect.";
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
        fmInfo() << "user passed the charset param in url." << url;
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
                         << QString("%1://").arg(Scheme::kSFtp);
        schemeComboBox->addItems(supportedSchemes);
        schemeComboBox->setFixedWidth(100);

        completer = new QCompleter(this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setMaxVisibleItems(kMaxHistoryItems);

        serverComboBox = new DComboBox(this);
        serverComboBox->addItem(tr("Clear History"));
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

        DLabel *emptyIcon = new DLabel();
        emptyIcon->setContentsMargins(120, 40, 0, 0);
        emptyIcon->setMaximumHeight(200);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            emptyIcon->setPixmap(QIcon(":icons/deepin/builtin/light/icons/no_favorites_yet.svg").pixmap({ 145, 145 }));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            emptyIcon->setPixmap(QIcon(":icons/deepin/builtin/dark/icons/no_favorites_yet.svg").pixmap({ 145, 145 }));

        DLabel *centerNotes = new DLabel();
        centerNotes->setMaximumHeight(30);
        centerNotes->setText(tr("No favorites yet"));
        centerNotes->setStyleSheet("color:gray;");

        emptyLayout->addWidget(emptyIcon, Qt::AlignHCenter);
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

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &ConnectToServerDialog::initUiForSizeMode);
#endif
}

void ConnectToServerDialog::onAddButtonClicked()
{
    const QString &currScheme = schemeComboBox->currentText();
    const QString &currHost = serverComboBox->currentText();
    if (currScheme.isEmpty() || currHost.isEmpty())
        return;

    const QString &fullUrlStr = getCurrentUrlString();
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
}

QString ConnectToServerDialog::schemeWithSlash(const QString &scheme) const
{
    return scheme + "://";
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
