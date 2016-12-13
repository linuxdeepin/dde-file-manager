#include "shareinfoframe.h"
#include "propertydialog.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "widgets/singleton.h"
#include "app/define.h"
#include "dfileservices.h"

#include <QFormLayout>
#include <QProcess>

ShareInfoFrame::ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent) :
    QFrame(parent),
    m_fileinfo(info)
{
    m_jobTimer = new QTimer();
    m_jobTimer->setInterval(500);
    m_jobTimer->setSingleShot(true);
    initUI();
    updateShareInfo(m_fileinfo->absoluteFilePath());
    initConnect();
}

void ShareInfoFrame::initUI()
{
    int labelWidth = 100;
    int fieldWidth = 160;

    m_sharCheckBox = new DCheckBox(this);
    m_sharCheckBox->setFixedHeight(20);
    SectionValueLabel* shareCheckLabel = new SectionValueLabel(tr("Share this folder"));
    shareCheckLabel->setFixedWidth(fieldWidth);

    SectionKeyLabel* shareNameLabel = new SectionKeyLabel(tr("Share name:"));
    shareNameLabel->setFixedWidth(labelWidth);
    m_shareNamelineEdit = new DLineEdit(this);
    m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
    m_shareNamelineEdit->setFixedWidth(fieldWidth);

    SectionKeyLabel* permissionLabel = new SectionKeyLabel(tr("Permission:"));
    permissionLabel->setFixedWidth(labelWidth);
    m_permissoComBox = new DComboBox(this);
    m_permissoComBox->setFixedWidth(fieldWidth);
    QStringList permissions;
    permissions << tr("Read and write") << tr("Read only");
    m_permissoComBox->addItems(permissions);

    SectionKeyLabel* anonymityLabel = new SectionKeyLabel(tr("Anonymity:"));
    anonymityLabel->setFixedWidth(labelWidth);
    m_anonymityCombox = new DComboBox(this);
    m_anonymityCombox->setFixedWidth(fieldWidth);
    QStringList anonymityChoices;
    anonymityChoices << tr("Not allow") << tr("Allow");
    m_anonymityCombox->addItems(anonymityChoices);

    QFormLayout* mainLayoyt = new QFormLayout(this);

    mainLayoyt->addRow(m_sharCheckBox, shareCheckLabel);
    mainLayoyt->addRow(shareNameLabel, m_shareNamelineEdit);
    mainLayoyt->addRow(permissionLabel, m_permissoComBox);
    mainLayoyt->addRow(anonymityLabel, m_anonymityCombox);
    mainLayoyt->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mainLayoyt->setContentsMargins(10, 10, 60, 10);
    setLayout(mainLayoyt);
}

void ShareInfoFrame::initConnect()
{
    connect(m_sharCheckBox, &DCheckBox::stateChanged, this, &ShareInfoFrame::handleCheckBoxChanged);
    connect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
    connect(m_permissoComBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePermissionComboxChanged(int)));
    connect(m_anonymityCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleAnonymityComboxChanged(int)));
    connect(m_jobTimer, &QTimer::timeout, this, &ShareInfoFrame::doShaeInfoSetting);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeleted, this, &ShareInfoFrame::updateShareInfo);
}

void ShareInfoFrame::handleCheckBoxChanged(int state)
{
    qDebug() << state;
    handShareInfoChanged();
}

void ShareInfoFrame::handleShareNameChanged(const QString &name)
{
//    qDebug() << name;
    if(name.isEmpty() || name == ""){
        m_jobTimer->stop();
        return;
    }
    handShareInfoChanged();
}

void ShareInfoFrame::handlePermissionComboxChanged(int index)
{
    qDebug() << index;
    handShareInfoChanged();
}

void ShareInfoFrame::handleAnonymityComboxChanged(int index)
{
    qDebug() << index;
    handShareInfoChanged();
}

void ShareInfoFrame::handShareInfoChanged()
{
    m_jobTimer->start();
}

void ShareInfoFrame::doShaeInfoSetting()
{
    QString shareName = m_shareNamelineEdit->text();

    ShareInfo info;
    info.setPath(m_fileinfo->absoluteFilePath());

    info.setShareName(shareName);
    if (m_permissoComBox->currentIndex() == 0){
        info.setIsWritable(true);
    }else{
        info.setIsWritable(false);
    }

    if (m_anonymityCombox->currentIndex() == 0){
        info.setIsGuestOk(false);
    }else{
        info.setIsGuestOk(true);
    }
    if (m_sharCheckBox->isChecked()){
        userShareManager->addUserShare(info);
        emit folderShared(info.path());
    }else{
        if(info.isWritable()){
            QString cmd = "chmod";
            QStringList args;
            args << "-R"<<"755"<<info.path();
            QProcess::startDetached(cmd, args);
        }
        userShareManager->deleteUserShareByPath(info.path());
    }
}

void ShareInfoFrame::updateShareInfo(const QString &filePath)
{
    qDebug () <<"update:"<< filePath << ", " << m_fileinfo->absoluteFilePath();
    if(filePath != m_fileinfo->absoluteFilePath())
        return;
    ShareInfo info = userShareManager->getShareInfoByPath(filePath);
    qDebug() << info;
    if (!info.shareName().isEmpty()){
        m_sharCheckBox->setChecked(true);
        disconnect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
        m_shareNamelineEdit->setText(info.shareName());
        connect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
        if (info.isWritable()){
            m_permissoComBox->setCurrentIndex(0);
        }else{
            m_permissoComBox->setCurrentIndex(1);
        }

        if (info.isGuestOk()){
            m_anonymityCombox->setCurrentIndex(1);
        }else{
            m_anonymityCombox->setCurrentIndex(0);
        }
    } else {
        m_sharCheckBox->setChecked(false);
        m_permissoComBox->setCurrentIndex(0);
        m_anonymityCombox->setCurrentIndex(0);
        m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
    }
}

void ShareInfoFrame::setFileinfo(const DAbstractFileInfoPointer &fileinfo)
{
    m_fileinfo = fileinfo;
    updateShareInfo(m_fileinfo->absoluteFilePath());
}

ShareInfoFrame::~ShareInfoFrame()
{

}

