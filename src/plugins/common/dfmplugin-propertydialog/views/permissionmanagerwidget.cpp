// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "permissionmanagerwidget.h"
#include "events/propertyeventcall.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-framework/event/event.h>

#include <DFontSizeManager>
#include <DLabel>
#include <DGuiApplicationHelper>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QApplication>
#include <QStorageInfo>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>

Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

static constexpr int kLabelWidth { 75 };
constexpr int kOwerAll = QFile::ExeOwner | QFile::WriteOwner | QFile::ReadOwner;
constexpr int kGroupAll = QFile::ExeGroup | QFile::WriteGroup | QFile::ReadGroup;
constexpr int kOtherAll = QFile::ExeOther | QFile::WriteOther | QFile::ReadOther;

PermissionManagerWidget::PermissionManagerWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
}

PermissionManagerWidget::~PermissionManagerWidget()
{
}

void PermissionManagerWidget::selectFileUrl(const QUrl &url)
{
    selectUrl = url;
    FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

    if (info.isNull())
        return;

    QUrl parentUrl = info->urlOf(UrlInfoType::kParentUrl);
    QStorageInfo storageInfo(parentUrl.toLocalFile());
    const QString &fsType = storageInfo.fileSystemType();

    if (info->isAttributes(OptInfoType::kIsFile)) {
        // append `Executable` string
        QString append = QStringLiteral(" , ") + QObject::tr("Executable");
        authorityList[3] += append;
        authorityList[5] += append;
        authorityList[7] += append;

        // file: read is read, read-write is read-write
        readOnlyIndex = readOnlyFlag;
        readWriteIndex = readWriteFlag;
    }

    if (info->isAttributes(OptInfoType::kIsDir)) {
        // folder: read is read and executable, read-write is read-write and executable
        readOnlyIndex = readOnlyWithXFlag;
        readWriteIndex = readWriteWithXFlag;
        executableFrame->hide();
    }

    ownerComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOwner | QFile::ReadOwner));
    ownerComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOwner));
    groupComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteGroup | QFile::ReadGroup));
    groupComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadGroup));
    otherComboBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOther | QFile::ReadOther));
    otherComboBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOther));

    setComboBoxByPermission(ownerComboBox, static_cast<int>(info->permissions() & kOwerAll), 12);
    setComboBoxByPermission(groupComboBox, static_cast<int>(info->permissions() & kGroupAll), 4);
    setComboBoxByPermission(otherComboBox, static_cast<int>(info->permissions() & kOtherAll), 0);

    if (info->isAttributes(OptInfoType::kIsFile)) {
        executableCheckBox->setText(tr("Allow to execute as program"));
        if (info->extendAttributes(ExtInfoType::kOwnerId).toUInt() != getuid())
            executableCheckBox->setDisabled(true);

        if (info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther))
            executableCheckBox->setChecked(true);

        // 一些文件系统不支持修改可执行权限
        if (!canChmod(info) || cannotChmodFsType.contains(fsType))
            executableCheckBox->setDisabled(true);

        connect(executableCheckBox, &QCheckBox::toggled, this, &PermissionManagerWidget::toggleFileExecutable);
    }

    // 置灰：
    // 1. 本身用户无权限
    // 2. 所属文件系统无权限机制
    // 3. Plugin-in signal event requirements grayed out
    bool pluginAshResult { false };
    dpfHookSequence->run("dfmplugin_propertydialog", "hook_PermissionView_Ash", url, &pluginAshResult);
    if (!canChmod(info) || cannotChmodFsType.contains(fsType) || pluginAshResult) {
        ownerComboBox->setDisabled(true);
        groupComboBox->setDisabled(true);
        otherComboBox->setDisabled(true);
    }

    // tmp: 暂时的处理
    if (fsType == "vfat" && !info->isAttributes(OptInfoType::kIsDir))
        ownerComboBox->setDisabled(false);

    connect(ownerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
    connect(groupComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
    connect(otherComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PermissionManagerWidget::onComboBoxChanged);
}

void PermissionManagerWidget::updateFileUrl(const QUrl &url)
{
    selectUrl = url;
}

void PermissionManagerWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);
    setTitle(QString(tr("Permissions")));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6, QFont::DemiBold);
    setExpand(false);

    authorityList << QObject::tr("Access denied")   // 0
                  << QObject::tr("Executable")   // 1
                  << QObject::tr("Write only")   // 2
                  << QObject::tr("Write only")   // 3 with x
                  << QObject::tr("Read only")   // 4
                  << QObject::tr("Read only")   // 5 with x
                  << QObject::tr("Read-write")   // 6
                  << QObject::tr("Read-write");   // 7 with x

    cannotChmodFsType << "vfat"
                      << "fuseblk"
                      << "cifs";

    DLabel *owner = new DLabel(QObject::tr("Owner"), this);
    DFontSizeManager::instance()->bind(owner, DFontSizeManager::SizeType::T7, QFont::Medium);
    ownerComboBox = new QComboBox(this);

    DLabel *group = new DLabel(QObject::tr("Group"), this);
    DFontSizeManager::instance()->bind(group, DFontSizeManager::SizeType::T7, QFont::Medium);
    groupComboBox = new QComboBox(this);

    DLabel *other = new DLabel(QObject::tr("Others"), this);
    DFontSizeManager::instance()->bind(other, DFontSizeManager::SizeType::T7, QFont::Medium);
    otherComboBox = new QComboBox(this);

    executableCheckBox = new QCheckBox(this);
    executableCheckBox->setText(tr("Allow to execute as program"));
    executableCheckBox->setToolTip(executableCheckBox->text());

