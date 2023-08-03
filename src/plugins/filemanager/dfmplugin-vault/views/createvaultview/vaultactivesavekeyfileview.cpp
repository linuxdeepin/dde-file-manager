// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivesavekeyfileview.h"
#include "utils/vaultdefine.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/policy/policymanager.h"

#include <dfm-framework/event/event.h>

#include <DPalette>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DFileDialog>
#include <DLabel>
#include <DFileChooserEdit>
#include <DFrame>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainterPath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveSaveKeyFileView::VaultActiveSaveKeyFileView(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

void VaultActiveSaveKeyFileView::initUI()
{
    titleLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T7, QFont::Medium);
    titleLabel->setForegroundRole(DPalette::TextTitle);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setText(tr("Save Recovery Key"));

    hintMsg = new DLabel(this);
    DFontSizeManager::instance()->bind(hintMsg, DFontSizeManager::T7, QFont::Normal);
    hintMsg->setForegroundRole(DPalette::TextTips);
    hintMsg->setWordWrap(true);
    hintMsg->setAlignment(Qt::AlignCenter);
    hintMsg->setText(tr("Keep the key safe to retrieve the vault password later"));

    defaultPathRadioBtn = new QRadioButton(this);
    DFontSizeManager::instance()->bind(defaultPathRadioBtn, DFontSizeManager::T7, QFont::Medium);
    defaultPathRadioBtn->setForegroundRole(DPalette::ButtonText);
    defaultPathRadioBtn->setChecked(true);
    defaultPathRadioBtn->setText(tr("Save to default path"));

    otherPathRadioBtn = new QRadioButton(this);
    DFontSizeManager::instance()->bind(otherPathRadioBtn, DFontSizeManager::T7, QFont::Medium);
    otherPathRadioBtn->setForegroundRole(DPalette::ButtonText);
    otherPathRadioBtn->setText(tr("Save to other locations"));

    otherRadioBtnHitMsg = new DLabel(tr("No permission, please reselect"), this);
    otherRadioBtnHitMsg->hide();
    DFontSizeManager::instance()->bind(otherRadioBtnHitMsg, DFontSizeManager::T9, QFont::Normal);
    otherRadioBtnHitMsg->setForegroundRole(DPalette::TextWarning);

    selectfileSavePathEdit = new DFileChooserEdit(this);
    DFontSizeManager::instance()->bind(otherPathRadioBtn, DFontSizeManager::T8, QFont::Medium);
    selectfileSavePathEdit->lineEdit()->setReadOnly(true);
    selectfileSavePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    filedialog = new DFileDialog(this, QDir::homePath(), QString("pubKey.key"));
    filedialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    filedialog->setDefaultSuffix(QString("key"));
    selectfileSavePathEdit->setDirectoryUrl(QDir::homePath());
    selectfileSavePathEdit->setFileMode(QFileDialog::Directory);
    selectfileSavePathEdit->setNameFilters({ "KEY file(*.key)" });
    selectfileSavePathEdit->setFileDialog(filedialog);
    selectfileSavePathEdit->setEnabled(false);

    group = new QButtonGroup(this);
    group->addButton(defaultPathRadioBtn, 1);
    group->addButton(otherPathRadioBtn, 2);

    // 下一步按钮
    nextBtn = new DPushButton(tr("Next"), this);

    RadioFrame *frame = new RadioFrame;

    DLabel *checkBoxLabel = new DLabel(frame);
    DFontSizeManager::instance()->bind(checkBoxLabel, DFontSizeManager::T10, QFont::Normal);
    checkBoxLabel->setForegroundRole(DPalette::TextTips);
    checkBoxLabel->setWordWrap(true);
    checkBoxLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    checkBoxLabel->setText(tr("The default path is invisible to other users, and the path information will not be shown."));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(defaultPathRadioBtn);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->setContentsMargins(30, 0, 0, 0);
    layout1->addWidget(checkBoxLabel);

    QVBoxLayout *layout2 = new QVBoxLayout(frame);
    layout2->setContentsMargins(8, 5, 8, 5);
    layout2->setSpacing(0);
    layout2->addLayout(layout);
    layout2->addLayout(layout1);
    layout2->addStretch(2);

    RadioFrame *frame1 = new RadioFrame;

    DFrame *line = new DFrame(this);
    line->setObjectName(QString("line"));
    line->setFixedHeight(1);
    line->installEventFilter(this);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->addWidget(otherPathRadioBtn);
    layout3->addWidget(otherRadioBtnHitMsg);
    layout3->addStretch(1);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->setContentsMargins(8, 5, 8, 5);
    layout4->addLayout(layout3);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->setContentsMargins(10, 10, 10, 10);
    layout5->addWidget(selectfileSavePathEdit);

    QVBoxLayout *vlayout5 = new QVBoxLayout(frame1);
    vlayout5->setMargin(0);
    vlayout5->setSpacing(0);
    vlayout5->addLayout(layout4);
    vlayout5->addWidget(line);
    vlayout5->addLayout(layout5);

    QVBoxLayout *vlayout3 = new QVBoxLayout;
    vlayout3->setContentsMargins(20, 0, 20, 0);
    vlayout3->addWidget(frame, 1);
    vlayout3->addSpacing(3);
    vlayout3->addWidget(frame1, 1);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->setContentsMargins(0, 0, 0, 0);
    vlayout1->addWidget(titleLabel);
    vlayout1->addWidget(hintMsg);
    vlayout1->addStretch(2);
    vlayout1->addLayout(vlayout3);
    vlayout1->addStretch(2);
    vlayout1->addWidget(nextBtn);

    setLayout(vlayout1);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultSaveKeyTitle);
    AddATTag(qobject_cast<QWidget *>(hintMsg), AcName::kAcLabelVaultSaveKeyContent);
    AddATTag(qobject_cast<QWidget *>(defaultPathRadioBtn), AcName::kAcRadioVaultSaveKeyDefault);
    AddATTag(qobject_cast<QWidget *>(checkBoxLabel), AcName::kAcLabelVaultSaveKeyDefaultMsg);
    AddATTag(qobject_cast<QWidget *>(otherPathRadioBtn), AcName::kAcRadioVaultSaveKeyOther);
    AddATTag(qobject_cast<QWidget *>(selectfileSavePathEdit), AcName::kAcEditVaultSaveKeyPath);
    AddATTag(qobject_cast<QWidget *>(nextBtn), AcName::kAcBtnVaultSaveKeyNext);
