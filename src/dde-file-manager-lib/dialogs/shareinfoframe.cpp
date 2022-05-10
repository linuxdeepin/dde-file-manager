/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "shareinfoframe.h"
#include "propertydialog.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileservices.h"
#include "dialogmanager.h"

#include <QStandardPaths>
#include <QFormLayout>
#include <QProcess>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QNetworkInterface>
#include <QPainter>
#include <QTextBrowser>
#include <QApplication>
#include <QClipboard>
#include <DPushButton>

#include <unistd.h>

ShareInfoFrame::ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent) :
    QFrame(parent),
    m_fileinfo(info)
{
    initUI();
    updateShareInfo(m_fileinfo->absoluteFilePath());
    initConnect();
}

void ShareInfoFrame::initUI()
{
    int labelWidth = 100;
    int fieldWidth = 160 + 38;
    QFont fontNormal = this->font();
    fontNormal.setFamily("SourceHanSansSC");
    fontNormal.setPixelSize(12);
    fontNormal.setWeight(QFont::Normal);
    fontNormal.setStyle(QFont::StyleNormal);

    QFont fontMedium = this->font();
    fontMedium.setFamily("SourceHanSansSC");
    fontMedium.setPixelSize(12);
    fontMedium.setWeight(QFont::Medium);
    fontMedium.setStyle(QFont::StyleNormal);

    m_isSharePasswordSet = userShareManager->isSharePasswordSet(UserShareManager::getCurrentUserName());

    //控件构造
    m_shareCheckBox = new QCheckBox(this);
    m_shareCheckBox->setFixedWidth(fieldWidth);
    m_shareCheckBox->setText(tr("Share this folder"));
    connect(m_shareCheckBox,&QCheckBox::clicked,this,&ShareInfoFrame::showShareInfo);

    QWidget *centerAlignContainer = new QWidget();
    QHBoxLayout *centerAlignLayout = new QHBoxLayout(centerAlignContainer);
    centerAlignLayout->addWidget(m_shareCheckBox);
    centerAlignLayout->setAlignment(Qt::AlignLeft);
    centerAlignLayout->setContentsMargins(0, 0, 0, 0);
    centerAlignContainer->setLayout(centerAlignLayout);

    SectionKeyLabel *shareNameLabel = new SectionKeyLabel(tr("Share name"));
    shareNameLabel->setFixedWidth(labelWidth);
    m_shareNamelineEdit = new QLineEdit(this);
    m_shareNamelineEdit->setObjectName("ShareNameEdit");
    m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
    m_shareNamelineEdit->setFixedWidth(fieldWidth);
    // sp3需求 共享文件名设置限制
    // 设置只能输入大小写字母、数字和部分符号的正则表达式
    QRegExp regx("^[^\\s+\\[\\]\"'/\\\\:|<>+=;,?*\r\n\t]*$");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);
    m_shareNamelineEdit->setValidator(validator);

    SectionKeyLabel *permissionLabel = new SectionKeyLabel(tr("Permission"));
    permissionLabel->setFixedWidth(labelWidth);
    m_permissoComBox = new QComboBox(this);
    m_permissoComBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_permissoComBox->setFixedWidth(fieldWidth);
    QStringList permissions;
    permissions << tr("Read and write") << tr("Read only");
    m_permissoComBox->addItems(permissions);

    SectionKeyLabel *anonymityLabel = new SectionKeyLabel(tr("Anonymous"));
    anonymityLabel->setFixedWidth(labelWidth);
    m_anonymityCombox = new QComboBox(this);
    m_anonymityCombox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_anonymityCombox->setFixedWidth(fieldWidth);
    QStringList anonymityChoices;
    anonymityChoices << tr("Not allow") << tr("Allow");
    m_anonymityCombox->addItems(anonymityChoices);

    SectionKeyLabel *networkAddLabel = new SectionKeyLabel(tr("Network path"));
    networkAddLabel->setFixedWidth(labelWidth + 5);
    networkAddLabel->setAttribute(Qt::WA_TranslucentBackground, true);

    QPalette pe;
    pe.setColor(QPalette::Text, QColor("#526A7F"));
    m_netScheme = new QLabel(QString("%1://").arg(SMB_SCHEME));
    m_netScheme->setAttribute(Qt::WA_TranslucentBackground, true);
    m_netScheme->setStyleSheet("QLineEdit{background-color:rgba(0,0,0,0)}");
    m_netScheme->setPalette(pe);

    m_networkAddrLabel = new QLineEdit(m_selfIp);
    m_networkAddrLabel->setReadOnly(true);
    m_networkAddrLabel->setText("");
    m_networkAddrLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_networkAddrLabel->setFont(fontNormal);
    m_networkAddrLabel->setPalette(pe);
    m_networkAddrLabel->setObjectName("NetworkAddress");
    m_networkAddrLabel->setStyleSheet("QLineEdit{background-color:rgba(0,0,0,0)}");

    if(m_updateIp == nullptr){
        m_updateIp = new QTimer();
        m_updateIp->setInterval(0);
        connect(m_updateIp,&QTimer::timeout, this, [this](){
            QList<QHostAddress> list = QNetworkInterface::allAddresses();

            foreach (QHostAddress address, list) {
                if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                    m_selfIp = address.toString();
                }
            }
            if(m_networkAddrLabel->text() != m_selfIp){
                if(m_selfIp == "127.0.0.1")
                    m_networkAddrLabel->setText("");
                else
                    m_networkAddrLabel->setText(m_selfIp);
            }

            m_updateIp->setInterval(2000);
        });
        m_updateIp->start();
    }

    SectionKeyLabel *userNameLabel = new SectionKeyLabel(tr("Username"));
    userNameLabel->setFixedWidth(labelWidth + 5);
    userNameLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_userNamelineEdit = new QLineEdit();
    m_userNamelineEdit->setAttribute(Qt::WA_TranslucentBackground, true);
    m_userNamelineEdit->setFont(fontNormal);
    m_userNamelineEdit->setPalette(pe);
    m_userNamelineEdit->setStyleSheet("QLineEdit{background-color:rgba(0,0,0,0)}");
    m_userNamelineEdit->setObjectName("UserNameEdit");
    m_userNamelineEdit->setText(UserShareManager::getCurrentUserName());
    m_userNamelineEdit->setReadOnly(true);

    SectionKeyLabel *sharePasswordLabel = new SectionKeyLabel(tr("Share password"));
    sharePasswordLabel->setFixedWidth(labelWidth + 5);
    sharePasswordLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_sharePasswordlineEdit = new QLineEdit(this);
    m_sharePasswordlineEdit->setAttribute(Qt::WA_TranslucentBackground, true);
    pe.setColor(QPalette::Text, QColor("#000000"));
    m_sharePasswordlineEdit->setPalette(pe);
    m_sharePasswordlineEdit->setStyleSheet("QLineEdit{background-color:rgba(0,0,0,0)}");
    QFont font = this->font();
    int defaultFontSize = font.pointSize();
    font.setPointSize(m_isSharePasswordSet ? 7 : defaultFontSize);
    m_sharePasswordlineEdit->setFont(font);
    m_sharePasswordlineEdit->setEchoMode(m_isSharePasswordSet ? QLineEdit::Password : QLineEdit::Normal);
    m_sharePasswordlineEdit->setObjectName("SharePasswordEdit");
    m_sharePasswordlineEdit->setAlignment(Qt::AlignJustify | Qt::AlignLeft);
    m_sharePasswordlineEdit->setText(m_isSharePasswordSet ? "------" : tr("None"));
    m_sharePasswordlineEdit->setDisabled(true);
    m_sharePasswordlineEdit->setCursorPosition(0);

    //控件布局
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(6);
    QFormLayout *firstLayout = new QFormLayout();
    firstLayout->setSpacing(10);
    DFMGlobal::setToolTip(shareNameLabel);
    DFMGlobal::setToolTip(permissionLabel);
    DFMGlobal::setToolTip(anonymityLabel);

    firstLayout->addRow(centerAlignContainer);
    firstLayout->addRow(shareNameLabel, m_shareNamelineEdit);
    firstLayout->addRow(permissionLabel, m_permissoComBox);

    firstLayout->addRow(anonymityLabel, m_anonymityCombox);
    firstLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    firstLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    firstLayout->setFormAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mainLayout->setContentsMargins(12, 10, 12, 10);

    QGridLayout *secondLayout = new QGridLayout();
    secondLayout->setSpacing(0);
    secondLayout->setContentsMargins(0, 0, 0, 0);
    if (m_sharePropertyBkgWidget == nullptr) {
        m_sharePropertyBkgWidget = new QWidget();
        m_sharePropertyBkgWidget->installEventFilter(this);
        m_sharePropertyBkgWidget->setAttribute(Qt::WA_StyledBackground, true);
    }

    QFormLayout *netLayout = new QFormLayout();
    netLayout->setSpacing(0);
    //Layout: network
    QHBoxLayout *networkAddrLayout = new QHBoxLayout;
    networkAddrLayout->setContentsMargins(0, 0, 0, 0);
    DPushButton *copyNetAddr = new DPushButton(QIcon(":icons/images/icons/property_bt_copy.png"), "");
    copyNetAddr->setToolTip(tr("Copy"));
    QObject::connect(copyNetAddr, &DPushButton::clicked, [=]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_netScheme->text() + m_networkAddrLabel->text());
    });
    QHBoxLayout *schemeLayout = new QHBoxLayout;
    schemeLayout->addWidget(m_netScheme);
    schemeLayout->addWidget(m_networkAddrLabel);
    networkAddrLayout->addItem(schemeLayout);
    networkAddrLayout->addWidget(copyNetAddr);
    networkAddrLayout->setStretch(0, 1);
    netLayout->addRow(networkAddLabel, networkAddrLayout);
    //Layout: user name
    QHBoxLayout *userNameLayout = new QHBoxLayout();
    userNameLayout->setContentsMargins(0, 0, 0, 0);
    DPushButton *copyUserName = new DPushButton(QIcon(":icons/images/icons/property_bt_copy.png"), "");
    copyUserName->setToolTip(tr("Copy"));
    QObject::connect(copyUserName, &DPushButton::clicked, [=]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_userNamelineEdit->text());
    });
    userNameLayout->addWidget(m_userNamelineEdit);
    userNameLayout->addWidget(copyUserName);
    userNameLayout->setStretch(0, 1);
    netLayout->addRow(userNameLabel, userNameLayout);

    netLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    netLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    netLayout->setFormAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    netLayout->setContentsMargins(10, 10, 10, 10);
    secondLayout->addLayout(netLayout, 0, 0, 1, 1);

    QHBoxLayout *netLayout2 = new QHBoxLayout();
    netLayout2->setContentsMargins(0, 0, 0, 0);
    //Layout: password
    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->setSpacing(6);
    passwordLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    QPushButton *passwordOperation = new QPushButton(m_isSharePasswordSet ? tr("Change password") : tr("Set password"));
    passwordOperation->setAttribute(Qt::WA_TranslucentBackground, true);
    passwordOperation->setFont(fontMedium);
    passwordOperation->setStyleSheet("QPushButton{color:#0082FA;background-color:rgba(0,0,0,0)}");
    connect(passwordOperation, &QPushButton::clicked, this, &ShareInfoFrame::setOrModifySharePassword);

    passwordLayout->addWidget(m_sharePasswordlineEdit);
    passwordLayout->addWidget(passwordOperation);

    passwordLayout->setStretch(0, 1);
    netLayout2->addWidget(sharePasswordLabel);
    netLayout2->addLayout(passwordLayout);
    netLayout2->setContentsMargins(10, 10, 10, 10);

    secondLayout->addLayout(netLayout2, 2, 0, 1, 1);
    m_sharePropertyBkgWidget->setLayout(secondLayout);

    mainLayout->addLayout(firstLayout, 0, 0);
    if(!splitLineGray)
        splitLineGray = new QPushButton(this);

    splitLineGray->installEventFilter(this);
    splitLineGray->setContentsMargins(0, 0, 0, 0);
    splitLineGray->setMaximumHeight(2);
    splitLineGray->setFocusPolicy(Qt::NoFocus);
    mainLayout->addWidget(splitLineGray, 1, 0);
    mainLayout->addWidget(m_sharePropertyBkgWidget, 2, 0);
    m_shareNotes = new QTextBrowser(this);
    m_shareNotes->setFont(fontNormal);
    m_shareNotes->setContentsMargins(0, 0, 0, 0);
    pe.setColor(QPalette::Text, QColor("#526A7F"));
    m_shareNotes->setPalette(pe);

    static QString notice = tr("This password will be applied to all shared folders, and users without the password can only access shared folders that allow anonymous access. ");
    m_shareNotes->setPlainText(notice);
    m_shareNotes->setFixedHeight(60);
    m_shareNotes->setReadOnly(true);
    m_shareNotes->setFrameStyle(QFrame::NoFrame);
    connect(m_shareNotes, &QTextBrowser::copyAvailable, this, [=](bool yesCopy) {
        if (yesCopy) {
            QTextCursor textCursor = m_shareNotes->textCursor();
            if (textCursor.hasSelection()) {
                textCursor.clearSelection();
                m_shareNotes->setTextCursor(textCursor);   // 去除选中
            }
        }
    });
    mainLayout->addWidget(m_shareNotes, 3, 0);
    setLayout(mainLayout);

    //当前文件夹已被共享
    if (userShareManager->isShareFile(m_fileinfo->filePath())) {
        auto creatorShareUid = userShareManager->getCreatorUidByShareName(m_fileinfo->fileSharedName().toLower());
        //文件共享创建者不是当前process的打开者或者不是文件所有者 排除root用户
        if ((creatorShareUid != getuid() || creatorShareUid != m_fileinfo->ownerId())
                && getuid() != 0) {
            this->setEnabled(false);
        }
    }

    //判断文件属主与进程属主是否相同，排除进程属主为根用户情况
    if (m_fileinfo->ownerId() != getuid() && getuid() != 0) {
        this->setEnabled(false);
//        m_shareCheckBox->setEnabled(false);
//        m_shareNamelineEdit->setEnabled(false);
//        m_permissoComBox->setEnabled(false);
//        m_anonymityCombox->setEnabled(false);
    }
}

