/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "computerpropertydialog.h"
#include "dfm-base/utils/universalutils.h"

#include <DSysInfo>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QFile>
#include <QDBusInterface>
#include <DFrame>

#include <cmath>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent)
    : DDialog(parent)
{
    iniUI();
    iniThread();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

ComputerPropertyDialog::~ComputerPropertyDialog()
{
    if (thread) {
        if (thread->isRunning()) {
            thread->stopThread();
        }
        thread->quit();
        thread->deleteLater();
    }
}

void ComputerPropertyDialog::iniUI()
{
    setTitle(tr("Computer"));

    computerIcon = new DLabel(this);
    QString distributerLogoPath = DSysInfo::distributionOrgLogo();
    QIcon logoIcon;
    if (!distributerLogoPath.isEmpty() && QFile::exists(distributerLogoPath)) {
        logoIcon = QIcon(distributerLogoPath);
    } else {
        logoIcon = QIcon::fromTheme("dfm_deepin_logo");
    }
    computerIcon->setPixmap(logoIcon.pixmap(152, 39));

    basicInfo = new DLabel(tr("Basic Info"), this);
    DFontSizeManager::instance()->bind(basicInfo, DFontSizeManager::T5, QFont::Medium);
    basicInfo->setForegroundRole(DPalette::TextTitle);
    basicInfo->setAlignment(Qt::AlignLeft);

    computerName = new KeyValueLabel(this);
    computerName->setLeftValue(tr("PC Name"));
    computerName->setRightFontSizeWeight(DFontSizeManager::T9);
    computerEdition = new KeyValueLabel(this);
    computerEdition->setLeftValue(tr("Edition"));
    computerEdition->setRightFontSizeWeight(DFontSizeManager::T9);
    computerVersionNum = new KeyValueLabel(this);
    computerVersionNum->setLeftValue(tr("Version"));
    computerVersionNum->setRightFontSizeWeight(DFontSizeManager::T9);
    computerType = new KeyValueLabel(this);
    computerType->setLeftValue(tr("Type"));
    computerType->setRightFontSizeWeight(DFontSizeManager::T9);
    computerCpu = new KeyValueLabel(this);
    computerCpu->setLeftValue(tr("Processor"), Qt::ElideNone, Qt::AlignLeft | Qt::AlignVCenter);
    computerCpu->setRightFontSizeWeight(DFontSizeManager::T9);
    computerMemory = new KeyValueLabel(this);
    computerMemory->setLeftValue(tr("Memory"));
    computerMemory->setRightFontSizeWeight(DFontSizeManager::T9);

    DFrame *basicInfoFrame = new DFrame(this);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 2, 10, 5);
    vlayout->setSpacing(8);
    vlayout->addWidget(basicInfo);
    vlayout->addWidget(computerName);
    vlayout->addWidget(computerEdition);
    vlayout->addWidget(computerVersionNum);
    vlayout->addWidget(computerType);
    vlayout->addWidget(computerCpu);
    vlayout->addWidget(computerMemory);
    basicInfoFrame->setLayout(vlayout);

    QFrame *contentFrame = new QFrame(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(computerIcon, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(basicInfoFrame);

    contentFrame->setLayout(mainLayout);

    setFixedWidth(320);

    addContent(contentFrame);
}

void ComputerPropertyDialog::iniThread()
{
    thread = new ComputerInfoThread();
    connect(thread, &ComputerInfoThread::sigSendComputerInfo, this, &ComputerPropertyDialog::computerProcess);
}

/*!
 * \brief                   接收线程发来的获取到计算机的信息，并对信息进行设置到相应控件中
 * \param computerInfo      接收到的计算机信息
 */
void ComputerPropertyDialog::computerProcess(QStringList computerInfo)
{
    computerName->setRightValue(computerInfo[0]);
    computerEdition->setRightValue(computerInfo[1]);
    computerVersionNum->setRightValue(computerInfo[2]);
    computerType->setRightValue(computerInfo[3]);
    computerCpu->setRightValue(computerInfo[4]);
    computerCpu->setRightWordWrap(true);
    QFontMetrics fontMetrics(computerCpu->font());
    QRect boundingRect = fontMetrics.boundingRect(computerInfo[4]);
    computerCpu->setRowMinimumHeight(boundingRect.height() - boundingRect.y());
    computerMemory->setRightValue(computerInfo[5].arg(tr("Available")));
}

void ComputerPropertyDialog::showEvent(QShowEvent *event)
{
    thread->startThread();
    DDialog::showEvent(event);
}

void ComputerPropertyDialog::closeEvent(QCloseEvent *event)
{
    thread->stopThread();
    DDialog::closeEvent(event);
}

static QString formatCap(qulonglong cap, const int size = 1024, quint8 precision = 1)
{
    static QString type[] = { " B", " KB", " MB", " GB", " TB" };

    qulonglong lc = cap;
    double dc = cap;
    double ds = size;

    for (size_t p = 0; p < sizeof(type); ++p) {
        if (cap < pow(size, p + 1) || p == sizeof(type) - 1) {
            if (!precision) {
                //! 内存总大小只能是整数所以当内存大小有小数时，就需要向上取整
                int mem = static_cast<int>(ceil(lc / pow(size, p)));
#ifdef __sw_64__
                return QString::number(mem) + type[p];
#else
                //! 如果向上取整后内存大小不为偶数，就向下取整
                if (mem % 2 > 0)
                    mem = static_cast<int>(floor(lc / pow(size, p)));
                return QString::number(mem) + type[p];
#endif
            }

            return QString::number(dc / pow(ds, p), 'f', precision) + type[p];
        }
    }

    return "";
}

ComputerInfoThread::ComputerInfoThread(QObject *parent)
    : QThread(parent)
{
}

ComputerInfoThread::~ComputerInfoThread()
{
}

void ComputerInfoThread::startThread()
{
    computerData.clear();
    threadStop = false;
    start();
}

void ComputerInfoThread::stopThread()
{
    threadStop = true;
}

void ComputerInfoThread::run()
{
    while (computerData.size() <= 1 && !threadStop) {
        computerProcess();
        sleep(1);
    }
}

/*!
 * \brief 通过DBus获取计算机信息，并保存在computerData中，
 *        判断computerData中是否数据量是大于1就把computerData通过信号发送给计算机页面
 */
void ComputerInfoThread::computerProcess()
{
    QString edition;
    QString version;
    QString memoryInstallStr;
    QString memoryStr;
    QString processor;
    QString systemType;
    qint64 tempMemory;

    qint64 memorys = UniversalUtils::computerMemory();
    if (memorys != -1)
        memoryInstallStr = formatCap(static_cast<qulonglong>(memorys), 1024, 0);

    UniversalUtils::computerInformation(processor, systemType, edition, version);

    // 如果dbus没有，则从dtk读数据
    if (DSysInfo::isDeepin()) {
        //! 获取系统版本名
        if (edition.isEmpty())
            edition = DSysInfo::uosEditionName();
        //! 获取系统版本号
        if (version.isEmpty())
            version = DSysInfo::majorVersion();
        //! 获取实际可用内存总量
        tempMemory = DSysInfo::memoryTotalSize();
        if (tempMemory >= 0)
            memoryStr = formatCap(static_cast<qulonglong>(tempMemory));
        // 如果dbus中没有获得，则从dtk获取安装的内存总量
        if (memoryInstallStr.isEmpty()) {
            tempMemory = DSysInfo::memoryInstalledSize();
            if (tempMemory >= 0)
                memoryInstallStr = formatCap(static_cast<qulonglong>(tempMemory), 1024, 0);
        }
        // 如果dbus中没有获得，则从dtk获取cpu信息
        if (processor.isEmpty())
            processor = QString("%1 x %2").arg(DSysInfo::cpuModelName()).arg(QThread::idealThreadCount());
    }
    // 通过qt获得
    if (systemType.isEmpty())
        systemType = QString::number(QSysInfo::WordSize) + tr("Bit");

    computerData.append(DSysInfo::computerName());
    if (!edition.isEmpty())
        computerData.append(edition);
    if (!version.isEmpty())
        computerData.append(version);
    if (!systemType.isEmpty())
        computerData.append(systemType);
    if (!processor.isEmpty())
        computerData.append(processor);
    if (!memoryInstallStr.isEmpty() && !memoryStr.isEmpty())
        computerData.append(memoryInstallStr + "(" + memoryStr + ' ' + QString("%1") + ")");
    if (computerData.size() > 1)
        emit sigSendComputerInfo(computerData);
}
