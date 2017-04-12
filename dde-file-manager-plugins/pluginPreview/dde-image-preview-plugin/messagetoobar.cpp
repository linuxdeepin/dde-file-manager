#include "messagetoobar.h"
#include <QFileInfo>
#include <QLabel>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImage>
#include <QResizeEvent>

MessageTooBar::MessageTooBar(const QString& file, QWidget *parent) :
    QFrame(parent)
{

    QFileInfo info(file);
    m_fileName = info.fileName();
    QImageReader reader(file);
    QImage img = reader.read();
    m_imgSize = QString("%1x%2").arg(QString::number(img.width()), QString::number(img.height()));

    initUI();
}

void MessageTooBar::initUI()
{
    setContentsMargins(0, 0, 0, 0);

    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setText(m_fileName);

    m_sizeLabel = new QLabel(this);
    m_sizeLabel->setText(m_imgSize);
    m_sizeLabel->setStyleSheet("QLabel{"
                                "color: #545454;"
                             "}");

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addSpacing(5);
    layout->addWidget(m_fileNameLabel);
    layout->addStretch();


    setLayout(layout);
}

void MessageTooBar::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    QFont font;
    font.setPixelSize(12);
    QFontMetrics fm(font);

    int sizeLabelWidth = fm.width(m_imgSize);
    m_sizeLabel->move((event->size().width() - sizeLabelWidth) / 2, -3);

    int maxTextWidth = m_sizeLabel->x() - 20;
    QString fileNameDisplayText = fm.elidedText(m_fileName, Qt::ElideRight, maxTextWidth);
    m_fileNameLabel->setText(fileNameDisplayText);
}
