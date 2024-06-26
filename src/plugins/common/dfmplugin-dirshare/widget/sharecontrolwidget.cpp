// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecontrolwidget.h"
#include "utils/usersharehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dialogs/smbsharepasswddialog/usersharepasswordsettingdialog.h>

#include <dfm-framework/dpf.h>

#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QCheckBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QStandardPaths>
#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QNetworkInterface>
#include <QTextBrowser>

#include <unistd.h>
#include <pwd.h>
#include <linux/limits.h>

using namespace dfmplugin_dirshare;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace ConstDef {
static constexpr int kKeyWidth { 80 };
static constexpr int kWidgetFixedWidth { 195 };
static constexpr char kShareNameRegx[] { "^[^\\[\\]\"'/\\\\:|<>+=;,?*\r\n\t]*$" };
static constexpr char kShareFileDir[] { "/var/lib/samba/usershares" };
}

class SectionKeyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit SectionKeyLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedWidth(DSizeModeHelper::element(ConstDef::kKeyWidth, ConstDef::kKeyWidth));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        this->setFixedWidth(DSizeModeHelper::element(ConstDef::kKeyWidth, ConstDef::kKeyWidth));
    });
#else
    setFixedWidth(ConstDef::kKeyWidth);
#endif
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T7, QFont::Medium);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

ShareControlWidget::ShareControlWidget(const QUrl &url, bool disableState, QWidget *parent)
    : DArrowLineDrawer(parent), url(url)
{
    setupUi(disableState);
    init();
    initConnection();
}

ShareControlWidget::~ShareControlWidget()
{
    dpfSignalDispatcher->unsubscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", this, &ShareControlWidget::updateWidgetStatus);
    dpfSignalDispatcher->unsubscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", this, &ShareControlWidget::updateWidgetStatus);
    dpfSignalDispatcher->unsubscribe("dfmplugin_dirshare", "signal_Share_RemoveShareFailed", this, &ShareControlWidget::updateWidgetStatus);

    if (refreshIp) {
        refreshIp->stop();
        refreshIp->deleteLater();
        refreshIp = nullptr;
    }
}

void ShareControlWidget::setOption(QWidget *w, const QVariantHash &option)
{
    if (option.contains("Option_Key_ExtendViewExpand")) {
        ShareControlWidget *view = dynamic_cast<ShareControlWidget *>(w);
        if (view)
            view->setExpand(option.value("Option_Key_ExtendViewExpand").toBool());
    }
}

void ShareControlWidget::setupUi(bool disableState)
{
    const QString &userName = getpwuid(getuid())->pw_name;
    isSharePasswordSet = UserShareHelperInstance->isUserSharePasswordSet(userName);
    setTitle(tr("Sharing"));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6, QFont::DemiBold);
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    QFrame *mainFrame = new QFrame(this);
    mainFrame->setDisabled(disableState);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 10);
    mainLayout->setSpacing(0);

    QFormLayout *basicInfoFrameLay = new QFormLayout(mainFrame);
    basicInfoFrameLay->setMargin(0);
    basicInfoFrameLay->setContentsMargins(20, 0, 10, 0);
    basicInfoFrameLay->setVerticalSpacing(6);

    setupShareSwitcher();
    basicInfoFrameLay->addRow(" ", shareSwitcher);
    setupShareNameEditor();
    basicInfoFrameLay->addRow(new SectionKeyLabel(tr("Share name"), this), shareNameEditor);
    setupSharePermissionSelector();
    basicInfoFrameLay->addRow(new SectionKeyLabel(tr("Permission"), this), sharePermissionSelector);
    setupShareAnonymousSelector();
    basicInfoFrameLay->addRow(new SectionKeyLabel(tr("Anonymous"), this), shareAnonymousSelector);

    // More share info
    moreInfoFrame = new QFrame(mainFrame);
    QVBoxLayout *moreInfoFrameLay = new QVBoxLayout(moreInfoFrame);
    moreInfoFrameLay->setMargin(0);
    moreInfoFrameLay->setContentsMargins(20, 10, 10, 0);
    moreInfoFrame->setLayout(moreInfoFrameLay);

    QFormLayout *formLay = new QFormLayout(moreInfoFrame);
    formLay->setMargin(0);
    formLay->setContentsMargins(0, 0, 0, 0);
    formLay->addRow(new SectionKeyLabel(tr("Network path"), this), setupNetworkPath());
    formLay->addRow(new SectionKeyLabel(tr("Username"), this), setupUserName());
    formLay->addRow(new SectionKeyLabel(tr("Share password"), this), setupSharePassword());
    moreInfoFrameLay->addLayout(formLay);
    setupShareNotes();
    moreInfoFrameLay->addWidget(m_shareNotes);

    mainLayout->addLayout(basicInfoFrameLay);
    mainLayout->addWidget(moreInfoFrame);
    mainFrame->setLayout(mainLayout);
    DFontSizeManager::instance()->bind(mainFrame, DFontSizeManager::SizeType::T7, QFont::Normal);
    setContent(mainFrame);

    timer = new QTimer(this);
    timer->setInterval(500);
}