void ShareInfoFrame::initConnect()
{
    connect(m_shareCheckBox, &QCheckBox::clicked, this, &ShareInfoFrame::handleCheckBoxChanged);
//    connect(m_shareNamelineEdit, &QLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
    connect(m_shareNamelineEdit, &QLineEdit::editingFinished, this, &ShareInfoFrame::handleShareNameFinished);
    //connect(m_shareNamelineEdit, &QLineEdit::returnPressed, this, [ = ]() {qDebug() << "回车按下";}); //不知为何没有发送returnPressed信号
    connect(m_permissoComBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePermissionComboxChanged(int)));
    connect(m_anonymityCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleAnonymityComboxChanged(int)));
    //connect(m_jobTimer, &QTimer::timeout, this, &ShareInfoFrame::doShareInfoSetting);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeleted, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeletedFailed, this, &ShareInfoFrame::updateShareInfo);
}

void ShareInfoFrame::handleCheckBoxChanged(const bool &checked)
{
    //为了避免在高速点击时引发ui错乱，只有在share流程完全结束后才允许用户再次修改共享状态
    m_shareCheckBox->setEnabled(false);
    bool ret = doShareInfoSetting();
    if (ret) {
        if (checked) {
//            emit folderShared(m_fileinfo->absoluteFilePath());
            if(!m_isSharePasswordSet){//if not set share password, then show the settings dialog
                setOrModifySharePassword();
            }
            activateWidgets();
        }
    } else {
        m_shareCheckBox->setChecked(false);
        m_shareCheckBox->setEnabled(true);
        disactivateWidgets();
    }
}

