/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dumpisooptdialog.h"
#include "utils/burnjobmanager.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/windowutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <DFileDialog>
#include <QWindow>
#include <QLabel>
#include <QLayout>
#include <QFont>
#include <QLineEdit>
#include <QtConcurrent>

namespace dfmplugin_burn {
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace GlobalServerDefines;

DumpISOOptDialog::DumpISOOptDialog(const QString &devId, QWidget *parent)
    : DDialog(parent), curDevId(devId)
{
    initliazeUi();
    initData();
    initConnect();
}

DumpISOOptDialog::~DumpISOOptDialog()
{
}

void DumpISOOptDialog::initliazeUi()
{
    if (WindowUtils::isWayLand()) {
        setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        setAttribute(Qt::WA_NativeWindow);
        windowHandle()->setProperty("_d_dwayland_minimizable", false);
        windowHandle()->setProperty("_d_dwayland_maximizable", false);
        windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    setModal(true);
    setFixedSize(400, 242);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    addButton(QObject::tr("Cancel", "button"));
    int index = addButton(tr("Create ISO Image", "button"), true, DDialog::ButtonType::ButtonRecommend);
    createImgBtn = getButton(index);
    if (createImgBtn)
        createImgBtn->setEnabled(false);
    layout()->setContentsMargins(0, 0, 0, 0);
    contentWidget = new QWidget(this);
    // m_contentWidget->setStyleSheet("border: 1px solid red"); // debug
    QVBoxLayout *contentLay = new QVBoxLayout;
    contentLay->setMargin(0);
    contentWidget->setLayout(contentLay);
    addContent(contentWidget, Qt::AlignTop);

    // comment title label
    saveAsImgLabel = new QLabel(tr("Save as Image File"));
    saveAsImgLabel->setFixedSize(308, 20);
    QFont font = saveAsImgLabel->font();
    font.setPixelSize(14);
    font.setWeight(QFont::Medium);
    font.setFamily("SourceHanSansSC");
    saveAsImgLabel->setFont(font);
    saveAsImgLabel->setAlignment(Qt::AlignHCenter);
    contentLay->addWidget(saveAsImgLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // comment label
    commentLabel = new QLabel(tr("All files in the disc will be packaged and created as an ISO image file."));
    commentLabel->setWordWrap(true);
    commentLabel->setFixedSize(360, 41);
    font = commentLabel->font();
    font.setPixelSize(14);
    font.setFamily("SourceHanSansSC");
    commentLabel->setFont(font);
    commentLabel->setAlignment(Qt::AlignHCenter);
    contentLay->addWidget(commentLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // savepath label
    savePathLabel = new QLabel(tr("Save the ISO image here:"));
    font = savePathLabel->font();
    font.setPixelSize(12);
    font.setFamily("SourceHanSansSC");
    savePathLabel->setFont(font);
    contentLay->addWidget(savePathLabel, 0);

    // file chooser
    fileChooser = new DFileChooserEdit;
    fileChooser->setFileMode(QFileDialog::FileMode::Directory);
    const QString &stdDocPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
    fileChooser->setDirectoryUrl(QUrl::fromLocalFile(stdDocPath));

    contentLay->addWidget(fileChooser);
}

void DumpISOOptDialog::initConnect()
{
    Q_ASSERT(fileChooser);
    connect(this, &DumpISOOptDialog::buttonClicked, this, &DumpISOOptDialog::onButtonClicked);
    connect(fileChooser, &DFileChooserEdit::fileChoosed, this, &DumpISOOptDialog::onFileChoosed);
    connect(fileChooser, &DFileChooserEdit::textChanged, this, &DumpISOOptDialog::onPathChanged);
}

void DumpISOOptDialog::initData()
{
    const auto &map { DevProxyMng->queryBlockInfo(curDevId) };
    curDiscName = qvariant_cast<QString>(map[DeviceProperty::kIdLabel]);
    curDev = qvariant_cast<QString>(map[DeviceProperty::kDevice]);
    if (!curDiscName.isEmpty())
        return;

    auto udiks2Size { map.value(DeviceProperty::kUDisks2Size).toULongLong() };
    curDiscName = DeviceUtils::nameOfDefault(curDiscName, udiks2Size);
}

void DumpISOOptDialog::onButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text)
    // start dump image
    if (index == 1) {
        QUrl image { UrlRoute::fromUserInput(fileChooser->text()) };
        if (curDev.isEmpty() || !image.isValid())
            qWarning() << "Error params: " << curDev << image;
        BurnJobManager::instance()->startDumpISOImage(curDev, image);
    }
}

void DumpISOOptDialog::onFileChoosed(const QString &fileName)
{
    Q_ASSERT(!curDiscName.isEmpty());

    auto info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(fileName + "/" + curDiscName + ".iso"));
    if (!info)
        return;

    int serial { 1 };
    static const int kMaxSerial { 4096 };

    // check repetat name iso
    while (info->exists()) {
        if (serial >= kMaxSerial) {
            qWarning() << "Repeat name files too much!";
            return;
        }
        QString discName { curDiscName + "(" + QString::number(serial) + ")" };
        ++serial;
        info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(fileName + "/" + discName + ".iso"));
    }

    fileChooser->setText(info->absoluteFilePath());
}

void DumpISOOptDialog::onPathChanged(const QString &path)
{
    const QUrl &url { UrlRoute::fromUserInput(path) };
    if (url.isEmpty() || !url.isValid() || !url.isLocalFile()) {
        qWarning() << "Path:" << path << "is invalid";
        createImgBtn->setEnabled(false);
        return;
    }

    createImgBtn->setEnabled(true);
}

}   // namespace dfmplugin_burn