void ShareControlWidget::setupShareSwitcher()
{
    shareSwitcher = new QCheckBox(this);
    shareSwitcher->setFixedWidth(ConstDef::kWidgetFixedWidth);
    QString text = tr("Share this folder");
    shareSwitcher->setToolTip(text);
    QFontMetrics fontWidth(shareSwitcher->font());
    int fontSize = fontWidth.horizontalAdvance(text);
    int fontW = shareSwitcher->width() - 10 - shareSwitcher->iconSize().width();
    if (fontSize > fontW) {
        text = fontWidth.elidedText(text, Qt::ElideMiddle, fontW);
    }
    shareSwitcher->setText(text);
}

void ShareControlWidget::setupShareNameEditor()
{
    shareNameEditor = new QLineEdit(this);

    QValidator *validator = new QRegularExpressionValidator(QRegularExpression(ConstDef::kShareNameRegx), this);
    shareNameEditor->setValidator(validator);

    connect(shareNameEditor, &QLineEdit::textChanged, this, [=](const QString &text) {
        QString newText(text);
        // daemon create the mountpoint of share: <name> on <host>, which occupied 255 bytes at most.
        // and only 255 - 20 bytes for share name.
        // the max length of folder name limited to 255 bytes in nativa file system.
        while (newText.toLocal8Bit().length() > (NAME_MAX - 20))
            newText.chop(1);
        shareNameEditor->setText(newText);
    });
}

void ShareControlWidget::setupSharePermissionSelector()
{
    sharePermissionSelector = new QComboBox(this);

    QPalette peMenuBg;
    QColor color = palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Window);
    peMenuBg.setColor(QPalette::Window, color);
    sharePermissionSelector->setPalette(peMenuBg);

    QStringList permissions { tr("Read and write"), tr("Read only") };
    sharePermissionSelector->addItems(permissions);
}

void ShareControlWidget::setupShareAnonymousSelector()
{
    shareAnonymousSelector = new QComboBox(this);

    QPalette peMenuBg;
    QColor color = palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Window);
    peMenuBg.setColor(QPalette::Window, color);
    shareAnonymousSelector->setPalette(peMenuBg);

    QStringList anonymousSelections { tr("Not allow"), tr("Allow") };
    shareAnonymousSelector->addItems(anonymousSelections);
}

