// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/radioframe.h"
#include "vaultactivesavekeyfileview.h"
#include "utils/vaultdefine.h"
#include "utils/encryption/operatorcenter.h"

#include <dfm-framework/event/event.h>

#include <DPalette>
#include <DFontSizeManager>
#include <DFileDialog>
#include <DLabel>
#include <DFileChooserEdit>
#include <DFrame>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DFontSizeManager>
#include <DPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainterPath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSaveKeyFileView::VaultActiveSaveKeyFileView(QWidget *parent)
    : VaultBaseView(parent)
{
    initUI();
    initConnect();
}

void VaultActiveSaveKeyFileView::setEncryptInfo(EncryptInfo &info)
{
    info.keyPath = selectfileSavePathEdit->text();
}

void VaultActiveSaveKeyFileView::initUI()
{
    titleLabel = new DLabel(this);
    titleLabel->setForegroundRole(DPalette::TextTitle);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setText(tr("Save Recovery Key"));

    hintMsg = new DLabel(this);
    DFontSizeManager::instance()->bind(hintMsg, DFontSizeManager::T7, QFont::Normal);
    hintMsg->setForegroundRole(DPalette::TextTips);
    hintMsg->setWordWrap(true);
    hintMsg->setAlignment(Qt::AlignCenter);
    hintMsg->setText(tr("Key files can be used to unlock the safe, please keep it in a safe place"));
    otherPathLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(otherPathLabel, DFontSizeManager::T8, QFont::Medium);
    otherPathLabel->setForegroundRole(DPalette::ButtonText);
    otherPathLabel->setText(tr("It is recommended to save it to external storage devices such as a USB drive"));

    otherRadioBtnHitMsg = new DLabel(tr("No permission, please reselect"), this);
    otherRadioBtnHitMsg->hide();
    DFontSizeManager::instance()->bind(otherRadioBtnHitMsg, DFontSizeManager::T9, QFont::Normal);
    otherRadioBtnHitMsg->setForegroundRole(DPalette::TextWarning);

    selectfileSavePathEdit = new DFileChooserEdit(this);
    selectfileSavePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    selectfileSavePathEdit->lineEdit()->setReadOnly(true);
    selectfileSavePathEdit->lineEdit()->setClearButtonEnabled(false);
    filedialog = new DFileDialog(this, QDir::homePath(), QString("pubKey.key"));
    filedialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    filedialog->setDefaultSuffix(QString("key"));
    selectfileSavePathEdit->setDirectoryUrl(QDir::homePath());
    selectfileSavePathEdit->setFileMode(QFileDialog::Directory);
    selectfileSavePathEdit->setNameFilters({ "KEY file(*.key)" });
    selectfileSavePathEdit->setFileDialog(filedialog);
    selectfileSavePathEdit->setEnabled(true);

    // 下一步按钮
    nextBtn = new DSuggestButton(tr("Next"), this);
    nextBtn->setFixedWidth(200);
    nextBtn->setEnabled(false);

    RadioFrame *frame = new RadioFrame;
    RadioFrame *frame1 = new RadioFrame;

    DFrame *line = new DFrame(this);
    line->setObjectName(QString("line"));
    line->setFixedHeight(1);
    line->installEventFilter(this);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->addWidget(otherPathLabel);
    layout3->addWidget(otherRadioBtnHitMsg);
    layout3->addStretch(1);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->setContentsMargins(8, 5, 8, 5);
    layout4->addLayout(layout3);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->setContentsMargins(10, 10, 10, 10);
    layout5->addWidget(selectfileSavePathEdit);

    QVBoxLayout *vlayout5 = new QVBoxLayout(frame1);
    vlayout5->setContentsMargins(0, 0, 0, 0);
    vlayout5->setSpacing(0);
    vlayout5->addLayout(layout4);
    vlayout5->addWidget(line);
    vlayout5->addLayout(layout5);

    QVBoxLayout *vlayout3 = new QVBoxLayout;
    vlayout3->setContentsMargins(20, 0, 20, 0);
    vlayout3->addWidget(frame, 1);
    vlayout3->addWidget(frame1, 1);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->setContentsMargins(0, 0, 0, 0);
    vlayout1->addWidget(titleLabel);
    vlayout1->addWidget(hintMsg);
    vlayout1->addStretch(1);
    vlayout1->addLayout(vlayout3);
    vlayout1->addStretch(3);
    vlayout1->addWidget(nextBtn, 0, Qt::AlignCenter);

    setLayout(vlayout1);
    initUiForSizeMode();

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultSaveKeyTitle);
    AddATTag(qobject_cast<QWidget *>(hintMsg), AcName::kAcLabelVaultSaveKeyContent);
    AddATTag(qobject_cast<QWidget *>(selectfileSavePathEdit), AcName::kAcEditVaultSaveKeyPath);
    AddATTag(qobject_cast<QWidget *>(nextBtn), AcName::kAcBtnVaultSaveKeyNext);
#endif
}

void VaultActiveSaveKeyFileView::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DFontSizeManager::instance()->bind(titleLabel, DSizeModeHelper::element(DFontSizeManager::SizeType::T7, DFontSizeManager::SizeType::T5), QFont::Medium);
#else
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::SizeType::T5, QFont::Medium);
#endif
}

void VaultActiveSaveKeyFileView::initConnect()
{
    connect(selectfileSavePathEdit, &DFileChooserEdit::fileChoosed, this, &VaultActiveSaveKeyFileView::slotChangeEdit);
    connect(filedialog, &DFileDialog::fileSelected, this, &VaultActiveSaveKeyFileView::slotSelectCurrentFile);
    connect(nextBtn, &DPushButton::clicked,
            this, &VaultActiveSaveKeyFileView::accepted);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void VaultActiveSaveKeyFileView::slotChangeEdit(const QString &fileName)
{
    fmDebug() << "Vault: File chooser edit changed, fileName:" << fileName;
    QDir dir(fileName);
    dir.cdUp();
    QString path = dir.absolutePath();
    QFile file(path);
    QFileDevice::Permissions ps = file.permissions();
    auto temp = ps & QFileDevice::WriteUser;
    if (temp != QFileDevice::WriteUser) {
        nextBtn->setEnabled(false);
        otherRadioBtnHitMsg->show();
        fmWarning() << "Vault: No write permission for path:" << path;
    } else if (!fileName.isEmpty()) {
        otherRadioBtnHitMsg->hide();
        nextBtn->setEnabled(true);
    }
}

void VaultActiveSaveKeyFileView::slotSelectCurrentFile(const QString &file)
{
    QFileInfo fileInfo(file);
    if (fileInfo.isDir()) {
        selectfileSavePathEdit->fileDialog()->selectFile(QString("pubKey.key"));
    } else if (!file.endsWith(QString(".key"))) {
        selectfileSavePathEdit->fileDialog()->selectFile(file + QString(".key"));
    }
}

void VaultActiveSaveKeyFileView::showEvent(QShowEvent *event)
{
    selectfileSavePathEdit->clear();
    otherRadioBtnHitMsg->hide();
    QWidget::showEvent(event);
}

bool VaultActiveSaveKeyFileView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->objectName() == QString("line") && event->type() == QEvent::Paint) {
        QFrame *frame = static_cast<QFrame *>(watched);
        QPainter painter(frame);
        QPalette palette = this->palette();
        painter.setBrush(palette.window());

        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);
        painter.drawRoundedRect(rect, 0, 0);
        {
            QPainterPath painterPath;
            painterPath.addRoundedRect(rect, 0, 0);
            painter.drawPath(painterPath);
        }
    }

    return QWidget::eventFilter(watched, event);
}