void ShareInfoFrame::handleShareNameFinished()
{
    // 修复bug-54080
    // 当失去焦点时，判断文件名是否符合规则
    if (!m_shareNamelineEdit->hasFocus())
        doShareInfoSetting();
    else    // 如果焦点存在，将焦点设置到下一个控件
        m_permissoComBox->setFocus();
    //handShareInfoChanged();
}

void ShareInfoFrame::handleShareNameChanged(const QString &str)
{
    // fix bug 69970 与文件名规则保持一致
    // 采用validator过滤，屏蔽信号
    QString dstText = DFMGlobal::preprocessingFileName(str);
    if (str != dstText) {
        QSignalBlocker blocker(m_shareNamelineEdit);

        int currPos = m_shareNamelineEdit->cursorPosition();
        m_shareNamelineEdit->setText(dstText);
        currPos += dstText.length() - str.length();
        m_shareNamelineEdit->setText(dstText);
        m_shareNamelineEdit->setCursorPosition(currPos);
    }
}

void ShareInfoFrame::handlePermissionComboxChanged(const int &index)
{
    Q_UNUSED(index);
    doShareInfoSetting();
}

void ShareInfoFrame::handleAnonymityComboxChanged(const int &index)
{
    Q_UNUSED(index);
    doShareInfoSetting();
}