QHBoxLayout *ShareControlWidget::setupNetworkPath()
{
    netScheme = new QLabel("smb://", this);

    networkAddrLabel = new QLabel("127.0.0.1", this);
    networkAddrLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    networkAddrLabel->setFixedWidth(ConstDef::kWidgetFixedWidth);

    copyNetAddr = new QPushButton(this);
    copyNetAddr->setFlat(true);
    copyNetAddr->setToolTip(tr("Copy"));
    auto setBtnIcon = [=] {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            copyNetAddr->setIcon(QIcon(":light/icons/property_bt_copy.svg"));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            copyNetAddr->setIcon(QIcon(":dark/icons/property_bt_copy.svg"));
    };
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, setBtnIcon);
    setBtnIcon();
    QObject::connect(copyNetAddr, &QPushButton::clicked, [=]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(netScheme->text() + networkAddrLabel->text());
    });

    QHBoxLayout *hBoxLine = new QHBoxLayout(this);
    hBoxLine->setContentsMargins(0, 0, 2, 0);
    hBoxLine->addWidget(netScheme);
    hBoxLine->addWidget(networkAddrLabel);
    hBoxLine->addWidget(copyNetAddr);

    return hBoxLine;
}

QHBoxLayout *ShareControlWidget::setupUserName()
{
    userNamelineLabel = new QLabel(this);
    userNamelineLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    userNamelineLabel->setText(getpwuid(getuid())->pw_name);
    userNamelineLabel->setFixedWidth(ConstDef::kWidgetFixedWidth);

    copyUserNameBt = new QPushButton(this);
    copyUserNameBt->setFlat(true);
    copyUserNameBt->setToolTip(tr("Copy"));
    auto setBtnIcon = [=] {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            copyUserNameBt->setIcon(QIcon(":light/icons/property_bt_copy.svg"));
        else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            copyUserNameBt->setIcon(QIcon(":dark/icons/property_bt_copy.svg"));
    };
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, setBtnIcon);
    setBtnIcon();
    QObject::connect(copyUserNameBt, &QPushButton::clicked, [=]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(userNamelineLabel->text());
    });

    QHBoxLayout *hBoxLine = new QHBoxLayout(this);
    hBoxLine->setContentsMargins(0, 0, 2, 0);
    hBoxLine->addWidget(userNamelineLabel);
    hBoxLine->addWidget(copyUserNameBt);

    return hBoxLine;
}

QHBoxLayout *ShareControlWidget::setupSharePassword()
{
    sharePassword = new DLabel(this);
    QFont font = this->font();
    int defaultFontSize = font.pointSize();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 5);
    font.setPointSize(isSharePasswordSet ? 5 : defaultFontSize);
    sharePassword->setFont(font);
    sharePassword->setAlignment(Qt::AlignJustify | Qt::AlignLeft | Qt::AlignVCenter);
    sharePassword->setText(isSharePasswordSet ? "●●●●●" : tr("None"));

    setPasswordBt = new DCommandLinkButton(tr("Set password"));
    setPasswordBt->setText(isSharePasswordSet ? tr("Change password") : tr("Set password"));
    setPasswordBt->setContentsMargins(0, 0, 0, 0);
    setPasswordBt->setToolTip(setPasswordBt->text());
    QObject::connect(setPasswordBt, &QPushButton::clicked, [this]() {
        showSharePasswordSettingsDialog();
    });

    QHBoxLayout *hBoxLine = new QHBoxLayout(this);
    hBoxLine->setMargin(0);
    hBoxLine->setStretch(0, 1);
    hBoxLine->addWidget(sharePassword);
    hBoxLine->addWidget(setPasswordBt);

    return hBoxLine;
}

