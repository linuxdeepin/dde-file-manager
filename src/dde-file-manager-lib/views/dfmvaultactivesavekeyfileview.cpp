/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#include "dfmvaultactivesavekeyfileview.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vaultglobaldefine.h"
#include "operatorcenter.h"
#include "dguiapplicationhelper.h"
#include "controllers/vaultcontroller.h"

#include <DPalette>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainterPath>
#include <QFileDialog>

DGUI_USE_NAMESPACE

DFMVaultActiveSaveKeyFileView::DFMVaultActiveSaveKeyFileView(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void DFMVaultActiveSaveKeyFileView::initUI()
{
    m_title = new DLabel(this);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T7, QFont::Medium);
    m_title->setForegroundRole(DPalette::TextTitle);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setText(tr("Save Recovery Key"));

    m_hintMsg = new DLabel(this);
    DFontSizeManager::instance()->bind(m_hintMsg, DFontSizeManager::T7, QFont::Normal);
    m_hintMsg->setForegroundRole(DPalette::TextTips);
    m_hintMsg->setWordWrap(true);
    m_hintMsg->setAlignment(Qt::AlignCenter);
    m_hintMsg->setText(tr("Keep the key safe to retrieve the vault password later"));

    m_defaultPathRadioBtn = new QRadioButton(this);
    DFontSizeManager::instance()->bind(m_defaultPathRadioBtn, DFontSizeManager::T7, QFont::Medium);
    m_defaultPathRadioBtn->setForegroundRole(DPalette::ButtonText);
    AC_SET_ACCESSIBLE_NAME(m_defaultPathRadioBtn, AC_VAULT_DEFAULT_PATH_RADIOBTN);
    m_defaultPathRadioBtn->setChecked(true);
    m_defaultPathRadioBtn->setText(tr("Save to default path"));

    m_otherPathRadioBtn = new QRadioButton(this);
    DFontSizeManager::instance()->bind(m_otherPathRadioBtn, DFontSizeManager::T7, QFont::Medium);
    m_otherPathRadioBtn->setForegroundRole(DPalette::ButtonText);
    AC_SET_ACCESSIBLE_NAME(m_otherPathRadioBtn, AC_VAULT_OTHER_PATH_RADIOBTN);
    m_otherPathRadioBtn->setText(tr("Save to other locations"));

    m_otherRadioBtnHitMsg = new DLabel(tr("No permission, please reselect"), this);
    m_otherRadioBtnHitMsg->hide();
    DFontSizeManager::instance()->bind(m_otherRadioBtnHitMsg, DFontSizeManager::T9, QFont::Normal);
    m_otherRadioBtnHitMsg->setForegroundRole(DPalette::TextWarning);

    m_SelectfileSavePathEdit = new DFileChooserEdit(this);
    DFontSizeManager::instance()->bind(m_otherPathRadioBtn, DFontSizeManager::T8, QFont::Medium);
    AC_SET_ACCESSIBLE_NAME(m_SelectfileSavePathEdit, AC_VAULT_SELECT_FILE_SAVE_PATH_EDIT);
    m_SelectfileSavePathEdit->lineEdit()->setReadOnly(true);
    m_SelectfileSavePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    QFileDialog *filedialog = new QFileDialog(m_SelectfileSavePathEdit, QDir::homePath(), QString("pubKey.key"));
    filedialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    filedialog->setNameFilter("KEY file(*.key)");
    filedialog->setDefaultSuffix(QString("key"));
    m_SelectfileSavePathEdit->setFileDialog(filedialog);
    m_SelectfileSavePathEdit->setDirectoryUrl(QDir::homePath());
    m_SelectfileSavePathEdit->setEnabled(false);
    m_SelectfileSavePathEdit->setFileMode(QFileDialog::Directory);

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(m_defaultPathRadioBtn, 1);
    group->addButton(m_otherPathRadioBtn, 2);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(slotSelectRadioBtn(QAbstractButton *)));
    connect(m_SelectfileSavePathEdit, &DFileChooserEdit::fileChoosed, this, &DFMVaultActiveSaveKeyFileView::slotChangeEdit);
    connect(filedialog, &QFileDialog::fileSelected, this, &DFMVaultActiveSaveKeyFileView::slotSelectCurrentFile);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    AC_SET_ACCESSIBLE_NAME(m_pNext, AC_VAULT_ACTIVE_KEY_NEXT_BUTTON);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSaveKeyFileView::slotNextBtnClicked);

    RadioFrame *frame = new RadioFrame;

    DLabel *checkBoxLabel = new DLabel(frame);
    DFontSizeManager::instance()->bind(checkBoxLabel, DFontSizeManager::T10, QFont::Normal);
    checkBoxLabel->setForegroundRole(DPalette::TextTips);
    checkBoxLabel->setWordWrap(true);
    checkBoxLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    checkBoxLabel->setText(tr("The default path is invisible to other users, and the path information will not be shown."));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_defaultPathRadioBtn);

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

    QFrame *line = new QFrame(this);
    line->setObjectName(QString("line"));
    line->setFixedHeight(1);
    line->installEventFilter(this);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->addWidget(m_otherPathRadioBtn);
    layout3->addWidget(m_otherRadioBtnHitMsg);
    layout3->addStretch(1);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->setContentsMargins(8, 5, 8, 5);
    layout4->addLayout(layout3);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->setContentsMargins(10, 10, 10, 10);
    layout5->addWidget(m_SelectfileSavePathEdit);

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
    vlayout1->addWidget(m_title);
    vlayout1->addWidget(m_hintMsg);
    vlayout1->addStretch(2);
    vlayout1->addLayout(vlayout3);
    vlayout1->addStretch(2);
    vlayout1->addWidget(m_pNext);

    setLayout(vlayout1);
}