void ShareInfoFrame::handShareInfoChanged()
{
    //m_jobTimer->start();
}

bool ShareInfoFrame::doShareInfoSetting()
{
    if (m_shareCheckBox->isChecked()) { //判断是否为添加共享
        if (!checkShareName()) { //检查共享名
            return false;
        }
    } else {
        emit unfolderShared();
//        hide();
        return DFileService::instance()->unShareFolder(this, m_fileinfo->fileUrl());
    }
    // fix bug#51124 只读共享文件夹，修改“匿名访问”，此文件夹权限改变
    // 用户权限保持不变，修改组、其他权限为可读写
    if (m_permissoComBox->currentIndex() == 0 && m_anonymityCombox->currentIndex() != 0) {
        DUrl localUrl = DUrl::fromLocalFile(m_fileinfo->fileUrl().toLocalFile());
        fileService->setPermissions(nullptr, localUrl, m_fileinfo->permissions()
                                    | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther
                                    | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup);
    }

    bool ret = DFileService::instance()->shareFolder(this, m_fileinfo->fileUrl(), m_shareNamelineEdit->text(),
                                                     m_permissoComBox->currentIndex() == 0,
                                                     m_anonymityCombox->currentIndex() != 0);

    //修改用户目录其他权限为可执行
    //该权限修改逻辑只针对普通用户共享自己的文件时有效
    //root用户共享的行为不主动修改目录权限，既共享时不修改/root的其他执行权限和普通用户主目录的其他执行权限
    if (ret && m_anonymityCombox->currentIndex() != 0 && getuid() != 0) {
        DUrl userUrl = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if (m_fileinfo->path().startsWith(userUrl.path())) {
            DAbstractFileInfoPointer userFileInfo = fileService->createFileInfo(this, userUrl);
            if (userFileInfo && userFileInfo->exists()
                    && (userFileInfo->permissions() & QFileDevice::ExeOther) != QFileDevice::ExeOther)
                fileService->setPermissions(this, userUrl, userFileInfo->permissions() | QFileDevice::ExeOther);
        }
    }

    return ret;
}

