// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "permissionmanagerwidget.h"
#include "events/propertyeventcall.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>

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
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);

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
        executableCheckBox->hide();
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
    if (info->extendAttributes(ExtInfoType::kOwnerId).toUInt() != getuid() || !canChmod(info) || cannotChmodFsType.contains(fsType) || pluginAshResult) {
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

    setExpand(false);

    QFormLayout *layout = new QFormLayout;

    QFrame *frame = new QFrame(this);

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

    ownerComboBox = new QComboBox(this);
    ownerComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    ownerComboBox->setFixedWidth(196);

    groupComboBox = new QComboBox(this);
    groupComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    groupComboBox->setFixedWidth(196);

    otherComboBox = new QComboBox(this);
    otherComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    otherComboBox->setFixedWidth(196);

    executableCheckBox = new QCheckBox(this);
    executableCheckBox->setText(tr("Allow to execute as program"));
    executableCheckBox->setFixedWidth(196);
    executableCheckBox->setToolTip(executableCheckBox->text());

    layout->setLabelAlignment(Qt::AlignLeft);

    DLabel *owner = new DLabel(QObject::tr("Owner"), this);
    DFontSizeManager::instance()->bind(owner, DFontSizeManager::SizeType::T7, QFont::DemiBold);
    DLabel *group = new DLabel(QObject::tr("Group"), this);
    DFontSizeManager::instance()->bind(group, DFontSizeManager::SizeType::T7, QFont::DemiBold);
    DLabel *other = new DLabel(QObject::tr("Others"), this);
    DFontSizeManager::instance()->bind(other, DFontSizeManager::SizeType::T7, QFont::DemiBold);

#ifdef DTKWIDGET_CLASS_DSizeMode
    owner->setFixedWidth(DSizeModeHelper::element(60, 107));
    group->setFixedWidth(DSizeModeHelper::element(60, 107));
    other->setFixedWidth(DSizeModeHelper::element(60, 107));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [owner, group, other]() {
        owner->setFixedWidth(DSizeModeHelper::element(60, 107));
        group->setFixedWidth(DSizeModeHelper::element(60, 107));
        other->setFixedWidth(DSizeModeHelper::element(60, 107));
    });
#else
    owner->setFixedWidth(107);
    group->setFixedWidth(107);
    other->setFixedWidth(107);
#endif

    layout->addRow(owner, ownerComboBox);
    layout->addRow(group, groupComboBox);
    layout->addRow(other, otherComboBox);
    layout->addRow(" ", executableCheckBox);

    layout->setContentsMargins(15, 10, 10, 10);

    frame->setLayout(layout);
    setContent(frame);
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

    FileInfoPointer info = InfoFactory::create<FileInfo>(selectUrl);
    if (info.isNull())
        return;

    if (isChecked) {
        PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(), selectUrl, info->permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    } else {
        PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(), selectUrl, info->permissions() & ~(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
    }
}

bool PermissionManagerWidget::canChmod(const FileInfoPointer &info)
{
    if (info.isNull())
        return false;

    if (!info->canAttributes(CanableInfoType::kCanRename))
        return false;

    QString path = info->pathOf(PathInfoType::kFilePath);
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                     QRegularExpression::DotMatchesEverythingOption
                                             | QRegularExpression::DontCaptureOption
                                             | QRegularExpression::OptimizeOnFirstUsageOption);
    if (regExp.match(path, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch())
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
    FileInfoPointer info = InfoFactory::create<FileInfo>(selectUrl);
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
    //点击combobox都需要保持执行权限，否则将失去相关权限位
    ownerFlags |= (permissions & QFile::ExeOwner);
    groupFlags |= (permissions & QFile::ExeGroup);
    otherFlags |= (permissions & QFile::ExeOther);
    PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(), selectUrl, QFileDevice::Permissions(ownerFlags) | QFileDevice::Permissions(groupFlags) | QFileDevice::Permissions(otherFlags));

    infoBytes = info->pathOf(PathInfoType::kAbsoluteFilePath).toUtf8();
    stat(infoBytes.data(), &fileStat);
    auto afterMode = fileStat.st_mode;
    // 修改权限失败
    if (preMode == afterMode) {
        qDebug() << "chmod failed";
        QSignalBlocker b1(ownerComboBox), b2(groupComboBox), b3(otherComboBox);
        Q_UNUSED(b1);
        Q_UNUSED(b2);
        Q_UNUSED(b3);
        setComboBoxByPermission(ownerComboBox, static_cast<int>(info->permissions() & kOwerAll), 12);
        setComboBoxByPermission(groupComboBox, static_cast<int>(info->permissions() & kGroupAll), 4);
        setComboBoxByPermission(otherComboBox, static_cast<int>(info->permissions() & kOtherAll), 0);
    }
}
