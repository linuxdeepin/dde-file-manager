// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerpropertydialog.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "dbus/dbussysteminfo.h"
#include "shutil/fileutils.h"

#include <DPlatformWindowHandle>
#include <DSpinner>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DSysInfo>
#include <DTitlebar>
#include <QGridLayout>
#include <QPixmap>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QPainter>
#include <DGraphicsClipEffect>
#include <QWindow>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static QString formatCap(qulonglong cap, const int size = 1024, quint8 precision = 1)
{
    static QString type[] = {" B", " KB", " MB", " GB", " TB"};

    qulonglong lc = cap;
    double dc = cap;
    double ds = size;

    for (size_t p = 0; p < sizeof(type); ++p) {
        if (cap < pow(size, p + 1) || p == sizeof(type) - 1) {
            if (!precision) {
                //! 内存总大小只能是整数所以当内存大小有小数时，就需要向上取整
                int mem = ceil(lc / pow(size, p));
#ifdef __sw_64__
                return QString::number(mem) + type[p];
#else
                //! 如果向上取整后内存大小不为偶数，就向下取整
                if(mem % 2 > 0)
                    mem = floor(lc / pow(size, p));
                return QString::number(mem) + type[p];
#endif
            }

            return QString::number(dc / pow(ds, p), 'f', precision) + type[p];
        }
    }

    return "";
}

static QString editionInfo()
{
    QString edition {""};
    // 专业版
    if (DSysInfo::UosEdition::UosProfessional == DSysInfo::uosEditionType()) {
        // 是否激活
        QDBusInterface deepinLicenseInfo("com.deepin.license",
                                         "/com/deepin/license/Info",
                                         "com.deepin.license.Info",
                                         QDBusConnection::systemBus());
        deepinLicenseInfo.setTimeout(1000);
        if (!deepinLicenseInfo.isValid()) {
            qWarning() << "Dbus com.deepin.license is not valid";
            return "";
        }
        qInfo() << "Start call Dbus com.deepin.license AuthorizationState";
        int activeInfo = deepinLicenseInfo.property("AuthorizationState").toInt();
        qInfo() << "End call Dbus com.deepin.license AuthorizationState";
        if (activeInfo == 1) {
            // 政务授权、企业授权、原始授权
            qInfo() << "Start call Dbus com.deepin.license AuthorizationProperty";
            uint authorizedInfo = deepinLicenseInfo.property("AuthorizationProperty").toUInt();
            qInfo() << "End call Dbus com.deepin.license AuthorizationProperty";
            if (authorizedInfo == 1) {
                edition = DSysInfo::uosEditionName() + "(" + QObject::tr("For Government") + ")" + "(" + DSysInfo::minorVersion() + ")";
            } else if (authorizedInfo == 2) {
                edition = DSysInfo::uosEditionName() + "(" + QObject::tr("For Enterprise") + ")" + "(" + DSysInfo::minorVersion() + ")";
            } else {
                edition = DSysInfo::uosEditionName() + "(" + DSysInfo::minorVersion() + ")";
            }
        } else {
            edition = DSysInfo::uosEditionName() + "(" + DSysInfo::minorVersion() + ")";
        }
    } else {
        edition = DSysInfo::uosEditionName() + "(" + DSysInfo::minorVersion() + ")";
    }
    return edition;
}

GetInfoWork::GetInfoWork(QObject *parent)
    : QThread(parent)
{
}

void GetInfoWork::setInitData(QList<QString> datas)
{
    m_datas = datas;
}

void GetInfoWork::startWork()
{
    m_bStop = false;
    start();
}

void GetInfoWork::stopWork()
{
    m_bStop = true;
}