void ShareInfoFrame::updateShareInfo(const QString &filePath)
{
    //为了避免在高速点击时引发ui错乱，只有在share流程完全结束后才允许用户再次修改共享状态
    m_shareCheckBox->setEnabled(true);
    if (filePath != m_fileinfo->absoluteFilePath())
        return;

    if (!m_fileinfo->fileSharedName().isEmpty()) {
        m_shareCheckBox->setChecked(true);
        showShareInfo(true);
        activateWidgets();
        m_shareNamelineEdit->setText(m_fileinfo->fileSharedName());
        if (m_fileinfo->isWritableShared()) {
            m_permissoComBox->setCurrentIndex(0);
        } else {
            m_permissoComBox->setCurrentIndex(1);
        }
        if (m_fileinfo->isAllowGuestShared()) {
            m_anonymityCombox->setCurrentIndex(1);
        } else {
            m_anonymityCombox->setCurrentIndex(0);
        }
    } else {
        m_shareCheckBox->setChecked(false);
        showShareInfo(false);
        m_permissoComBox->setCurrentIndex(0);
        m_anonymityCombox->setCurrentIndex(0);

        const QString share_name = m_fileinfo->fileDisplayName().remove(QRegExp("[%<>*?|\\\\+=;:\",]"));

        m_shareNamelineEdit->setText(share_name);

        disactivateWidgets();
    }
}