#endif
}

void VaultActiveSaveKeyFileView::initConnect()
{
    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(slotSelectRadioBtn(QAbstractButton *)));
    connect(selectfileSavePathEdit, &DFileChooserEdit::fileChoosed, this, &VaultActiveSaveKeyFileView::slotChangeEdit);
    connect(filedialog, &DFileDialog::fileSelected, this, &VaultActiveSaveKeyFileView::slotSelectCurrentFile);
    connect(nextBtn, &DPushButton::clicked,
            this, &VaultActiveSaveKeyFileView::slotNextBtnClicked);
}

void VaultActiveSaveKeyFileView::slotNextBtnClicked()
{
    //! 获取密钥字符串
    QString pubKey = OperatorCenter::getInstance()->getPubKey();
    if (pubKey.isEmpty()) {
        return;
    }

    bool flg = false;
    if (defaultPathRadioBtn->isChecked()) {
        //! 密钥保存默认路径
        QString path = kVaultBasePath + QString("/") + (kRSAPUBKeyFileName) + QString(".key");
        flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    } else if (otherPathRadioBtn->isChecked()) {
        //! 密钥保存用户指定路径
        QString path = selectfileSavePathEdit->text();
        flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    }

    if (flg) {
        emit sigAccepted();
    }
}

void VaultActiveSaveKeyFileView::slotSelectRadioBtn(QAbstractButton *btn)
{
    if (btn == defaultPathRadioBtn) {
        selectfileSavePathEdit->setEnabled(false);
        nextBtn->setEnabled(true);
    } else if (btn == otherPathRadioBtn) {
        selectfileSavePathEdit->setEnabled(true);
        filedialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        if (selectfileSavePathEdit->text().isEmpty())
            nextBtn->setEnabled(false);
    }
}

void VaultActiveSaveKeyFileView::slotChangeEdit(const QString &fileName)
{
    QDir dir(fileName);
    dir.cdUp();
    QString path = dir.absolutePath();
    QFile file(path);
    QFileDevice::Permissions ps = file.permissions();
    auto temp = ps & QFileDevice::WriteUser;
    if (temp != QFileDevice::WriteUser) {
        nextBtn->setEnabled(false);
        otherRadioBtnHitMsg->show();
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
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kCreateVaultPage);
    defaultPathRadioBtn->setChecked(true);
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
        painter.setBrush(palette.background());

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

RadioFrame::RadioFrame(QFrame *parent)
    : QFrame(parent)
{
    DPalette pal;
    QColor color;
    color.setRgbF(0.9, 0.9, 0.9, 0.03);
    pal.setColor(DPalette::Light, color);
    this->setPalette(pal);
}

void RadioFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);   // 反锯齿;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter.setBrush(QBrush(QColor("#4c252525")));
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        QColor color;
        color.setRgbF(0.0, 0.0, 0.0, 0.03);
        painter.setBrush(QBrush(color));
    }

    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 8, 8);
    //也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 8, 8);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);
    }
    QFrame::paintEvent(event);
}