void ShareControlWidget::setupShareNotes()
{
    static QString notice = tr("This password will be applied to all shared folders, and users without the password can only access shared folders that allow anonymous access. ");
    m_shareNotes = new DTipLabel(notice, this);
    m_shareNotes->setWordWrap(true);
    m_shareNotes->setAlignment(Qt::AlignLeft);

    // from chenke, any application changes the palette, then they should
    // be responsibility for the styles.
    // the parent of shareNotes is changed outside: FilePropertyDialog::initInfoUI()
    auto setTextColor = [this](DGuiApplicationHelper::ColorType type) {
        QPalette pal = m_shareNotes->palette();
        if (type == DGuiApplicationHelper::LightType) {
            pal.setColor(QPalette::Active, QPalette::Text, QColor(0, 0, 0, 0.6 * 255));
            pal.setColor(QPalette::Inactive, QPalette::Text, QColor(0, 0, 0, 0.3 * 255));
        } else {
            pal.setColor(QPalette::Active, QPalette::Text, QColor(255, 255, 255, 0.7 * 255));
            pal.setColor(QPalette::Inactive, QPalette::Text, QColor(255, 255, 255, 0.2 * 255));
        }
        m_shareNotes->setPalette(pal);
        dynamic_cast<DLabel *>(m_shareNotes)->setForegroundRole(QPalette::Text);
    };

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, setTextColor);
    setTextColor(DGuiApplicationHelper::instance()->themeType());
}

void ShareControlWidget::init()
{
    info = InfoFactory::create<FileInfo>(url);
    if (!info) {
        fmWarning() << "cannot create file info of " << url;
        return;
    }

    if (!watcher) {
        watcher = WatcherFactory::create<AbstractFileWatcher>(info->urlOf(UrlInfoType::kParentUrl));
        watcher->startWatcher();
    }

    QString filePath = url.path();
    auto shareName = UserShareHelperInstance->shareNameByPath(filePath);
    if (shareName.isEmpty())
        shareName = info->displayOf(DisPlayInfoType::kFileDisplayName);
    shareNameEditor->setText(shareName);

    bool isShared = UserShareHelperInstance->isShared(filePath);
    shareSwitcher->setChecked(isShared);
    if (isShared) {
        auto shareInfo = UserShareHelperInstance->shareInfoByPath(filePath);
        sharePermissionSelector->setCurrentIndex(shareInfo.value(ShareInfoKeys::kWritable).toBool() ? 0 : 1);
        shareAnonymousSelector->setCurrentIndex(shareInfo.value(ShareInfoKeys::kAnonymous).toBool() ? 1 : 0);
    }

    sharePermissionSelector->setEnabled(isShared);
    shareAnonymousSelector->setEnabled(isShared);
}

void ShareControlWidget::initConnection()
{
    connect(shareSwitcher, &QCheckBox::clicked, this, [this](bool checked) {
        this->userShareOperation(checked);
    });

    connect(shareAnonymousSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShareControlWidget::updateShare);
    connect(sharePermissionSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShareControlWidget::updateShare);
    connect(shareNameEditor, &QLineEdit::editingFinished, this, &ShareControlWidget::updateShare);
    connect(UserShareHelper::instance(), &UserShareHelper::sambaPasswordSet, this, &ShareControlWidget::onSambaPasswordSet);

    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", this, &ShareControlWidget::updateWidgetStatus);
    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", this, &ShareControlWidget::updateWidgetStatus);
    dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_RemoveShareFailed", this, &ShareControlWidget::updateWidgetStatus);

    connect(watcher.data(), &AbstractFileWatcher::fileRename, this, &ShareControlWidget::updateFile);

    // the timer is used to control the frequency of switcher action.
    connect(timer, &QTimer::timeout, this, [this] { shareSwitcher->setEnabled(true); });

    // this timer is used to refresh shared ip address every 2 seconds.
    refreshIp = new QTimer();
    refreshIp->setInterval(0);
    connect(refreshIp, &QTimer::timeout, this, [this]() {
        selfIp = UserShareHelper::instance()->sharedIP();
        int port = UserShareHelper::instance()->getSharePort();
        if (port != -1)
            selfIp += QString(":%1").arg(port);
        if (networkAddrLabel->text() != selfIp) {
            networkAddrLabel->setText(selfIp);
        }
        refreshIp->setInterval(2000);
    });

    showMoreInfo(shareSwitcher->isChecked());
}

