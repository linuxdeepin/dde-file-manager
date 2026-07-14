// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multifilepropertiesdialog.h"
#include "events/propertyeventcall.h"
#include "utils/propertydialogmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <denhancedwidget.h>

#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>

#define DDIALOG_TITLEBAR_HEIGHT 50
#define TITLEBAR_AND_HEADER_SPACING 5
#define HEADER_AND_SCROLL_SPACING 15

inline constexpr int kDialogWidth { 380 };
inline constexpr int kArrowExpandSpacing { 10 };
inline constexpr int kHeaderIconHeight { 128 };
inline constexpr int kHeaderNameLabelHeight { 30 };
inline constexpr int kBtnHeight { 40 };
inline constexpr int kVerticalLineHeight { 30 };
inline constexpr int kBottomHeight { 10 };
inline constexpr int kBtnSpacing { 10 };

using namespace dfmplugin_propertydialog;
DFMBASE_USE_NAMESPACE

MultiFilePropertiesDialog::MultiFilePropertiesDialog(const QList<QUrl> &urls,
                                                     QWidget *parent)
    : DDialog(parent)
    , fileUrls(urls)
{
    initUI();
    initConnect();
}

void MultiFilePropertiesDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    QTimer::singleShot(0, this, [this]() {
        processHeight();
    });
}

void MultiFilePropertiesDialog::processHeight()
{
    // 调整滚动区域视图大小
    int scrollWidgetHeight = basicInfoWidget->height()
            + kArrowExpandSpacing
            + permInfoWidget->height()
            + kArrowExpandSpacing;
    scrollWidget->resize(kDialogWidth, scrollWidgetHeight);

    // 调整窗口大小
    int dialogHeight = DDIALOG_TITLEBAR_HEIGHT
            + TITLEBAR_AND_HEADER_SPACING
            + frameHeader->height()
            + HEADER_AND_SCROLL_SPACING
            + scrollWidgetHeight
            + kBtnHeight
            + kBottomHeight;
    QRect newRect = geometry();
    newRect.setHeight(dialogHeight);
    setGeometry(newRect);
}

void MultiFilePropertiesDialog::saveBtnClicked()
{
    if (curFilesPropertyState == orgFilesPropertyState) {
        accept();
        return;
    }

    if (fileUrls.size() < 1) {
        fmCritical() << "Not select files.";
        reject();
        return;
    }

    FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrls.at(0));
    if (!info) {
        fmCritical() << "Failed to create file info.";
        reject();
        return;
    }

    QString strChange("");
    if (curFilesPropertyState.hideState != orgFilesPropertyState.hideState) {
        changeFilesHideState(curFilesPropertyState.hideState, strChange);
    }
    if (curFilesPropertyState.ownerIndex != orgFilesPropertyState.ownerIndex) {
        changeFilesOwnerState(curFilesPropertyState.ownerIndex, strChange);
    }
    if (curFilesPropertyState.groupIndex != orgFilesPropertyState.groupIndex) {
        changeFilesGroupState(curFilesPropertyState.groupIndex, strChange);
    }
    if (curFilesPropertyState.otherIndex != orgFilesPropertyState.otherIndex) {
        changeFilesOtherState(curFilesPropertyState.otherIndex, strChange);
    }

    QString message = tr("Set %1 and %2 file attributes to %3.")
            .arg(info->nameOf(FileInfo::FileNameInfoType::kFileName))
            .arg(fileUrls.size())
            .arg(strChange);
    UniversalUtils::notifyMessage(message, tr("File Manager"));
    accept();
}

void MultiFilePropertiesDialog::handleHideBoxStateChanged(int state)
{
    curFilesPropertyState.hideState = state;

    saveBtn->setEnabled(curFilesPropertyState == orgFilesPropertyState ? false : true);
}

void MultiFilePropertiesDialog::handleOwnerBoxStateChanged(int index)
{
    curFilesPropertyState.ownerIndex = index;

    saveBtn->setEnabled(curFilesPropertyState == orgFilesPropertyState ? false : true);
}

void MultiFilePropertiesDialog::handleGroupBoxStateChanged(int index)
{
    curFilesPropertyState.groupIndex = index;

    saveBtn->setEnabled(curFilesPropertyState == orgFilesPropertyState ? false : true);
}

void MultiFilePropertiesDialog::handleOtherBoxStateChanged(int index)
{
    curFilesPropertyState.otherIndex = index;

    saveBtn->setEnabled(curFilesPropertyState == orgFilesPropertyState ? false : true);
}