void GetInfoWork::run()
{
    // 设置初始值
    DBusSystemInfo *systemInfo = nullptr;
    QDBusInterface *deepin_systemInfo = nullptr;
    QString computerName("");
    QString keyName = qApp->translate("ComputerPropertyDialog", "Computer name");
    QString Edition("");
    QString keyEditon = qApp->translate("ComputerPropertyDialog", "Edition");
    QString version("");
    QString keyVersion = qApp->translate("ComputerPropertyDialog", "Version");
    QString systemType("");
    QString keyType = qApp->translate("ComputerPropertyDialog", "Type");
    QString processor("");
    QString keyProcessor = qApp->translate("ComputerPropertyDialog", "Processor");
    QString memoryInstallStr("");
    QString memoryStr("");
    QString keyMemory = qApp->translate("ComputerPropertyDialog", "Memory");
    qint64 tempMemory;
    QMap<QString, QString> mapDatas;

    while (!m_datas.isEmpty() && !m_bStop) {
        // 创建dbus对象
        systemInfo = new DBusSystemInfo();
        // 从com.deepin.system.SystemInfo中获取实际安装的内存的大小
        deepin_systemInfo = new QDBusInterface("com.deepin.system.SystemInfo",
                                               "/com/deepin/system/SystemInfo",
                                               "com.deepin.system.SystemInfo",
                                               QDBusConnection::systemBus(), this);

        // 部分数据优先从dbus读取
        if (deepin_systemInfo) {
            // 获取安装内存
            if (m_datas.contains(keyMemory))
                memoryInstallStr = formatCap(deepin_systemInfo->property("MemorySize").toULongLong(), 1024, 0);
        }
        if (systemInfo->isValid()) {
            // 获取cpu信息
            if (m_datas.contains(keyProcessor))
                processor = systemInfo->processor();
            // 获取系统是64位还是32位
            if (m_datas.contains(keyType))
                systemType = QString::number(systemInfo->systemType()) + qApp->translate("ComputerPropertyDialog", "Bit");
            // 获取系统版本信息
            if (m_datas.contains(keyEditon) || m_datas.contains(keyVersion)) {
                if (Edition.isEmpty()) {
                    Edition = systemInfo->version();
                    QStringList temp = Edition.split(' ');
                    if (temp.size() > 1) {
                        version = temp[0];
                        Edition = temp[1];
                    } else if (!temp.isEmpty()) {
                        Edition = temp[0];
                    }
                }
            }
        }
        // 如果dbus没有，则从dtk读数据
        if (DSysInfo::isDeepin()) {
            // 获取计算机名称
            if (m_datas.contains(keyName)) {
                computerName = DSysInfo::computerName();
                // 因为用户可以修改计算机名称，所以检测到名称变化后，立刻发送出去
                if (!computerName.isEmpty()) {
                   mapDatas.insert(keyName, computerName);
                   emit sigSendInfo(mapDatas);
                   m_datas.removeAll(keyName);
                   mapDatas.clear();
                }
            }
            // 获取系统版本名
            if (m_datas.contains(keyEditon)) {
                if (DSysInfo::UosType::UosServer == DSysInfo::uosType()) {  // 服务器版本
                    Edition = DSysInfo::minorVersion() + DSysInfo::uosEditionName();
                } else {
                    Edition = editionInfo();
                    if (Edition.isEmpty())
                        qWarning() << "get edition info failed";
                }
            }
            // 获取系统版本号
            if (m_datas.contains(keyVersion))
                version = DSysInfo::majorVersion();
            if (m_datas.contains(keyMemory)) {
                // 获取实际可用内存总量
                tempMemory = DSysInfo::memoryTotalSize();
                if (tempMemory >= 0)
                    memoryStr = formatCap(static_cast<qulonglong>(tempMemory));
            }
            if (m_datas.contains(keyMemory) && memoryInstallStr.isEmpty()) {
                // 如果没有通过dbus获取安装内存，则通过dtk获取
                tempMemory = DSysInfo::memoryInstalledSize();
                if (tempMemory >= 0)
                    memoryInstallStr = formatCap(static_cast<qulonglong>(tempMemory), 1024, 0);
            }
            if (m_datas.contains(keyProcessor) && processor.isEmpty()) {
                // 如果没有用过dbus获取CPU信息，则通过dtk获取
                processor = QString("%1 x %2").arg(DSysInfo::cpuModelName())
                                              .arg(QThread::idealThreadCount());
            }

        }
        // 通过qt获得
        if (systemType.isEmpty()) {
            systemType = QString::number(QSysInfo::WordSize) + qApp->translate("ComputerPropertyDialog", "Bit");
        }

        // 对外发送属性更新信号
        mapDatas.clear();
        if (m_datas.contains(keyEditon) && !Edition.isEmpty()) {
            mapDatas.insert(keyEditon, Edition);
            m_datas.removeAll(keyEditon);
        }
        if (m_datas.contains(keyVersion) && !version.isEmpty()) {
            mapDatas.insert(keyVersion, version);
            m_datas.removeAll(keyVersion);
        }
        if (m_datas.contains(keyType) && !systemType.isEmpty()) {
            mapDatas.insert(keyType, systemType);
            m_datas.removeAll(keyType);
        }
        if (m_datas.contains(keyProcessor) && !processor.isEmpty()) {
            mapDatas.insert(keyProcessor, processor);
            m_datas.removeAll(keyProcessor);
        }
        if (m_datas.contains(keyMemory) && !memoryInstallStr.isEmpty() && !memoryStr.isEmpty()) {
            QString strMemory(memoryInstallStr + "(" +  memoryStr + ' ' + qApp->translate("ComputerPropertyDialog", "Available") + ")");
            mapDatas.insert(keyMemory, strMemory);
            m_datas.removeAll(keyMemory);
        }
        if (!mapDatas.isEmpty())
            emit sigSendInfo(mapDatas);
        // 销毁dbus对象
        if (systemInfo) {
            delete systemInfo;
            systemInfo = nullptr;
        }
        if (deepin_systemInfo) {
            delete deepin_systemInfo;
            deepin_systemInfo = nullptr;
        }
        // 暂停1s
        QThread::sleep(1);
    }
}

ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent) : DDialog(parent)
{
    m_systemInfo = new DBusSystemInfo();
    initUI();
}

