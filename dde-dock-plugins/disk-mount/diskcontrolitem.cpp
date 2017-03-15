#include "diskcontrolitem.h"
#include "qdiskinfo.h"
#include <QVBoxLayout>
#include <QIcon>
#include <QtMath>

DWIDGET_USE_NAMESPACE

DiskControlItem::DiskControlItem(const QDiskInfo &info, QWidget *parent)
    : QFrame(parent),

      m_unknowIcon(":/icons/resources/unknown.svg"),

      m_diskIcon(new QLabel),
      m_diskName(new QLabel),
      m_diskCapacity(new QLabel),
      m_capacityValueBar(new QProgressBar),
      m_unmountButton(new DImageButton)
{
//    QIcon::setThemeName("deepin");

    m_diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_diskName->setStyleSheet("color:white;");

    m_diskCapacity->setStyleSheet("color:rgba(255, 255, 255, .6);");

    m_capacityValueBar->setTextVisible(false);
    m_capacityValueBar->setFixedHeight(2);
    m_capacityValueBar->setStyleSheet("QProgressBar {"
                                      "border:none;"
                                      "background-color:rgba(255, 255, 255, .1);"
                                      "}"
                                      "QProgressBar::chunk {"
                                      "background-color:rgba(255, 255, 255, .8);"
                                      "}");

    m_unmountButton->setNormalPic(":/icons/resources/unmount-normal.png");
    m_unmountButton->setHoverPic(":/icons/resources/unmount-hover.png");
    m_unmountButton->setPressPic(":/icons/resources/unmount-press.png");
    m_unmountButton->setStyleSheet("margin-top:12px;");

    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_diskName);
    infoLayout->addWidget(m_diskCapacity);
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(3, 6, 0, 8);

    QHBoxLayout *unmountLayout = new QHBoxLayout;
    unmountLayout->addLayout(infoLayout);
    unmountLayout->addWidget(m_unmountButton);
    unmountLayout->setSpacing(0);
    unmountLayout->setMargin(0);

    QVBoxLayout *progressLayout = new QVBoxLayout;
    progressLayout->addLayout(unmountLayout);
    progressLayout->addWidget(m_capacityValueBar);
    progressLayout->setSpacing(0);
    progressLayout->setContentsMargins(10, 0, 0, 5);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addWidget(m_diskIcon);
    centralLayout->addLayout(progressLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 5, 0);

    setLayout(centralLayout);
    setObjectName("DiskItem");
    setStyleSheet("QFrame #DiskItem:hover {"
                  "background-color:rgba(255, 255, 255, .1);"
                  "border-radius:4px;"
                  "}");

    connect(m_unmountButton, &DImageButton::clicked, [this] {emit requestUnmount(m_info.id());});

    updateInfo(info);
}

void DiskControlItem::updateInfo(const QDiskInfo &info)
{
    m_info = info;

    m_diskIcon->setPixmap(QIcon::fromTheme(info.iconName(), m_unknowIcon).pixmap(48, 48));
    if (!info.name().isEmpty())
        m_diskName->setText(info.name());
    else
        m_diskName->setText(tr("Unknown device"));
    if (info.total())
        m_diskCapacity->setText(QString("%1/%2").arg(formatDiskSize(info.used())).arg(formatDiskSize(info.total())));
    else if (info.name().isEmpty())
        m_diskCapacity->clear();
    else
        m_diskCapacity->setText(tr("Unknown volume"));
    m_capacityValueBar->setMinimum(0);
    m_capacityValueBar->setMaximum(std::max(1ull, info.total()));
    m_capacityValueBar->setValue(info.used());
}

QString DiskControlItem::sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

qreal DiskControlItem::dRound64(qreal num, int count)
{
    if (count <= 0)
        return qRound64(num);

    qreal base = qPow(10, count);

    return qRound64(num * base) / base;
}


const QString DiskControlItem::formatDiskSize(const quint64 num) const
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) {
        total = QString( "%1 TB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / tb), 'f', 1 )) );
    } else if( num >= gb ) {
        total = QString( "%1 GB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / gb), 'f', 1 )) );
    } else if( num >= mb ) {
        total = QString( "%1 MB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / mb), 'f', 1 )) );
    } else if( num >= kb ) {
        total = QString( "%1 KB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / kb),'f',1 )) );
    } else {
        total = QString( "%1 B" ).arg( num );
    }

    return total;
}
