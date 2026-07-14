// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multifilepermissionwidget.h"
#include "events/propertyeventcall.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DFrame>

#include <QVBoxLayout>
#include <QFormLayout>

#include <sys/unistd.h>

Q_DECLARE_METATYPE(bool *)

using namespace dfmplugin_propertydialog;
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

inline constexpr int kLabelWidth { 75 };
inline constexpr int kExtendedWidgetWidth { 360 };

MultiFilePermissionWidget::MultiFilePermissionWidget(const QList<QUrl> &urls,
                                                     QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
    loadData(urls);
    // 先执行完数据加载，然后关联信号
    initConnect();
}

void MultiFilePermissionWidget::getOrgPermissonBoxState(FilePropertyState &states)
{
    states.ownerIndex = ownerComboBox->currentIndex();
    states.groupIndex = groupComboBox->currentIndex();
    states.otherIndex = otherComboBox->currentIndex();
}

void MultiFilePermissionWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);
    setExpand(false);
    setFixedWidth(kExtendedWidgetWidth);

    setTitle(tr("Permissions"));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType::T6,
                                       QFont::DemiBold);

    DLabel *owner = new DLabel(QObject::tr("Owner"), this);
    DFontSizeManager::instance()->bind(owner, DFontSizeManager::SizeType::T7,
                                       QFont::Medium);
    owner->setFixedWidth(kLabelWidth);
    ownerComboBox = new DComboBox(this);
    ownerComboBox->addItem(QObject::tr("Read-write"));
    ownerComboBox->addItem(QObject::tr("Read only"));

    DLabel *group = new DLabel(QObject::tr("Group"), this);
    DFontSizeManager::instance()->bind(group, DFontSizeManager::SizeType::T7,
                                       QFont::Medium);
    group->setFixedWidth(kLabelWidth);
    groupComboBox = new DComboBox(this);
    groupComboBox->addItem(QObject::tr("Read-write"));
    groupComboBox->addItem(QObject::tr("Read only"));

    DLabel *other = new DLabel(QObject::tr("Others"), this);
    DFontSizeManager::instance()->bind(other, DFontSizeManager::SizeType::T7,
                                       QFont::Medium);
    other->setFixedWidth(kLabelWidth);
    otherComboBox = new DComboBox(this);
    otherComboBox->addItem(QObject::tr("Read-write"));
    otherComboBox->addItem(QObject::tr("Read only"));

    DFrame *mainFrame = new DFrame(this);
    mainFrame->setFrameShape(QFrame::Shape::NoFrame);
    QVBoxLayout *mainFrameLay = new QVBoxLayout(mainFrame);
    mainFrameLay->setContentsMargins(0, 0, 0, 10);

    QFormLayout *formLay = new QFormLayout;
    formLay->setContentsMargins(10, 10, 10, 0);
    formLay->setLabelAlignment(Qt::AlignLeft);
    formLay->setSpacing(11);
    formLay->addRow(owner, ownerComboBox);
    formLay->addRow(group, groupComboBox);
    formLay->addRow(other, otherComboBox);

    mainFrameLay->addLayout(formLay);
    DFontSizeManager::instance()->bind(mainFrame, DFontSizeManager::SizeType::T7,
                                       QFont::Normal);
    mainFrame->setLayout(mainFrameLay);
    setContent(mainFrame);
    updateComboBoxViewPalette();
    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::themeTypeChanged,
            this, &MultiFilePermissionWidget::updateComboBoxViewPalette);
}