ComputerPropertyDialog::~ComputerPropertyDialog()
{
    // 安全退出线程
    if (m_getInfoWork && m_getInfoWork->isRunning()) {
        m_getInfoWork->stopWork();
        m_getInfoWork->wait();
    }
}

void ComputerPropertyDialog::initUI()
{
    QLabel *iconLabel = new QLabel(this);

    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    QString distributerLogoPath = DSysInfo::distributionOrgLogo();
    QIcon logoIcon;
    if (!distributerLogoPath.isEmpty() && QFile::exists(distributerLogoPath)) {
        logoIcon = QIcon(distributerLogoPath);
    } else {
        logoIcon = QIcon::fromTheme("dfm_deepin_logo");
    }

    iconLabel->setPixmap(logoIcon.pixmap(152, 39));
    QLabel *nameLabel = new QLabel(tr("Computer"), this);
    auto pt = nameLabel->palette();
    pt.setColor(QPalette::Text, palette().color(QPalette::Normal, QPalette::Text));
    auto font = nameLabel->font();
    font.setBold(true);
    font.setPixelSize(17);
    nameLabel->setFont(font);
    nameLabel->setPalette(pt);

    QLabel *messageLabel = new QLabel(tr("Basic Info"), this);
    messageLabel->setPalette(pt);
    messageLabel->setFont(font);


    QGridLayout *gridLayout = new QGridLayout;
    //gridLayout->setColumnMinimumWidth(0, 100);
    gridLayout->setColumnMinimumWidth(1, 170);
    gridLayout->setSpacing(10);

    QStringList msgsTitle;
    msgsTitle << tr("Computer name")
              << tr("Version")
              << tr("Edition")
              << tr("OS build")
              << tr("Type")
              << tr("Processor")
              << tr("Memory");

    int row = 0;
    QHash<QString, QString> datas = getMessage(msgsTitle);

    foreach (const QString &key, msgsTitle) {

        if (DSysInfo::isCommunityEdition() && key == tr("Version"))
            continue;

        QLabel *keyLabel = new QLabel(key, this);
        QLabel *valLabel = new QLabel(datas.value(key), this);
        // 缓存值label,便于刷新属性值
        m_mapItems.insert(key, valLabel);

        valLabel->setTextFormat(Qt::PlainText);
        valLabel->setWordWrap(true);

        font = keyLabel->font();
        font.setPixelSize(13);
        keyLabel->setFont(font);

        font = valLabel->font();
        font.setPixelSize(12);
        valLabel->setFont(font);

        pt = keyLabel->palette();
        pt.setColor(QPalette::Text, palette().color(QPalette::Normal, QPalette::HighlightedText));
        keyLabel->setPalette(pt);

        pt = valLabel->palette();
        pt.setColor(QPalette::Text, palette().color(QPalette::Inactive, QPalette::Text));
        valLabel->setPalette(pt);

        gridLayout->addWidget(keyLabel, row, 0);
        gridLayout->addWidget(valLabel, row, 1);
        //! 使内存显示不换行
        if (key == tr("Memory"))
            valLabel->setWordWrap(false);
        //CPU字符匹配，因为CPU长度不定长在计算中gridLayout会产生错误
        if (key == tr("Processor")) {
            auto boundingRect = valLabel->fontMetrics().boundingRect(valLabel->text());
            gridLayout->setRowMinimumHeight(row, boundingRect.height() - boundingRect.y());
        }
        // 如果没有拿到对应属性值
        if (datas.value(key).isEmpty()
                && key != tr("OS build")) {
           // 隐藏属性视图
           valLabel->hide();
           // 创建并显示等待视图
           QFrame *frameWait = new QFrame(this);
           QHBoxLayout *lay = new QHBoxLayout;
           lay->setContentsMargins(0, 0, 0, 0);
           lay->setSpacing(10);
           // 创建等待圈控件
           DSpinner *spinWait = new DSpinner(frameWait);
           spinWait->setFixedSize(16, 16);
           spinWait->start();
           lay->addWidget(spinWait);
           // 创建等待label控件
           QLabel *labelWait = new QLabel(tr("Obtaining..."), frameWait);
           font = labelWait->font();
           font.setPixelSize(12);
           labelWait->setFont(font);
           pt = labelWait->palette();
           pt.setColor(QPalette::Text, palette().color(QPalette::Inactive, QPalette::Text));
           labelWait->setPalette(pt);
           lay->addWidget(labelWait);
           // 布局
           frameWait->setLayout(lay);
           gridLayout->addWidget(frameWait, row, 1);
           // 缓存第一次没有获取到的属性
           m_mapNotFinish.insert(key, frameWait);
        }
        row++;
    }

    QFrame *contentFrame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(21);

    QFrame *baseInfoFrame = new QFrame;
    QString backColor = palette().color(QPalette::Base).name();
    baseInfoFrame->setStyleSheet(QString("background-color: %1; border-radius: 8px;").arg(backColor));
    QVBoxLayout *baseInfoLayout = new QVBoxLayout;
    baseInfoFrame->setLayout(baseInfoLayout);
    baseInfoLayout->addWidget(messageLabel);
    baseInfoLayout->addSpacing(10);
    baseInfoLayout->setContentsMargins(10, 5, 10, 5);
    baseInfoLayout->addLayout(gridLayout);

    mainLayout->addWidget(baseInfoFrame);

    setFixedWidth(320);
    contentFrame->setLayout(mainLayout);

    addContent(contentFrame);

    // 启动子线程，获取并更新当前未拿到的属性信息
    if (!m_mapNotFinish.isEmpty() && !m_getInfoWork) {
        m_getInfoWork = new GetInfoWork(this);
        connect(m_getInfoWork, &GetInfoWork::sigSendInfo,
                this, &ComputerPropertyDialog::slotSetInfo);
        m_getInfoWork->setInitData(m_mapNotFinish.keys());
        m_getInfoWork->startWork();
    }
}