#ifdef DTKWIDGET_CLASS_DSizeMode
    owner->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
    group->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
    other->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [owner, group, other]() {
        owner->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
        group->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
        other->setFixedWidth(DSizeModeHelper::element(kLabelWidth, kLabelWidth));
    });
#else
    owner->setFixedWidth(kLabelWidth);
    group->setFixedWidth(kLabelWidth);
    other->setFixedWidth(kLabelWidth);
#endif

    QFrame *mainFrame = new QFrame(this);

    QVBoxLayout *mainFrameLay = new QVBoxLayout(mainFrame);
    mainFrameLay->setContentsMargins(0, 0, 0, 0);
    mainFrameLay->setContentsMargins(0, 0, 0, 10);

    QFormLayout *formLay = new QFormLayout;
    formLay->setContentsMargins(0, 0, 0, 0);
    formLay->setContentsMargins(10, 10, 10, 0);
    formLay->setLabelAlignment(Qt::AlignLeft);
    formLay->setSpacing(11);
    formLay->addRow(owner, ownerComboBox);
    formLay->addRow(group, groupComboBox);
    formLay->addRow(other, otherComboBox);

    executableFrame = new QFrame(mainFrame);
    QHBoxLayout *exeLay = new QHBoxLayout;
    exeLay->setContentsMargins(0, 0, 0, 0);
    exeLay->setContentsMargins(0, 0, 0, 0);
    exeLay->setSpacing(0);
    exeLay->addSpacing(95);
    exeLay->addWidget(executableCheckBox);
    executableFrame->setLayout(exeLay);

    mainFrameLay->addLayout(formLay);
    mainFrameLay->addWidget(executableFrame);
    DFontSizeManager::instance()->bind(mainFrame, DFontSizeManager::SizeType::T7, QFont::Normal);

    mainFrame->setLayout(mainFrameLay);
    setContent(mainFrame);
    updateBackgroundColor();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &PermissionManagerWidget::updateBackgroundColor);
}

void PermissionManagerWidget::updateBackgroundColor()
{
    QPalette palette = this->palette();

    QColor bgColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        bgColor.setRgb(255, 255, 255);
    else
        bgColor.setRgb(40, 40, 40);

    palette.setColor(QPalette::Window, bgColor);
    setPalette(palette);

    // 此处需要设置一下combobox的弹出列表的调色板，否则会带上自定义的背景色
    if (ownerComboBox && groupComboBox && otherComboBox) {
        ownerComboBox->view()->setPalette(palette);
        groupComboBox->view()->setPalette(palette);
        otherComboBox->view()->setPalette(palette);
    }
}

QString PermissionManagerWidget::getPermissionString(int enumFlag)
{
    enumFlag &= 0x0007;
    if (enumFlag < 0 || enumFlag > 7) {
        return QStringLiteral("..what?");
    } else {
        return authorityList[enumFlag];
    }
}

void PermissionManagerWidget::setComboBoxByPermission(QComboBox *cb, int permission, int offset)
{
    int index = permission >> offset;
    if (index == readWriteFlag || index == readWriteWithXFlag) {
        cb->setCurrentIndex(0);
    } else if (index == readOnlyFlag || index == readOnlyWithXFlag) {
        cb->setCurrentIndex(1);
    } else {
        if (cb->count() < 3)
            cb->addItem(getPermissionString(index), QVariant(permission));
        cb->setCurrentIndex(2);
    }
}