void ShareInfoFrame::activateWidgets()
{
    m_permissoComBox->setEnabled(true);
    m_anonymityCombox->setEnabled(true);
}

void ShareInfoFrame::disactivateWidgets()
{
    m_permissoComBox->setEnabled(false);
    m_anonymityCombox->setEnabled(false);
}

void ShareInfoFrame::setOrModifySharePassword()
{
    dialogManager->showSharePasswordSettingDialog(this);
}

void ShareInfoFrame::showShareInfo(bool value)
{
    if(value){
        m_sharePropertyBkgWidget->show();
        m_shareNotes->show();
        splitLineGray->show();
    }
    else{
        m_sharePropertyBkgWidget->hide();
        m_shareNotes->hide();
        splitLineGray->hide();
    }
}

void ShareInfoFrame::setFileinfo(const DAbstractFileInfoPointer &fileinfo)
{
    m_fileinfo = fileinfo;
    updateShareInfo(m_fileinfo->absoluteFilePath());
}

bool ShareInfoFrame::checkShareName() //返回值表示是否继续
{
    // 共享名不能为空
    const QString &name = m_shareNamelineEdit->text();
    if (name.isEmpty() || name == "") {
        return false;
    }
    // 修复BUG-44972
    // 当共享文件名为“..”或“.”时，弹出提示框
    if (name == ".." || name == ".") {
        QString strMsg = tr("The share name must not be two dots (..) or one dot (.)");
        dialogManager->showMessageDialog(DialogManager::msgWarn, strMsg);
        return false;
    }

    if (m_fileinfo->fileSharedName().toLower() == m_shareNamelineEdit->text().toLower()) { //共享名未更改（不区分大小写）时，直接返回true
        return true;
    }
    QDir d("/var/lib/samba/usershares"); //该目录存放了通过程序net共享的共享信息，文件名是共享名,文件名统一小写
    QFileInfoList infolist = d.entryInfoList(QDir::Files); //读取/var/lib/samba/usershares目录下文件信息
    for (QFileInfo info : infolist) {
        if (m_shareNamelineEdit->text().toLower() == info.fileName()) { //查询共享名是否重复，因为程序net保存的文件名统一小写，所以先将共享名转为小写判断
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));

            if (!info.isWritable()) { //不可则无法替换
                dialog.setTitle(tr("The share name is used by another user."));
                dialog.addButton(tr("OK","button"), true);
            } else { //可写则添加替换按钮
                dialog.setTitle(tr("The share name already exists. Do you want to replace the shared folder?"));
                dialog.addButton(tr("Cancel","button"), true);
                dialog.addButton(tr("Replace","button"), false, DDialog::ButtonWarning);
            }

            if (dialog.exec() != DDialog::Accepted) {
                //m_shareCheckBox->setChecked(false);
                m_shareNamelineEdit->setFocus(); //进入编辑
                return false;
            }
            break; //终止循环
        }
    }
    return  true;
}

bool ShareInfoFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_sharePropertyBkgWidget && event->type() == QEvent::Paint) {
        QPainter painter(m_sharePropertyBkgWidget);
        QStyleOption opt;
        opt.init(m_sharePropertyBkgWidget);
        m_sharePropertyBkgWidget->setStyleSheet("background-color: rgba(0,0,0,0.03);border-radius: 8px;");
        m_sharePropertyBkgWidget->style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, m_sharePropertyBkgWidget);
        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);
        int vpos = m_userNamelineEdit->height()*3;
        painter.drawRect(QRect(QPoint(0,vpos),QPoint(m_sharePropertyBkgWidget->width(),vpos)));
    }
    else if (obj == splitLineGray && event->type() == QEvent::Paint) {
        QPainter painter(splitLineGray);
        QColor color(0,0,0);
        color.setAlphaF(0.05);
        painter.setBrush(color);
        painter.setPen(color);
        painter.drawRect(splitLineGray->rect());
    }
    return QFrame::eventFilter(obj, event);
}

ShareInfoFrame::~ShareInfoFrame()
{
    if(m_updateIp){
        m_updateIp->stop();
        m_updateIp->deleteLater();
        m_updateIp = nullptr;
    }
}

