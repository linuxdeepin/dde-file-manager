// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerpropertydialog.h"
#include <dfm-base/utils/universalutils.h>

#include <DSysInfo>
#include <DFontSizeManager>
#include <DFrame>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QFile>
#include <QDBusInterface>

#include <cmath>

#ifdef COMPILE_ON_V20
#    define SYSTEM_INFO_SERVICE "com.deepin.daemon.SystemInfo"
#    define SYSTEM_INFO_PATH "/com/deepin/daemon/SystemInfo"
#else
#    define SYSTEM_INFO_SERVICE "org.deepin.dde.SystemInfo1"
#    define SYSTEM_INFO_PATH "/org/deepin/dde/SystemInfo1"
#endif

static constexpr int kMaximumHeightOfTwoRow { 52 };
static constexpr int kMaximumHeightOfOneRow { 31 };

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;
ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent)
    : DDialog(parent)
{
    iniUI();
    iniThread();
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
    DLabel *titleLabel = new DLabel(tr("Computer"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    titleLabel->setForegroundRole(DPalette::TextTitle);

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
    DFontSizeManager::instance()->bind(basicInfo, DFontSizeManager::T5, QFont::DemiBold);
    basicInfo->setForegroundRole(DPalette::TextTitle);
    basicInfo->setAlignment(Qt::AlignLeft);

    computerName = new KeyValueLabel(this);
    computerName->setLeftValue(tr("Computer name"));
    computerName->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerName->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerName->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);
    computerVersionNum = new KeyValueLabel(this);
    computerVersionNum->setLeftValue(tr("Version"));
    computerVersionNum->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerVersionNum->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerVersionNum->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);
    computerEdition = new KeyValueLabel(this);
    computerEdition->setLeftValue(tr("Edition"));
    computerEdition->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerEdition->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerEdition->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);
    computerOSBuild = new KeyValueLabel(this);
    computerOSBuild->setLeftValue(tr("OS build"));
    computerOSBuild->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerOSBuild->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerOSBuild->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);
    computerType = new KeyValueLabel(this);
    computerType->setLeftValue(tr("Type"));
    computerType->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerType->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerType->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);
    computerCpu = new KeyValueLabel(this);
    computerCpu->setLeftValue(tr("Processor"));
    computerCpu->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerCpu->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerCpu->rightWidget()->setMaximumHeight(kMaximumHeightOfTwoRow);
    computerMemory = new KeyValueLabel(this);
    computerMemory->setLeftValue(tr("Memory"));
    computerMemory->setLeftFontSizeWeight(DFontSizeManager::T7, QFont::Medium);
    computerMemory->setRightFontSizeWeight(DFontSizeManager::T8, QFont::Light);
    computerMemory->rightWidget()->setMaximumHeight(kMaximumHeightOfOneRow);

    DFrame *basicInfoFrame = new DFrame(this);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 10, 10, 5);
    vlayout->setSpacing(8);
    vlayout->addWidget(basicInfo);
    vlayout->addWidget(computerName);
    vlayout->addWidget(computerVersionNum);
    vlayout->addWidget(computerEdition);
    vlayout->addWidget(computerOSBuild);
    vlayout->addWidget(computerType);
    vlayout->addWidget(computerCpu);
    vlayout->addWidget(computerMemory);
    basicInfoFrame->setLayout(vlayout);

    QFrame *contentFrame = new QFrame(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
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
    qRegisterMetaType<QMap<ComputerInfoItem, QString>>("QMap<ComputerInfoItem, QString>");
    connect(thread, &ComputerInfoThread::sigSendComputerInfo, this, &ComputerPropertyDialog::computerProcess);
}

/*!
 * \brief                   接收线程发来的获取到计算机的信息，并对信息进行设置到相应控件中
 * \param computerInfo      接收到的计算机信息
 */
