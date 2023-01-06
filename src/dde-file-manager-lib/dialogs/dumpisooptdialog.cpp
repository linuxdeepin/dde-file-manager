// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dumpisooptdialog.h"
#include "app/define.h"
#include "dialogs/dialogmanager.h"
#include "interfaces/dfmglobal.h"
#include "fileoperations/filejob.h"

#include <DFileDialog>
#include <QWindow>
#include <QLayout>
#include <QFont>
#include <QLineEdit>
#include <QtConcurrent>

#include <disomaster.h>

DWIDGET_USE_NAMESPACE

DumpISOOptDialog::DumpISOOptDialog(const QString &dev, const QString &discName, QWidget *parent)
    : DDialog(parent),
      m_curDevice(dev),
      m_curDiscName(discName)
{
    initliazeUi();
    initConnect();
}

void DumpISOOptDialog::setJobWindowId(quint64 wid)
{
    m_winId = wid;
}

void DumpISOOptDialog::initliazeUi()
{
    if (DFMGlobal::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setModal(true);
    setFixedSize(400, 242);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    addButton(QObject::tr("Cancel", "button"));
    int index = addButton(QObject::tr("Create ISO Image", "button"), true, DDialog::ButtonType::ButtonRecommend);
    m_createImgBtn = getButton(index);
    if (m_createImgBtn)
        m_createImgBtn->setEnabled(false);
    layout()->setContentsMargins(0, 0, 0, 0);
    m_contentWidget = new QWidget(this);
    // m_contentWidget->setStyleSheet("border: 1px solid red"); // debug
    QVBoxLayout *contentLay = new QVBoxLayout;
    contentLay->setMargin(0);
    m_contentWidget->setLayout(contentLay);
    addContent(m_contentWidget, Qt::AlignTop);

    // comment title label
    m_saveAsImgLabel = new QLabel(QObject::tr("Save as Image File"));
    m_saveAsImgLabel->setFixedSize(308, 20);
    QFont font = m_saveAsImgLabel->font();
    font.setPixelSize(14);
    font.setWeight(QFont::Medium);
    font.setFamily("SourceHanSansSC");
    m_saveAsImgLabel->setFont(font);
    m_saveAsImgLabel->setAlignment(Qt::AlignHCenter);
    contentLay->addWidget(m_saveAsImgLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // comment label
    m_commentLabel = new QLabel(QObject::tr("All files in the disc will be packaged and created as an ISO image file."));
    m_commentLabel->setWordWrap(true);
    m_commentLabel->setFixedSize(360, 41);
    font = m_commentLabel->font();
    font.setPixelSize(14);
    font.setFamily("SourceHanSansSC");
    m_commentLabel->setFont(font);
    m_commentLabel->setAlignment(Qt::AlignHCenter);
    contentLay->addWidget(m_commentLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // savepath label
    m_savePathLabel = new QLabel(QObject::tr("Save the ISO image here:"));
    font = m_savePathLabel->font();
    font.setPixelSize(12);
    font.setFamily("SourceHanSansSC");
    m_savePathLabel->setFont(font);
    contentLay->addWidget(m_savePathLabel, 0);

    // file chooser
    m_fileChooser = new DFileChooserEdit;
    m_fileChooser->setFileMode(QFileDialog::FileMode::Directory);
    const QString &stdDocPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
    m_fileChooser->setDirectoryUrl(QUrl::fromLocalFile(stdDocPath));

    contentLay->addWidget(m_fileChooser);
}

void DumpISOOptDialog::initConnect()
{
    Q_ASSERT(m_fileChooser);
    connect(this, &DumpISOOptDialog::buttonClicked, this, &DumpISOOptDialog::onButtonClicked);
    connect(m_fileChooser, &DFileChooserEdit::fileChoosed, this, &DumpISOOptDialog::onFileChoosed);
    connect(m_fileChooser, &DFileChooserEdit::textChanged, this, &DumpISOOptDialog::onPathChanged);
}

void DumpISOOptDialog::onButtonClicked(int index, const QString &text)
{
    qInfo() << "button clicked" << text;

    // start dump image
    if (index == 1) {
        auto winId { m_winId };
        DUrl dev {m_curDevice};
        DUrl image { DUrl::fromUserInput(m_fileChooser->text()) };
        QtConcurrent::run([=] {
            QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalDumpImage));
            job->moveToThread(qApp->thread());
            job->setWindowId(static_cast<int>(winId));
            dialogManager->addJob(job);
            job->doISODump(dev, image);
            qInfo() << "Start dump image";
            dialogManager->removeJob(job->getJobId(), true);
        });
    }
}

void DumpISOOptDialog::onFileChoosed(const QString &fileName)
{
    Q_ASSERT(!m_curDiscName.isEmpty());
    QFileInfo info { fileName + "/" + m_curDiscName + ".iso" };
    int serial { 1 };
    static int maxSerial { 4096 };

    // check repetat name iso
    while (info.exists()) {
        if (serial >= maxSerial) {
            qWarning() << "Repeat name files too much!";
            return;
        }
        QString discName { m_curDiscName + "(" + QString::number(serial) + ")" };
        ++serial;
        info = QFileInfo { fileName + "/" + discName + ".iso" };
    }

    m_fileChooser->setText(info.absoluteFilePath());
}

void DumpISOOptDialog::onPathChanged(const QString &path)
{
    const DUrl &url = DUrl::fromUserInput(path);
    if (url.isEmpty() || !url.isValid() || !url.isLocalFile()) {
        qWarning() << "Path:" << path << "is invalid";
        m_createImgBtn->setEnabled(false);
        return;
    }

    m_createImgBtn->setEnabled(true);
}