bool ShareControlWidget::validateShareName()
{
    const QString &name = shareNameEditor->text().trimmed();
    if (name.isEmpty())
        return false;

    if (name == ".." || name == ".") {
        DialogManagerInstance->showErrorDialog(tr("The share name must not be two dots (..) or one dot (.)"), "");
        return false;
    }

    bool isShared = UserShareHelperInstance->isShared(url.path());
    if (isShared) {
        const auto &&sharedName = UserShareHelperInstance->shareNameByPath(url.path());
        if (sharedName == name.toLower())
            return true;
    }

    const auto &&shareFileLst = QDir(ConstDef::kShareFileDir).entryInfoList(QDir::Files);
    for (const auto &shareFile : shareFileLst) {
        if (name.toLower() == shareFile.fileName()) {
            DDialog dlg(this);
            dlg.setIcon(QIcon::fromTheme("dialog-warning"));

            if (!shareFile.isWritable()) {
                dlg.setTitle(tr("The share name is used by another user."));
                dlg.addButton(tr("OK", "button"), true);
            } else {
                dlg.setTitle(tr("The share name already exists. Do you want to replace the shared folder?"));
                dlg.addButton(tr("Cancel", "button"), true);
                dlg.addButton(tr("Replace", "button"), false, DDialog::ButtonWarning);
            }

            if (dlg.exec() != DDialog::Accepted) {
                if(isShared){
                    QString filePath = url.path();
                    auto shareName = UserShareHelperInstance->shareNameByPath(filePath);
                    shareNameEditor->setText(shareName);
                    shareSwitcher->setChecked(isShared);
                }
                shareNameEditor->setFocus();
                return false;
            }
            break;
        }
    }

    return true;
}

void ShareControlWidget::updateShare()
{
   if (!isUpdating)
       shareFolder();
   return;
}

void ShareControlWidget::shareFolder()
{
    bool isShared = UserShareHelperInstance->isShared(url.path());
    if (!shareSwitcher->isChecked())
        return;
    isUpdating = true;
    if (!validateShareName()) {
        if(!isShared){
            shareSwitcher->setChecked(false);
            sharePermissionSelector->setEnabled(false);
            shareAnonymousSelector->setEnabled(false);
        }
        isUpdating = false;
        return;
    }

    bool writable = sharePermissionSelector->currentIndex() == 0;
    bool anonymous = shareAnonymousSelector->currentIndex() == 1;
    if (anonymous) {   // set the directory's access permission to 777
        // 1. set the permission of shared folder to 777;
        DFMIO::DFile file(url);
        if (file.exists() && writable) {
            using namespace DFMIO;
            bool ret = file.setPermissions(file.permissions() | DFile::Permission::kWriteGroup | DFile::Permission::kExeGroup
                                           | DFile::Permission::kWriteOther | DFile::Permission::kExeOther);
            if (!ret)
                fmWarning() << "set permission of " << url << "failed.";
        }

        // 2. set the mode 'other' of  /home/$USER to r-x when enable anonymous access,
        // otherwise the anonymous user cannot mount the share successfully.
        // and never change the mode of /root
        if (getuid() != 0) {
            QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            DFMIO::DFile home(homePath);
            if (home.exists()) {
                using namespace DFMIO;
                bool ret = home.setPermissions(home.permissions() | DFile::Permission::kReadOther | DFile::Permission::kExeOther);
                if (!ret)
                    fmWarning() << "set permission for user home failed: " << homePath;
            }
        }
    }
    ShareInfo info {
        { ShareInfoKeys::kName, shareNameEditor->text().trimmed().toLower() },
        { ShareInfoKeys::kPath, url.path() },
        { ShareInfoKeys::kComment, "" },
        { ShareInfoKeys::kWritable, writable },
        { ShareInfoKeys::kAnonymous, anonymous }
    };
    bool success = UserShareHelperInstance->share(info);
    if (!success) {
        shareSwitcher->setChecked(false);
        sharePermissionSelector->setEnabled(false);
        shareAnonymousSelector->setEnabled(false);
    }
    isUpdating = false;
}