void ComputerPropertyDialog::computerProcess(QMap<ComputerInfoItem, QString> computerInfo)
{
    if (computerInfo.contains(ComputerInfoItem::kName)) {
        computerName->setRightValue(computerInfo[ComputerInfoItem::kName]);
        computerName->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kVersion)) {
        computerVersionNum->setRightValue(computerInfo[ComputerInfoItem::kVersion]);
        computerVersionNum->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kEdition)) {
        computerEdition->setRightValue(computerInfo[ComputerInfoItem::kEdition]);
        computerEdition->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kOSBuild)) {
        computerOSBuild->setRightValue(computerInfo[ComputerInfoItem::kOSBuild]);
        computerOSBuild->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kType)) {
        computerType->setRightValue(computerInfo[ComputerInfoItem::kType]);
        computerType->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kCpu)) {
        computerCpu->setRightValue(computerInfo[ComputerInfoItem::kCpu]);
        computerCpu->adjustHeight();
    }
    if (computerInfo.contains(ComputerInfoItem::kMemory)) {
        computerMemory->setRightValue(computerInfo[ComputerInfoItem::kMemory]);
        computerMemory->adjustHeight();
    }
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
    QStringList type { " B", " KB", " MB", " GB", " TB" };

    qulonglong lc = cap;
    double dc = cap;
    double ds = size;

    for (int p = 0; p < type.size(); ++p) {
        if (cap < pow(size, p + 1) || p == (type.size() - 1)) {
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
    computerData.insert(ComputerInfoItem::kName, computerName());
    computerData.insert(ComputerInfoItem::kVersion, versionNum());
    computerData.insert(ComputerInfoItem::kEdition, edition());
    computerData.insert(ComputerInfoItem::kOSBuild, osBuild());
    computerData.insert(ComputerInfoItem::kType, systemType());
    computerData.insert(ComputerInfoItem::kCpu, cpuInfo());
    computerData.insert(ComputerInfoItem::kMemory, memoryInfo());
    emit sigSendComputerInfo(computerData);
}

QString ComputerInfoThread::computerName() const
{
    return DSysInfo::computerName();
}

QString ComputerInfoThread::versionNum() const
{
    return DSysInfo::majorVersion();
}

QString ComputerInfoThread::edition() const
{
    if (DSysInfo::isDeepin()) {
        if (DSysInfo::uosType() == DSysInfo::UosServer) {
            return QString("%1%2").arg(DSysInfo::minorVersion()).arg(DSysInfo::uosEditionName());
        } else {
            QString defaultEdition = QString("%1(%2)").arg(DSysInfo::uosEditionName()).arg(DSysInfo::minorVersion());
            if (DSysInfo::UosEdition::UosProfessional == DSysInfo::uosEditionType()
                || DSysInfo::UosEdition::UosMilitary == DSysInfo::uosEditionType()) {
                QDBusInterface deepinLicenseInfo("com.deepin.license",
                                                 "/com/deepin/license/Info",
                                                 "com.deepin.license.Info",
                                                 QDBusConnection::systemBus());
                deepinLicenseInfo.setTimeout(1000);
                if (!deepinLicenseInfo.isValid()) {
                    fmWarning() << "Dbus com.deepin.license is not valid!";
                    return defaultEdition;
                }
                fmInfo() << "Start call Dbus com.deepin.license AuthorizationState";
                int activeInfo = deepinLicenseInfo.property("AuthorizationState").toInt();
                fmInfo() << "End call Dbus com.deepin.license AuthorizationState";
                if (kActivated == activeInfo) {
                    fmInfo() << "Start call Dbus com.deepin.license ServiceProperty";
                    QVariant info = deepinLicenseInfo.property("ServiceProperty");
                    fmInfo() << "End call Dbus com.deepin.license serviceProperty";
                    if (info.isValid() && info.toUInt() == kSecretsSecurity) {
                        return QString("%1(%2)(%3)").arg(DSysInfo::uosEditionName()).arg(tr("For Secrets Security")).arg(DSysInfo::minorVersion());
                    }

                    fmInfo() << "Start call Dbus com.deepin.license AuthorizationProperty";
                    uint authorizedInfo = deepinLicenseInfo.property("AuthorizationProperty").toUInt();
                    fmInfo() << "End call Dbus com.deepin.license AuthorizationProperty";
                    if (kGovernment == authorizedInfo) {
                        return QString("%1(%2)(%3)").arg(DSysInfo::uosEditionName()).arg(tr("For Government")).arg(DSysInfo::minorVersion());
                    } else if (kEnterprise == authorizedInfo) {
                        return QString("%1(%2)(%3)").arg(DSysInfo::uosEditionName()).arg(tr("For Enterprise")).arg(DSysInfo::minorVersion());
                    } else if (kUnauthorized == authorizedInfo) {
                        return defaultEdition;
                    } else {
                        const QString info = deepinLicenseInfo.property("AuthorizationPropertyString").toString();
                        if (!info.isEmpty())
                            return QString("%1(%2)(%3)").arg(DSysInfo::uosEditionName()).arg(info).arg(DSysInfo::minorVersion());
                        return defaultEdition;
                    }
                } else {
                    return defaultEdition;
                }
            } else {
                return defaultEdition;
            }
        }
    } else {
        return QString("%1 %2").arg(DSysInfo::productVersion()).arg(DSysInfo::productTypeString());
    }
}

QString ComputerInfoThread::osBuild() const
{
    return DSysInfo::buildVersion();
}

QString ComputerInfoThread::systemType() const
{
    return QString::number(QSysInfo::WordSize) + tr("Bit");
}

QString ComputerInfoThread::cpuInfo() const
{
    // 1. 如果CPU型号名称中已包含频率信息，直接返回
    QString modelName = DSysInfo::cpuModelName();
    if (modelName.contains("Hz"))
        return modelName;

    // 2. 获取处理器基本信息
    QString processor = modelName.isEmpty() ? QObject::tr("Unknow") : modelName;
    fmInfo("Start call Dbus %s...", SYSTEM_INFO_SERVICE);
    QDBusInterface interface(SYSTEM_INFO_SERVICE,
                             SYSTEM_INFO_PATH,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::sessionBus());
    interface.setTimeout(1000);

    if (!interface.isValid()) {
        fmWarning() << QString("Dbus %1 is not valid!").arg(SYSTEM_INFO_SERVICE);
        return processor;
    }

    // 3. 从DBus获取处理器信息
    QDBusMessage msgCpuInfo = interface.call("Get", SYSTEM_INFO_SERVICE, "Processor");
    QList<QVariant> argsCpuInfo = msgCpuInfo.arguments();
    if (argsCpuInfo.count() > 0) {
        processor = argsCpuInfo.at(0).value<QDBusVariant>().variant().toString();
        if (processor.contains("Hz"))
            return processor;
    }

    // 4. 获取CPU频率
    QString freqProperty = processor.contains("PANGU") ? "CPUMaxMHz" : "CurrentSpeed";
    QDBusMessage msgFreq = interface.call("Get", SYSTEM_INFO_SERVICE, freqProperty);
    QList<QVariant> argsFreq = msgFreq.arguments();

    double cpuGHz = 0.0;
    if (argsFreq.count() > 0) {
        double cpuMhz = argsFreq.at(0).value<QDBusVariant>().variant().toDouble();
        cpuGHz = cpuMhz / 1000.0;   // 转换为GHz
    }

    // 5. 构建最终的CPU信息字符串
    QString actualProcessor = DSysInfo::cpuModelName().isEmpty() ? processor : DSysInfo::cpuModelName();
    if (cpuGHz <= 0.0)
        return actualProcessor;

    return QString("%1 @ %2GHz").arg(actualProcessor).arg(QString::number(cpuGHz, 'f', 2));
}

QString ComputerInfoThread::memoryInfo() const
{
    return QString("%1 (%2 %3)")
            .arg(formatCap(DSysInfo::memoryInstalledSize(), 1024, 0))
            .arg(formatCap(DSysInfo::memoryTotalSize()))
            .arg(tr("Available"));
}
