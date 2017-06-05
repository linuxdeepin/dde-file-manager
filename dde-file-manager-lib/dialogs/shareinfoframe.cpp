#include "shareinfoframe.h"
#include "propertydialog.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileservices.h"

#include <QFormLayout>
#include <QProcess>
#include <QComboBox>

ShareInfoFrame::ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent) :
    QFrame(parent),
    m_fileinfo(info)
{
    m_jobTimer = new QTimer();
    m_jobTimer->setInterval(1000);
    m_jobTimer->setSingleShot(true);
    initUI();
    updateShareInfo(m_fileinfo->absoluteFilePath());
    initConnect();
}

void ShareInfoFrame::initUI()
{
    int labelWidth = 100;
    int fieldWidth = 160;

    m_shareCheckBox = new QCheckBox(this);
    m_shareCheckBox->setFixedHeight(20);
    SectionValueLabel* shareCheckLabel = new SectionValueLabel(tr("Share this folder"));
    shareCheckLabel->setFixedWidth(fieldWidth);

    SectionKeyLabel* shareNameLabel = new SectionKeyLabel(tr("Share name:"));
    shareNameLabel->setFixedWidth(labelWidth);
    m_shareNamelineEdit = new QLineEdit(this);
    m_shareNamelineEdit->setObjectName("ShareNameEdit");
    m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
    m_shareNamelineEdit->setFixedWidth(fieldWidth);

    SectionKeyLabel* permissionLabel = new SectionKeyLabel(tr("Permission:"));
    permissionLabel->setFixedWidth(labelWidth);
    m_permissoComBox = new QComboBox(this);
    m_permissoComBox->setFixedHeight(23);
    m_permissoComBox->setFixedWidth(fieldWidth);
    QStringList permissions;
    permissions << tr("Read and write") << tr("Read only");
    m_permissoComBox->addItems(permissions);

    SectionKeyLabel* anonymityLabel = new SectionKeyLabel(tr("Anonymous:"));
    anonymityLabel->setFixedWidth(labelWidth);
    m_anonymityCombox = new QComboBox(this);
    m_anonymityCombox->setFixedHeight(23);
    m_anonymityCombox->setFixedWidth(fieldWidth);
    QStringList anonymityChoices;
    anonymityChoices << tr("Not allow") << tr("Allow");
    m_anonymityCombox->addItems(anonymityChoices);

    QFormLayout* mainLayoyt = new QFormLayout(this);

    mainLayoyt->addRow(m_shareCheckBox, shareCheckLabel);
    mainLayoyt->addRow(shareNameLabel, m_shareNamelineEdit);
    mainLayoyt->addRow(permissionLabel, m_permissoComBox);
    mainLayoyt->addRow(anonymityLabel, m_anonymityCombox);
    mainLayoyt->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mainLayoyt->setContentsMargins(10, 10, 60, 10);
    setLayout(mainLayoyt);
}

void ShareInfoFrame::initConnect()
{
    connect(m_shareCheckBox, &QCheckBox::clicked, this, &ShareInfoFrame::handleCheckBoxChanged);
    connect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
    connect(m_permissoComBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handlePermissionComboxChanged(int)));
    connect(m_anonymityCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleAnonymityComboxChanged(int)));
    connect(m_jobTimer, &QTimer::timeout, this, &ShareInfoFrame::doShareInfoSetting);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &ShareInfoFrame::updateShareInfo);
    connect(userShareManager, &UserShareManager::userShareDeleted, this, &ShareInfoFrame::updateShareInfo);
}

void ShareInfoFrame::handleCheckBoxChanged(const bool& checked)
{
    if(checked){
        emit folderShared(m_fileinfo->absoluteFilePath());
        activateWidgets();
        m_permissoComBox->setCurrentIndex(1);
    } else{
        disactivateWidgets();
    }
    doShareInfoSetting();
}

void ShareInfoFrame::handleShareNameChanged(const QString &name)
{
    if(name.isEmpty() || name == ""){
        m_jobTimer->stop();
        return;
    }
    handShareInfoChanged();
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
    m_jobTimer->start();
}

void ShareInfoFrame::doShareInfoSetting()
{
    if (!m_shareCheckBox->isChecked()) {
        DFileService::instance()->unShareFolder(this, m_fileinfo->fileUrl());
        return;
    }

    DFileService::instance()->shareFolder(this, m_fileinfo->fileUrl(), m_shareNamelineEdit->text(),
                                          m_permissoComBox->currentIndex() == 0,
                                          m_anonymityCombox->currentIndex() != 0);
}

void ShareInfoFrame::updateShareInfo(const QString &filePath)
{
    if (filePath != m_fileinfo->absoluteFilePath())
        return;

    if (!m_fileinfo->fileSharedName().isEmpty()) {
        m_shareCheckBox->setChecked(true);
        disconnect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);
        int cursorPos = m_shareNamelineEdit->cursorPosition();
        m_shareNamelineEdit->setText(m_fileinfo->fileSharedName());
        m_shareNamelineEdit->setCursorPosition(cursorPos);
        connect(m_shareNamelineEdit, &DLineEdit::textChanged, this, &ShareInfoFrame::handleShareNameChanged);

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
        m_permissoComBox->setCurrentIndex(0);
        m_anonymityCombox->setCurrentIndex(0);
        m_shareNamelineEdit->setText(m_fileinfo->fileDisplayName());
        disactivateWidgets();
    }
}

void ShareInfoFrame::activateWidgets()
{
    m_shareNamelineEdit->setEnabled(true);
    m_permissoComBox->setEnabled(true);
    m_anonymityCombox->setEnabled(true);
}

void ShareInfoFrame::disactivateWidgets()
{
    m_shareNamelineEdit->setEnabled(false);
    m_permissoComBox->setEnabled(false);
    m_anonymityCombox->setEnabled(false);
}

void ShareInfoFrame::setFileinfo(const DAbstractFileInfoPointer &fileinfo)
{
    m_fileinfo = fileinfo;
    updateShareInfo(m_fileinfo->absoluteFilePath());
}

ShareInfoFrame::~ShareInfoFrame()
{

}