void ShareControlWidget::unshareFolder()
{
    UserShareHelperInstance->removeShareByPath(url.path());
}

void ShareControlWidget::updateWidgetStatus(const QString &filePath)
{
    if (filePath != url.path())
        return;

    auto shareInfo = UserShareHelperInstance->shareInfoByPath(filePath);
    bool valid = !shareInfo.value(ShareInfoKeys::kName).toString().isEmpty()
            && QFile(shareInfo.value(ShareInfoKeys::kPath).toString()).exists();
    if (valid) {
        shareSwitcher->setChecked(true);
        const auto &&name = shareInfo.value(ShareInfoKeys::kName).toString();
        shareNameEditor->setText(name);
        if (shareInfo.value(ShareInfoKeys::kWritable).toBool())
            sharePermissionSelector->setCurrentIndex(0);
        else
            sharePermissionSelector->setCurrentIndex(1);

        if (shareInfo.value(ShareInfoKeys::kAnonymous).toBool())
            shareAnonymousSelector->setCurrentIndex(1);
        else
            shareAnonymousSelector->setCurrentIndex(0);

        uint shareUid = UserShareHelperInstance->whoShared(name);
        if ((shareUid != info->extendAttributes(ExtInfoType::kOwnerId).toUInt() || shareUid != getuid()) && getuid() != 0)
            this->setEnabled(false);

        sharePermissionSelector->setEnabled(true);
        shareAnonymousSelector->setEnabled(true);
    } else {
        shareSwitcher->setChecked(false);
        sharePermissionSelector->setEnabled(false);
        shareAnonymousSelector->setEnabled(false);
    }
}

void ShareControlWidget::updateFile(const QUrl &oldOne, const QUrl &newOne)
{
    if (UniversalUtils::urlEquals(oldOne, url))
        url = newOne;
    init();
}

void ShareControlWidget::onSambaPasswordSet(bool result)
{
    isSharePasswordSet = result;

    QFont font = sharePassword->font();
    int defaultFontSize = font.pointSize();
    font.setPointSize(isSharePasswordSet ? 5 : defaultFontSize);
    sharePassword->setFont(font);
    sharePassword->setFixedWidth(isSharePasswordSet ? ConstDef::kWidgetFixedWidth - 140 : ConstDef::kWidgetFixedWidth - 128);
    sharePassword->setText(isSharePasswordSet ? "●●●●●" : tr("None"));
    setPasswordBt->setText(isSharePasswordSet ? tr("Change password") : tr("Set password"));
}

void ShareControlWidget::showMoreInfo(bool showMore)
{
    moreInfoFrame->setHidden(!showMore);

    if (refreshIp) {
        if (showMore)
            refreshIp->start();
        else
            refreshIp->stop();
    }
}

void ShareControlWidget::userShareOperation(bool checked)
{
    if (!isSharePasswordSet && checked)
        showSharePasswordSettingsDialog();

    sharePermissionSelector->setEnabled(checked);
    shareAnonymousSelector->setEnabled(checked);
    shareSwitcher->setEnabled(false);
    timer->start();
    if (checked)
        this->shareFolder();
    else
        this->unshareFolder();

    showMoreInfo(checked);
}

void ShareControlWidget::showSharePasswordSettingsDialog()
{
    if (this->property("UserSharePwdSettingDialogShown").toBool())
        return;
    UserSharePasswordSettingDialog *dialog = new UserSharePasswordSettingDialog(this);
    dialog->show();
    dialog->moveToCenter();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::finished, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    this->setProperty("UserSharePwdSettingDialogShown", true);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::inputPassword, [=](const QString &password) {
        QString userName = UserShareHelperInstance->currentUserName();
        UserShareHelperInstance->setSambaPasswd(userName, password);
    });
    QObject::connect(dialog, &UserSharePasswordSettingDialog::closed, [=] {
        this->setProperty("UserSharePwdSettingDialogShown", false);
    });
}

#include "sharecontrolwidget.moc"