void MultiFilePropertiesDialog::initUI()
{
    setFixedWidth(kDialogWidth);
    setAttribute(Qt::WA_DeleteOnClose, true);

    QLabel *headerIcon = new QLabel(this);
    QIcon icon;
    icon.addFile(QString { ":/images/images/multiple_files.png" });
    icon.addFile(QString { ":/images/images/multiple_files@2x.png" });
    headerIcon->setPixmap(icon.pixmap(kHeaderIconHeight, kHeaderIconHeight));
    headerIcon->setFixedHeight(kHeaderIconHeight);

    QLabel *headerFileName = new QLabel(tr("Multiple Files"), this);
    DFontSizeManager::instance()->bind(headerFileName, DFontSizeManager::T9,
                                       QFont::Medium);
    headerFileName->setFixedHeight(kHeaderNameLabelHeight);

    QVBoxLayout *vlayHeader = new QVBoxLayout;
    vlayHeader->setContentsMargins(0, 0,  0, 0);
    vlayHeader->addWidget(headerIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayHeader->addWidget(headerFileName, 0, Qt::AlignHCenter);
    frameHeader = new QFrame(this);
    frameHeader->setLayout(vlayHeader);
    addContent(frameHeader);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Window, Qt::NoBrush);
    scrollArea->viewport()->setPalette(palette);

    scrollWidget = new QFrame(this);
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 10);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);

    basicInfoWidget = new MultiFileBasicInfoWidget(fileUrls, scrollWidget);
    scrollWidgetLayout->addWidget(basicInfoWidget, 0, Qt::AlignTop);
    basicInfoWidget->getOrgHideBoxState(orgFilesPropertyState);

    permInfoWidget = new MultiFilePermissionWidget(fileUrls, scrollWidget);
    scrollWidgetLayout->addWidget(permInfoWidget, 0, Qt::AlignTop);
    permInfoWidget->getOrgPermissonBoxState(orgFilesPropertyState);

    curFilesPropertyState = orgFilesPropertyState;

    scrollWidgetLayout->addStretch();

    scrollWidget->setLayout(scrollWidgetLayout);
    scrollArea->setWidget(scrollWidget);

    cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setFixedHeight(kBtnHeight);
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);

    DVerticalLine *line = new DVerticalLine;
    line->setFixedHeight(kVerticalLineHeight);

    saveBtn = new DSuggestButton(tr("Save"), this);
    saveBtn->setFixedHeight(kBtnHeight);
    saveBtn->setAttribute(Qt::WA_NoMousePropagation);
    saveBtn->setEnabled(false);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(kBtnSpacing);
    btnLayout->addSpacing(kBtnSpacing);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(line);
    btnLayout->addWidget(saveBtn);
    btnLayout->addSpacing(kBtnSpacing);

    QVBoxLayout *vlayContent = new QVBoxLayout;
    vlayContent->setContentsMargins(0, 0, 0, kBottomHeight);
    vlayContent->setSpacing(0);
    vlayContent->addWidget(scrollArea);
    vlayContent->addLayout(btnLayout);
    QVBoxLayout *dialogLay = qobject_cast<QVBoxLayout *>(layout());
    if (dialogLay) {
        dialogLay->addLayout(vlayContent, 1);
    }

    // 判断是否筛选权限视图
    if (!fileUrls.isEmpty()) {
        PropertyFilterType controlFilter = PropertyDialogManager::instance().basicFiledFiltes(fileUrls.at(0));
        if ((controlFilter & PropertyFilterType::kPermission) > 0) {
            permInfoWidget->disablePermissionComboBox();
        }
    }
}

void MultiFilePropertiesDialog::initConnect()
{
    if (basicInfoWidget) {
        connectHeightChange(basicInfoWidget);
        connect(basicInfoWidget, &MultiFileBasicInfoWidget::hideBoxStateChanged,
                this, &MultiFilePropertiesDialog::handleHideBoxStateChanged);
    }
    if (permInfoWidget) {
        connectHeightChange(permInfoWidget);
        connect(permInfoWidget, &MultiFilePermissionWidget::ownerComboxChanged,
                this, &MultiFilePropertiesDialog::handleOwnerBoxStateChanged);
        connect(permInfoWidget, &MultiFilePermissionWidget::groupComboxChanged,
                this, &MultiFilePropertiesDialog::handleGroupBoxStateChanged);
        connect(permInfoWidget, &MultiFilePermissionWidget::otherComboxChanged,
                this, &MultiFilePropertiesDialog::handleOtherBoxStateChanged);
    }
    connect(cancelBtn, &QPushButton::clicked,
            this, &MultiFilePropertiesDialog::reject);
    connect(saveBtn, &DSuggestButton::clicked,
            this, &MultiFilePropertiesDialog::saveBtnClicked);
}

