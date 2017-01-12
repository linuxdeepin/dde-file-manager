#include "computerpropertydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DTitlebar>
#include <QGridLayout>
#include <QPixmap>
#include "dplatformwindowhandle.h"
#include <QProcess>
#include <QDebug>
DWIDGET_USE_NAMESPACE

ComputerPropertyDialog::ComputerPropertyDialog(QWidget *parent) : QDialog(parent)
{
    DPlatformWindowHandle handle(this);
    Q_UNUSED(handle);
    initUI();
}

void ComputerPropertyDialog::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint);
    DTitlebar* titleBar = new DTitlebar(this);
    titleBar->setWindowFlags(Qt::WindowCloseButtonHint);
    titleBar->setFixedHeight(20);

    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/images/dialogs/images/deepin_logo.png"));
    QLabel* nameLabel = new QLabel(tr("Computer"), this);
    nameLabel->setObjectName("NameLabel");

    QLabel* lineLabel = new QLabel(this);
    lineLabel->setObjectName("Line");
    lineLabel->setFixedSize(300, 2);
    lineLabel->setStyleSheet("QLabel#Line{"
                                "border: none;"
                                "background-color: #f0f0f0;"
                             "}");

    QLabel* messageLabel = new QLabel(tr("Basic Info"), this);
    messageLabel->setObjectName("NameLabel");

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setColumnMinimumWidth(0, 100);
    gridLayout->setColumnMinimumWidth(1, 200);
    gridLayout->setSpacing(10);

    QStringList msgsTitle;
    msgsTitle << tr("Computer Name")
         << tr("Brand")
         << tr("Type")
         << tr("Processor")
         << tr("Memory")
         << tr("Disk");

    int row = 0;
    QHash<QString, QString> datas = getMessage(msgsTitle);

    foreach (const QString& key, msgsTitle) {
        QLabel* keyLabel = new QLabel(key,this);
        keyLabel->setObjectName("KeyLabel");
        QLabel* valLabel = new QLabel(datas.value(key), this);
        valLabel->setObjectName("ValLabel");
        valLabel->setWordWrap(true);

        gridLayout->addWidget(keyLabel,row, 0, Qt::AlignRight|Qt::AlignTop);
        gridLayout->addWidget(valLabel,row, 1, Qt::AlignLeft|Qt::AlignTop);
        gridLayout->setRowMinimumHeight(row, 12);
        row ++;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(titleBar);
    mainLayout->addSpacing(80);
    mainLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(60);
    mainLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(lineLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(5);
    QHBoxLayout* messageLayout = new QHBoxLayout;
    messageLayout->setSpacing(0);
    messageLayout->addSpacing(30);
    messageLayout->addWidget(messageLabel);
    mainLayout->addLayout(messageLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch(1);

    setFixedSize(320, 460);
    setLayout(mainLayout);

    setStyleSheet("QLabel#NameLabel{"
                    "font-size: 12px;"
                  "}"
                  "QLabel#KeyLabel{"
                    "font-size: 11px;"
                    "color: #777777;"
                  "}"
                  "QLabel#ValLabel{"
                    "font-size:11px;"
                   "}");
}

QHash<QString, QString> ComputerPropertyDialog::getMessage(const QStringList& data)
{
    QHash<QString, QString> datas;
    datas.insert(data.at(0),getComputerName());
    datas.insert(data.at(1),getBrand());
    datas.insert(data.at(2),getArch());
    datas.insert(data.at(3),getProcessor());
    datas.insert(data.at(4),getMemory());
    datas.insert(data.at(5),getDisk());
    return datas;
}

void ComputerPropertyDialog::closeEvent(QCloseEvent *e)
{
    emit closed();
}

QString ComputerPropertyDialog::getComputerName()
{
    QString cmd = "hostname";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result = p.readLine();
    return result.trimmed();
}

QString ComputerPropertyDialog::getBrand()
{
    QString cmd = "cat";
    QStringList args;
    args << "/etc/deepin-version";
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result = "deepin ";
    QString local = QLocale::system().name();
    while(p.canReadLine()){
        QString str = p.readLine();
        str = str.trimmed();
        if(str.contains("Version")){
            result = result + str.split("=").last() + " ";
        }

        if(local == "zh_CN"){
            if(str.contains("Type[zh_CN]")){
                result = result + str.split("=").last() + " ";
            }
        } else{
            if(str.contains("Type")){
                result = result + str.split("=").last() + " ";
            }
        }

    }
    return result.trimmed();
}

QString ComputerPropertyDialog::getArch()
{
    QString cmd = "arch";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result = p.readLine();
    result = result.trimmed();
    return (result.right(2) + tr("Bit"));
}

QString ComputerPropertyDialog::getProcessor()
{
    QString cmd = "lscpu";
    QStringList args;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    QString result;
    while (p.canReadLine()) {
        result = p.readLine();
        if(result.contains("Model name:")){
            result.remove("Model name:");
            QStringList datas = result.split(" ");
            result = "";
            for(int i = 0; i < datas.count(); i++){
                if(datas.at(i) != ""){
                    result += datas.at(i) + " ";
                }
            }
            break;
        }
    }
    cmd = "nproc";
    QProcess nproc;
    nproc.start(cmd, args);
    nproc.waitForFinished(-1);
    result = result.trimmed();
    result = result + " X" + nproc.readLine();
    return result.trimmed();
}

QString ComputerPropertyDialog::getMemory()
{
    QString cmd = "free";
    QProcess p;
    p.start(cmd, (QStringList() << "-h"));
    p.waitForFinished(-1);
    p.readLine();
    QString str = p.readLine();
    QStringList datas = str.split(" ");
    for(int i = 0; i < datas.length(); i++){
        if(datas.at(i) == ""){
            datas.removeAt(i);
            i--;
        }
    }
    QString result = datas.at(1);
    return result.trimmed();
}

QString ComputerPropertyDialog::getDisk()
{
    QString cmd = "lsblk";
    QStringList args;
    args << "/dev/sda" << "--output=size";
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished(-1);
    p.readLine();
    QString result = p.readLine();
    return result.trimmed();
}