void PermissionManagerWidget::toggleFileExecutable(bool isChecked)
{

    FileInfoPointer info = InfoFactory::create<FileInfo>(selectUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info.isNull())
        return;

    auto winID = qApp->activeWindow() ? qApp->activeWindow()->winId() : 0;
    if (isChecked) {
        PropertyEventCall::sendSetPermissionManager(winID, selectUrl, info->permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    } else {
        PropertyEventCall::sendSetPermissionManager(winID, selectUrl, info->permissions() & ~(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
    }
}

bool PermissionManagerWidget::canChmod(const FileInfoPointer &info)
{
    if (info.isNull())
        return false;

    // Check if file exists
    if (!info->exists())
        return false;

    // 禁止对库目录进行权限修改
    if (SystemPathUtil::instance()->isSystemPath(info->pathOf(PathInfoType::kAbsoluteFilePath)))
        return false;

    // Root user can always chmod
    if (SysInfoUtils::isRootUser())
        return true;

    // Check if current user is the owner of the file
    uint currentUid = static_cast<uint>(getuid());
    uint fileOwnerId = info->extendAttributes(ExtInfoType::kOwnerId).toUInt();

    // Only the owner of the file can change its permissions
    if (currentUid != fileOwnerId)
        return false;

    // Check if this is a local file (non-virtual, non-remote)
    if (!info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool())
        return false;

    return true;
}

void PermissionManagerWidget::setExecText()
{
    executableCheckBox->adjustSize();
    QString text = tr("Allow to execute as program");
    QFontMetrics fontWidth(executableCheckBox->font());
    int fontSize = fontWidth.horizontalAdvance(text);
    int fontW = executableCheckBox->width() - executableCheckBox->iconSize().width() - this->getContent()->layout()->contentsMargins().right() - contentsMargins().right();
    if (fontSize > fontW) {
        text = fontWidth.elidedText(text, Qt::ElideMiddle, fontW);
    }

    executableCheckBox->setText(text);
}

void PermissionManagerWidget::paintEvent(QPaintEvent *evt)
{
    setExecText();
    DArrowLineDrawer::paintEvent(evt);
}

void PermissionManagerWidget::onComboBoxChanged()
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(selectUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info.isNull())
        return;

    struct stat fileStat;
    QByteArray infoBytes(info->pathOf(PathInfoType::kAbsoluteFilePath).toUtf8());
    stat(infoBytes.data(), &fileStat);
    auto preMode = fileStat.st_mode;
    int ownerFlags = ownerComboBox->currentData().toInt();
    int groupFlags = groupComboBox->currentData().toInt();
    int otherFlags = otherComboBox->currentData().toInt();
    QFile::Permissions permissions = info->permissions();
    // 点击combobox都需要保持执行权限，否则将失去相关权限位
    ownerFlags |= (permissions & QFile::ExeOwner);
    groupFlags |= (permissions & QFile::ExeGroup);
    otherFlags |= (permissions & QFile::ExeOther);
    auto winID = qApp->activeWindow() ? qApp->activeWindow()->winId() : 0;
    PropertyEventCall::sendSetPermissionManager(winID, selectUrl, QFileDevice::Permissions(ownerFlags) | QFileDevice::Permissions(groupFlags) | QFileDevice::Permissions(otherFlags));

    infoBytes = info->pathOf(PathInfoType::kAbsoluteFilePath).toUtf8();
    stat(infoBytes.data(), &fileStat);
    auto afterMode = fileStat.st_mode;
    // 修改权限失败
    if (preMode == afterMode) {
        fmDebug() << "chmod failed";
        QSignalBlocker b1(ownerComboBox), b2(groupComboBox), b3(otherComboBox);
        Q_UNUSED(b1);
        Q_UNUSED(b2);
        Q_UNUSED(b3);
        setComboBoxByPermission(ownerComboBox, static_cast<int>(info->permissions() & kOwerAll), 12);
        setComboBoxByPermission(groupComboBox, static_cast<int>(info->permissions() & kGroupAll), 4);
        setComboBoxByPermission(otherComboBox, static_cast<int>(info->permissions() & kOtherAll), 0);
    }
}