void DFMVaultActiveSaveKeyFileView::slotNextBtnClicked()
{
    //! 获取密钥字符串
    QString pubKey = OperatorCenter::getInstance()->getPubKey();
    if (pubKey.isEmpty()) {
        return;
    }

    bool flg = false;
    if (m_defaultPathRadioBtn->isChecked()) {
        //! 密钥保存默认路径
        QString path = VAULT_BASE_PATH + QString("/") + (RSA_PUB_KEY_FILE_NAME) + QString(".key");
        flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    } else if (m_otherPathRadioBtn->isChecked()) {
        //! 密钥保存用户指定路径
        QString path = m_SelectfileSavePathEdit->text();
        flg = OperatorCenter::getInstance()->saveKey(pubKey, path);
    }

    if (flg) {
        emit sigAccepted();
    }
}

void DFMVaultActiveSaveKeyFileView::slotSelectRadioBtn(QAbstractButton *btn)
{
    if (btn == m_defaultPathRadioBtn) {
        m_SelectfileSavePathEdit->setEnabled(false);
        m_pNext->setEnabled(true);
    } else if (btn == m_otherPathRadioBtn) {
        m_SelectfileSavePathEdit->setEnabled(true);
        if (m_SelectfileSavePathEdit->text().isEmpty())
            m_pNext->setEnabled(false);
    }
}

void DFMVaultActiveSaveKeyFileView::slotChangeEdit(const QString &fileName)
{
    QDir dir(fileName);
    dir.cdUp();
    QString path = dir.absolutePath();
    QFile file(path);
    QFileDevice::Permissions ps = file.permissions();
    auto temp = ps & QFileDevice::WriteUser;
    if (temp != QFileDevice::WriteUser) {
        m_pNext->setEnabled(false);
        m_otherRadioBtnHitMsg->show();
    } else if (!fileName.isEmpty()) {
        m_otherRadioBtnHitMsg->hide();
        m_pNext->setEnabled(true);
    }
}

void DFMVaultActiveSaveKeyFileView::slotSelectCurrentFile(const QString &file)
{
    QFileInfo fileInfo(file);
    if (fileInfo.isDir()) {
        m_SelectfileSavePathEdit->fileDialog()->selectFile(QString("pubKey.key"));
    } else if (!file.endsWith(QString(".key"))) {
        m_SelectfileSavePathEdit->fileDialog()->selectFile(file + QString(".key"));
    }
}

void DFMVaultActiveSaveKeyFileView::showEvent(QShowEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::CREATEVAULTPAGE);
    m_defaultPathRadioBtn->setChecked(true);
    m_SelectfileSavePathEdit->clear();
    m_otherRadioBtnHitMsg->hide();
    QWidget::showEvent(event);
}

bool DFMVaultActiveSaveKeyFileView::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == QString("line") && event->type() == QEvent::Paint) {
        QFrame *frame = static_cast<QFrame*>(watched);
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
