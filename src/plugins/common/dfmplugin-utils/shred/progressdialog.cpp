// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progressdialog.h"

#include <DWaterProgress>
#include <DLabel>

#include <QVBoxLayout>
#include <QKeyEvent>

using namespace dfmplugin_utils;
DWIDGET_USE_NAMESPACE

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    progress = new DWaterProgress(this);
    infoLable = new DLabel(this);
    infoLable->setAlignment(Qt::AlignCenter);
    infoLable->setElideMode(Qt::ElideMiddle);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(progress, 0, Qt::AlignHCenter);
    mainLay->addWidget(infoLable);
    mainLay->addSpacing(10);
    mainLay->setSpacing(30);

    setLayout(mainLay);
}

void ProgressWidget::setValue(int value, const QString &fileName)
{
    progress->start();
    progress->setValue(value);
    if (!fileName.isEmpty())
        infoLable->setText(tr("Shredding file \" %1 \"").arg(fileName));
}

void ProgressWidget::stopProgress()
{
    progress->stop();
}

ShredFailedWidget::ShredFailedWidget(const QString &reason, QWidget *parent)
    : QWidget(parent)
{
    DLabel *failedImage = new DLabel(this);
    failedImage->setPixmap(QIcon::fromTheme("dialog-error").pixmap(100, 100));

    infoLabel = new DLabel(this);
    infoLabel->setMaximumWidth(340);
    infoLabel->setElideMode(Qt::ElideMiddle);
    infoLabel->setToolTip(reason);

    QString showText = reason;
    if (reason.length() > 50) {
        showText = reason.left(25) + "..." + reason.right(22);
    }
    infoLabel->setText(showText);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(failedImage, 0, Qt::AlignHCenter);
    mainLay->addWidget(infoLabel, 0, Qt::AlignHCenter);

    setLayout(mainLay);
}

ProgressDialog::ProgressDialog(QWidget *parent)
    : DDialog(parent)
{
    setFixedSize(380, 310);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    initUi();
    initConnect();
}

void ProgressDialog::updateProgressValue(int value, const QString &fileName)
{
    if (!proWidget || !proWidget->isVisible())
        return;

    if (value < 0 || value > 100) {
        fmCritical() << "The progress value error!";
        return;
    }

    if (proWidget && proWidget->isVisible()) {
        proWidget->setValue(value, fileName);
    }
}

void ProgressDialog::handleShredResult(bool result, const QString &reason)
{
    if (result) {

    } else {
        if (proWidget && proWidget->isVisible()) {
            proWidget->stopProgress();
        }
        clearContents(true);

        setTitle(tr("File shredder failure"));
        failedWidget = new ShredFailedWidget(reason, this);
        addContent(failedWidget);
        addButton(tr("Close"));
    }
}

void ProgressDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        event->ignore();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void ProgressDialog::handleButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    Q_UNUSED(index);

    emit rejected();
}

void ProgressDialog::initUi()
{
    setIcon(QIcon::fromTheme("dialog-warning"));
    setTitle(tr("Shredding file"));

    proWidget = new ProgressWidget(this);
    addContent(proWidget);
}

void ProgressDialog::initConnect()
{
    connect(this, &ProgressDialog::buttonClicked, this, &ProgressDialog::handleButtonClicked);
}