void MultiFilePropertiesDialog::connectHeightChange(DArrowLineDrawer *w)
{
    DEnhancedWidget *hancedWidget = new DEnhancedWidget(w,w);
    connect(hancedWidget, &DEnhancedWidget::heightChanged,
            this, &MultiFilePropertiesDialog::processHeight);
}

bool MultiFilePropertiesDialog::changeFilesHideState(int state, QString &strChange)
{
    if (fileUrls.size() < 1) {
        fmCritical() << "Not select files.";
        return false;
    }

    FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrls.at(0));
    if (!info) {
        fmCritical() << "Failed to create file info.";
        return false;
    }
    const QUrl parentUrl = info->urlOf(UrlInfoType::kParentUrl);
    if (!parentUrl.isValid()) {
        fmCritical() << "Parent url is not valid.";
        return false;
    }

    quint64 winIDs = QApplication::activeWindow()->winId();
    if (state == Qt::Checked) {
        PropertyEventCall::sendFilesHideOrVisible(winIDs, parentUrl, fileUrls, true);
        if (strChange.isEmpty()) {
            strChange = tr("Hidden files");
        } else {
            strChange += "," + tr("Hidden files");
        }
    } else if (state == Qt::Unchecked) {
        PropertyEventCall::sendFilesHideOrVisible(winIDs, parentUrl, fileUrls, false);
        if (strChange.isEmpty()) {
            strChange = tr("Unhide files");
        } else {
            strChange += "," + tr("Unhide files");
        }
    }

    return true;
}

bool MultiFilePropertiesDialog::changeFilesOwnerState(int index, QString &strChange)
{
    for (int i = 0; i < fileUrls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrls.at(i));
        if (info.isNull()) {
            fmCritical() << "Failed to get file info: " << fileUrls.at(i).path();
            return false;
        }

        QFile::Permissions permissions = info->permissions();

        // 去掉拥有者权限的干扰权限(ReadUser WriteUser ExeUser)
        permissions &= ~(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);

        if (index == 0) {   // read-write
            // 添加读写权限
            permissions |= (QFile::ReadOwner | QFile::WriteOwner);
        } else if (index == 1) {    // read only
            // 移除写权限，添加读权限
            permissions &= ~QFile::WriteOwner;
            permissions |= QFile::ReadOwner;
        }
        PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(),
                                                    fileUrls.at(i), permissions);
    }
    QString strTmp;
    if (index == 0)
        strTmp = tr("Owner>Read-write");
    else
        strTmp = tr("Owner>Read only");
    if (strChange.isEmpty()) {
        strChange = strTmp;
    } else {
        strChange += "," + strTmp;
    }

    return true;
}

bool MultiFilePropertiesDialog::changeFilesGroupState(int index, QString &strChange)
{
    for (int i = 0; i < fileUrls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrls.at(i));
        if (info.isNull()) {
            fmCritical() << "Failed to get file info: " << fileUrls.at(i).path();
            return false;
        }

        QFile::Permissions permissions = info->permissions();
        if (index == 0) {   // read-write
            // 添加读写权限
            permissions |= (QFile::ReadGroup | QFile::WriteGroup);
        } else if (index == 1) {    // read only
            // 移除写权限，添加读权限
            permissions &= ~QFile::WriteGroup;
            permissions |= QFile::ReadGroup;
        }
        PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(),
                                                    fileUrls.at(i), permissions);
    }

    QString strTmp;
    if (index == 0)
        strTmp = tr("Group>Read-write");
    else
        strTmp = tr("Group>Read only");
    if (strChange.isEmpty()) {
        strChange = strTmp;
    } else {
        strChange += "," + strTmp;
    }

    return true;
}

bool MultiFilePropertiesDialog::changeFilesOtherState(int index, QString &strChange)
{
    for (int i = 0; i < fileUrls.size(); ++i) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrls.at(i));
        if (info.isNull()) {
            fmCritical() << "Failed to get file info: " << fileUrls.at(i).path();
            return false;
        }

        QFile::Permissions permissions = info->permissions();
        if (index == 0) {   // read-write
            // 添加读写权限
            permissions |= (QFile::ReadOther | QFile::WriteOther);
        } else if (index == 1) {    // read only
            // 移除写权限，添加读权限
            permissions &= ~QFile::WriteOther;
            permissions |= QFile::ReadOther;
        }
        PropertyEventCall::sendSetPermissionManager(qApp->activeWindow()->winId(),
                                                    fileUrls.at(i), permissions);
    }

    QString strTmp;
    if (index == 0)
        strTmp = tr("Others>Read-write");
    else
        strTmp = tr("Others>Read only");
    if (strChange.isEmpty()) {
        strChange = strTmp;
    } else {
        strChange += "," + strTmp;
    }

    return true;
}