QHash<QString, QString> ComputerPropertyDialog::getMessage(const QStringList &data)
{
    QHash<QString, QString> datas;
    QString version;
    QString Edition;
    QString memoryInstallStr;
    QString memoryStr;
    QString processor;
    QString systemType;
    qint64 tempMemory;

    //! 从com.deepin.system.SystemInfo中获取实际安装的内存的大小
    QDBusInterface *deepin_systemInfo = new QDBusInterface("com.deepin.system.SystemInfo",
                                                           "/com/deepin/system/SystemInfo",
                                                           "com.deepin.system.SystemInfo",
                                                           QDBusConnection::systemBus(), this);

    // 部分数据优先从dbus读取
    // 获取安装的内存总量
    if (deepin_systemInfo)
        memoryInstallStr = formatCap(deepin_systemInfo->property("MemorySize").toULongLong(), 1024, 0);
    if (m_systemInfo->isValid()) {
        //! 获取cpu信息
        processor = m_systemInfo->processor();
        //! 获取系统是64位还是32位
        systemType = QString::number(m_systemInfo->systemType()) + tr("Bit");

        if (Edition.isEmpty()) {
            Edition = m_systemInfo->version();
            QStringList temp = Edition.split(' ');
            if (temp.size() > 1) {
                version = temp[0];
                Edition = temp[1];
            } else if (!temp.isEmpty()) {
                Edition = temp[0];
            }
        }
    }
    // 如果dbus没有，则从dtk读数据
    if (DSysInfo::isDeepin()) {
        //! 获取系统版本名
        if (DSysInfo::UosType::UosServer == DSysInfo::uosType()) {  // 服务器版本
            Edition = DSysInfo::minorVersion() + DSysInfo::uosEditionName();
        } else {
            Edition = editionInfo();
            if (Edition.isEmpty())
                qWarning() << "get edition info failed";
        }
        //! 获取系统版本号
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
            processor = QString("%1 x %2").arg(DSysInfo::cpuModelName())
                                         .arg(QThread::idealThreadCount());
    }
    // 通过qt获得
    if (systemType.isEmpty())
        systemType = QString::number(QSysInfo::WordSize) + tr("Bit");

    datas.insert(data.at(0), DSysInfo::computerName());
    datas.insert(data.at(1), version);
    datas.insert(data.at(2), Edition);
    datas.insert(data.at(3), DSysInfo::buildVersion());
    datas.insert(data.at(4), systemType);
    datas.insert(data.at(5), processor);
    if (!memoryInstallStr.isEmpty() && !memoryStr.isEmpty())
        datas.insert(data.at(6), memoryInstallStr
                     + "(" +  memoryStr
                     + ' ' + tr("Available") + ")");
    else
        datas.insert(data.at(6), "");
    return datas;
}

