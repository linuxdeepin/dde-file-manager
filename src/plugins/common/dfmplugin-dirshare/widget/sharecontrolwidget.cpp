/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "sharecontrolwidget.h"

#include "services/common/usershare/usershareservice.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/decorator/decoratorfile.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include <QIcon>
#include <QStandardPaths>

#include <unistd.h>

DPDIRSHARE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

namespace ConstDef {
static const int kWidgetFixedWidth { 160 };
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
    setFixedWidth(100);
    QFont font = this->font();
    font.setWeight(QFont::Bold - 8);
    font.setPixelSize(13);
    setFont(font);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

ShareControlWidget::ShareControlWidget(const QUrl &url, QWidget *parent)
    : DArrowLineDrawer(parent), url(url)
{
    setupUi();
    init();
    initConnection();
}

void ShareControlWidget::setupUi()
{
    setTitle(tr("Sharing"));
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    QFrame *frame = new QFrame(this);
    setContent(frame);
    QFormLayout *mainLay = new QFormLayout(this);
    mainLay->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    mainLay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    mainLay->setFormAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mainLay->setContentsMargins(10, 10, 10, 10);
    frame->setLayout(mainLay);

    shareSwitcher = new QCheckBox(tr("Share this folder"), this);
    QWidget *switcherContainer = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(this);
    switcherContainer->setLayout(lay);
    lay->addWidget(shareSwitcher);
    lay->setAlignment(Qt::AlignCenter);
    lay->setContentsMargins(0, 0, 0, 0);
    mainLay->addRow(switcherContainer);

    shareNameEditor = new QLineEdit(this);
    shareNameEditor->setFixedWidth(ConstDef::kWidgetFixedWidth);
    mainLay->addRow(new SectionKeyLabel(tr("Share name:"), this), shareNameEditor);
    sharePermissionSelector = new QComboBox(this);
    sharePermissionSelector->setFixedWidth(ConstDef::kWidgetFixedWidth);
    mainLay->addRow(new SectionKeyLabel(tr("Permission:"), this), sharePermissionSelector);
    shareAnonymousSelector = new QComboBox(this);
    shareAnonymousSelector->setFixedWidth(ConstDef::kWidgetFixedWidth);
    mainLay->addRow(new SectionKeyLabel(tr("Anonymous:"), this), shareAnonymousSelector);

    QValidator *validator = new QRegularExpressionValidator(QRegularExpression(ConstDef::kShareNameRegx), this);
    shareNameEditor->setValidator(validator);
    QStringList permissions { tr("Read and write"), tr("Read only") };
    sharePermissionSelector->addItems(permissions);
    QStringList anonymousSelections { tr("Not allow"), tr("Allow") };
    shareAnonymousSelector->addItems(anonymousSelections);
}

void ShareControlWidget::init()
{
    info = InfoFactory::create<AbstractFileInfo>(url);
    if (!info) {
        qWarning() << "cannot create file info of " << url;
        return;
    }

    QString filePath = url.path();
    DSC_USE_NAMESPACE auto shareName = UserShareService::service()->getShareNameByPath(filePath);
    if (shareName.isEmpty())
        shareName = info->fileDisplayName();
    shareNameEditor->setText(shareName);

    bool isShared = UserShareService::service()->isSharedPath(filePath);
    shareSwitcher->setChecked(isShared);
    if (isShared) {
        auto shareInfo = UserShareService::service()->getInfoByPath(filePath);
        sharePermissionSelector->setCurrentIndex(shareInfo.getWritable() ? 0 : 1);
        shareAnonymousSelector->setCurrentIndex(shareInfo.getAnonymous() ? 1 : 0);
    }

    sharePermissionSelector->setEnabled(isShared);
    shareAnonymousSelector->setEnabled(isShared);
}

void ShareControlWidget::initConnection()
{
    connect(shareSwitcher, &QCheckBox::clicked, this, [this](bool checked) {
        sharePermissionSelector->setEnabled(checked);
        shareAnonymousSelector->setEnabled(checked);
        if (checked)
            this->shareFolder();
        else
            this->unshareFolder();
    });

    connect(shareAnonymousSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShareControlWidget::updateShare);
    connect(sharePermissionSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShareControlWidget::updateShare);
    connect(shareNameEditor, &QLineEdit::editingFinished, this, &ShareControlWidget::updateShare);

    connect(UserShareService::service(), &UserShareService::shareAdded, this, &ShareControlWidget::updateWidgetStatus);
    connect(UserShareService::service(), &UserShareService::shareRemoved, this, &ShareControlWidget::updateWidgetStatus);
    connect(UserShareService::service(), &UserShareService::shareRemoveFailed, this, &ShareControlWidget::updateWidgetStatus);
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

    if (UserShareService::service()->isSharedPath(url.path())) {
        auto sharedName = UserShareService::service()->getShareNameByPath(url.path());
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
    shareFolder();
    shareSwitcher->setEnabled(true);
}

void ShareControlWidget::shareFolder()
{
    if (!shareSwitcher->isChecked())
        return;

    if (!validateShareName()) {
        shareSwitcher->setChecked(false);
        sharePermissionSelector->setEnabled(false);
        shareAnonymousSelector->setEnabled(false);
        return;
    }

    shareSwitcher->setEnabled(false);

    bool writable = sharePermissionSelector->currentIndex() == 0;
    bool anonymous = shareAnonymousSelector->currentIndex() == 1;
    if (anonymous) {   // set the directory's access permission to 777
        // 1. set the permission of shared folder to 777;
        DecoratorFile file(url);
        if (file.exists() && writable) {
            using namespace DFMIO;
            bool ret = file.setPermissions(file.permissions() | DFile::Permission::kWriteGroup | DFile::Permission::kExeGroup
                                           | DFile::Permission::kWriteOther | DFile::Permission::kExeOther);
            if (!ret)
                qWarning() << "set permission of " << url << "failed.";
        }

        // 2. set the mode 'other' of  /home/$USER to r-x when enable anonymous access,
        // otherwise the anonymous user cannot mount the share successfully.
        // and never change the mode of /root
        if (getuid() != 0) {
            QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            DecoratorFile home(homePath);
            if (home.exists()) {
                using namespace DFMIO;
                bool ret = home.setPermissions(home.permissions() | DFile::Permission::kReadOther | DFile::Permission::kExeOther);
                if (!ret)
                    qWarning() << "set permission for user home failed: " << homePath;
            }
        }
    }
    ShareInfo info(shareNameEditor->text().trimmed(), url.path(), "", writable, anonymous);
    UserShareService::service()->addShare(info);
}

void ShareControlWidget::unshareFolder()
{
    UserShareService::service()->removeShare(url.path());
}

void ShareControlWidget::updateWidgetStatus(const QString &filePath)
{
    shareSwitcher->setEnabled(true);
    if (filePath != url.path())
        return;

    auto shareInfo = UserShareService::service()->getInfoByPath(filePath);
    if (shareInfo.isValid()) {
        shareSwitcher->setChecked(true);
        shareNameEditor->setText(shareInfo.getShareName());
        if (shareInfo.getWritable())
            sharePermissionSelector->setCurrentIndex(0);
        else
            sharePermissionSelector->setCurrentIndex(1);

        if (shareInfo.getAnonymous())
            shareAnonymousSelector->setCurrentIndex(1);
        else
            shareAnonymousSelector->setCurrentIndex(0);

        uint shareUid = UserShareService::service()->getUidByShareName(shareInfo.getShareName());
        if ((shareUid != info->ownerId() || shareUid != getuid()) && getuid() != 0)
            this->setEnabled(false);

        sharePermissionSelector->setEnabled(true);
        shareAnonymousSelector->setEnabled(true);
    } else {
        shareSwitcher->setChecked(false);
        sharePermissionSelector->setEnabled(false);
        shareAnonymousSelector->setEnabled(false);
    }
}

#include "sharecontrolwidget.moc"