void MultiFilePermissionWidget::loadData(const QList<QUrl> &urls)
{
    int count = urls.size();
    if (count < 1) {
        fmCritical() << "No files to load permissions.";
        return;
    }

    if (!canChmodByFs(urls.at(0))) {
        disablePermissionComboBox();
        return;
    }

    constexpr int kOwnerAll = QFile::WriteOwner | QFile::ReadOwner;
    constexpr int kGroupAll = QFile::WriteGroup | QFile::ReadGroup;
    constexpr int kOtherAll = QFile::WriteOther | QFile::ReadOther;

    int firstOwnerPerm = -1;
    int firstGroupPerm = -1;
    int firstOtherPerm = -1;
    bool ownerPermConsistent = true;
    bool groupPermConsistent = true;
    bool otherPermConsistent = true;

    bool hasCanNotChmodFile { false };
    for (int i = 0; i < count; ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(urls.at(i));
        if (info.isNull()) {
            fmCritical() << "Failed to get file info: " << urls.at(i).path();
            disablePermissionComboBox();
            return;
        }

        if (!hasCanNotChmodFile && !canChmodByFile(info)) {
            fmCritical() << "The file can not chmod: " << urls.at(i).path();
            disablePermissionComboBox();
            hasCanNotChmodFile = true;
        }

        int ownerPerm = static_cast<int>(info->permissions() & kOwnerAll);
        int groupPerm = static_cast<int>(info->permissions() & kGroupAll);
        int otherPerm = static_cast<int>(info->permissions() & kOtherAll);

        if (i == 0) {
            firstOwnerPerm = ownerPerm;
            firstGroupPerm = groupPerm;
            firstOtherPerm = otherPerm;
        } else {
            if (ownerPerm != firstOwnerPerm)
                ownerPermConsistent = false;
            if (groupPerm != firstGroupPerm)
                groupPermConsistent = false;
            if (otherPerm != firstOtherPerm)
                otherPermConsistent = false;
        }
    }

    // 设置拥有者权限下拉框
    if (ownerPermConsistent && firstOwnerPerm != -1) {
        if ((firstOwnerPerm & QFile::WriteOwner)
                && (firstOwnerPerm & QFile::ReadOwner)) {
            ownerComboBox->setCurrentIndex(0);   // Read-write
        } else if (firstOwnerPerm & QFile::ReadOwner) {
            ownerComboBox->setCurrentIndex(1);   // Read only
        } else {
            ownerComboBox->addItem(tr("No read/write"));
            ownerComboBox->setCurrentIndex(2);
        }
    } else {
        ownerComboBox->addItem(tr("--"));
        ownerComboBox->setCurrentIndex(2);
    }

    // 设置组用户权限下拉框
    if (groupPermConsistent && firstGroupPerm != -1) {
        if ((firstGroupPerm & QFile::WriteGroup)
                && (firstGroupPerm & QFile::ReadGroup)) {
            groupComboBox->setCurrentIndex(0);   // Read-write
        } else if (firstGroupPerm & QFile::ReadGroup) {
            groupComboBox->setCurrentIndex(1);   // Read only
        } else {
            groupComboBox->addItem(tr("No read/write"));
            groupComboBox->setCurrentIndex(2);
        }
    } else {
        groupComboBox->addItem(tr("--"));
        groupComboBox->setCurrentIndex(2);
    }

    // 设置其他用户权限下拉框
    if (otherPermConsistent && firstOtherPerm != -1) {
        if ((firstOtherPerm & QFile::WriteOther)
                && (firstOtherPerm & QFile::ReadOther)) {
            otherComboBox->setCurrentIndex(0);   // Read-write
        } else if (firstOtherPerm & QFile::ReadOther) {
            otherComboBox->setCurrentIndex(1);   // Read only
        } else {
            otherComboBox->addItem(tr("No read/write"));
            otherComboBox->setCurrentIndex(2);   // Please select (no permission)
        }
    } else {
        otherComboBox->addItem(tr("--"));
        otherComboBox->setCurrentIndex(2);
    }
}

void MultiFilePermissionWidget::initConnect()
{
    connect(ownerComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MultiFilePermissionWidget::onOwnerComboBoxChanged);
    connect(groupComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MultiFilePermissionWidget::onGroupComboBoxChanged);
    connect(otherComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MultiFilePermissionWidget::onOtherComboBoxChanged);
}

bool MultiFilePermissionWidget::canChmodByFs(const QUrl &url)
{
    const static QStringList cannotChomodFsTypes { "vfat", "fuseblk", "cifs" };
    
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (info.isNull()) {
        fmCritical() << "Failed to create file info: " << url.toString();
        return false;
    }
    QUrl parentUrl = info->urlOf(UrlInfoType::kParentUrl);
    QStorageInfo storageInfo(parentUrl.toLocalFile());
    const QString &fsType = storageInfo.fileSystemType();
    if (cannotChomodFsTypes.contains(fsType, Qt::CaseInsensitive)) {
        fmWarning() << "Filesystem type does not support chmod: " << fsType;
        return false;
    }

    bool pluginAshResult { false };
    dpfHookSequence->run("dfmplugin_propertydialog", "hook_PermissionView_Ash", url, &pluginAshResult);
    if (pluginAshResult) {
        fmWarning() << "Permission modification is disabled by plugin hook.";
        return false;
    }

    return true;
}

bool MultiFilePermissionWidget::canChmodByFile(const FileInfoPointer &info)
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

void MultiFilePermissionWidget::updateComboBoxViewPalette()
{
    QPalette palette = this->palette();
    QColor bgColor;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        bgColor.setRgb(255, 255, 255);
    else
        bgColor.setRgb(40, 40, 40);
    palette.setColor(QPalette::Window, bgColor);
    setPalette(palette);
}

void MultiFilePermissionWidget::disablePermissionComboBox()
{
    ownerComboBox->setDisabled(true);
    groupComboBox->setDisabled(true);
    otherComboBox->setDisabled(true);
}

void MultiFilePermissionWidget::onOwnerComboBoxChanged(int index)
{
    if (index > 2 || index < 0)
        return;

    emit ownerComboxChanged(index);
}

void MultiFilePermissionWidget::onGroupComboBoxChanged(int index)
{
    if (index > 2 || index < 0)
        return;

    emit groupComboxChanged(index);
}

void MultiFilePermissionWidget::onOtherComboBoxChanged(int index)
{
    if (index > 2 || index < 0)
        return;

    emit otherComboxChanged(index);
}