void ComputerPropertyDialog::updateComputerInfo()
{
    // 如果线程对象存在
    if (m_getInfoWork) {
        if (m_getInfoWork->isRunning()) {
            m_getInfoWork->stopWork();
            m_getInfoWork->wait();
        }
    } else {
        m_getInfoWork = new GetInfoWork(this);
        connect(m_getInfoWork, &GetInfoWork::sigSendInfo,
                this, &ComputerPropertyDialog::slotSetInfo);
    }
    // 设置所有属性项都需要更新
    m_getInfoWork->setInitData(m_mapItems.keys());
    m_getInfoWork->startWork();
}

void ComputerPropertyDialog::slotSetInfo(QMap<QString, QString> mapNewDatas)
{
    QMap<QString, QString>::iterator itr = mapNewDatas.begin();
    for (; itr != mapNewDatas.end(); ++itr) {
        QString strItemKey = itr.key();
        // 销毁等待控件
        if (m_mapNotFinish.contains(strItemKey)) {
            delete m_mapNotFinish[strItemKey];
            m_mapNotFinish.remove(strItemKey);
        }
        // 更新计算机信息
        if (m_mapItems.contains(strItemKey)) {
            m_mapItems[strItemKey]->setText(itr.value());
            m_mapItems[strItemKey]->show();
        }
    }
    adjustSize();
}

void ComputerPropertyDialog::hideEvent(QHideEvent *event)
{
    // 结束子线程
    if (m_getInfoWork) {
        if (m_getInfoWork->isRunning()) {
            m_getInfoWork->stopWork();
            m_getInfoWork->wait();
        }
        delete m_getInfoWork;
        m_getInfoWork = nullptr;
    }
    DDialog::hideEvent(event);
}
