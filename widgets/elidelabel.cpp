#include "elidelabel.h"

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
    setWordWrap(true);
    setAlignment(Qt::AlignCenter);
}

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_fullText(text)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
    setWordWrap(true);
    setAlignment(Qt::AlignCenter);
}

void ElidedLabel::setFullText(const QString &text)
{
    m_fullText = text;
    this->setText(text);
    elideText();
    if (m_simpleWrapMode){
        showSimpleWrapText();
    }else{
        showFullWrapText();
    }
}

void ElidedLabel::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void ElidedLabel::setTextLimitExpand(const QString &text)
{
    this->setMaximumWidth(this->fontMetrics().width(text));
    setFullText(text);
}

QString ElidedLabel::fullText() const
{
    return m_fullText;
}

QString ElidedLabel::getSimpWrapText(){
    return m_simpleWrapText;
}

QString ElidedLabel::getFullWrapText(){
    return m_fullWrapText;
}

QStringList ElidedLabel::getTexts(){
    return m_texts;
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resizeEvent";
    elideText();
    QLabel::resizeEvent(event);
}

void ElidedLabel::elideText()
{
    QStringList texts;
    QString fullWrapText;
    QString simpleWrapText;
    QFontMetrics fm = this->fontMetrics();
    int fmWidth = width() - 20;
    int i = 0, start = 0;

    if (fm.width(m_fullText) < fmWidth){
        texts.append(m_fullText);
    }else{
        for(i = 0; i < m_fullText.length(); i++){
            QString unitString = m_fullText.mid(start, i-start);
            if (fm.width(unitString) < fmWidth){
                continue;
            }else{
                QString retString = m_fullText.mid(start, i - start - 1);
                texts.append(retString);
                start = i -1;
            }
        }
        QString lastString = m_fullText.mid(start, m_fullText.length());
        texts.append(lastString);
    }
    fullWrapText = texts.join("\n");
    if (texts.length() >= 2){
        QString elidedText = fm.elidedText(	QStringList(texts.mid(1)).join("\n"), Qt::ElideRight, fmWidth);
        QStringList simpleList;
        simpleList << texts.at(0) << elidedText;
        simpleWrapText = simpleList.join("\n");
    }else{
        simpleWrapText = texts.at(0);
    }
    m_texts.clear();
    m_texts = texts;
    m_fullWrapText = fullWrapText;
    m_simpleWrapText = simpleWrapText;
}

bool ElidedLabel::isSimpleWrapMode(){
    return m_simpleWrapMode;
}

void ElidedLabel::setSimpleMode(bool flag){
    m_simpleWrapMode = flag;
}

void ElidedLabel::showSimpleWrapText(){
    qDebug() << "elidedlabel showSimpleWrapText start ";
    m_simpleWrapMode = true;
    QFontMetrics fm = fontMetrics();
    if (m_texts.length() == 1){
        setFixedHeight(fm.lineSpacing());
    }else{
        setFixedHeight(fm.lineSpacing() * 2);
        qDebug() << fm.lineSpacing() * 2<< "=========" << height();
    }
    qDebug() << "showSimpleWrapText" << height();
    setText(m_simpleWrapText);

    qDebug() << "elidedlabel showSimpleWrapText end ";
}


void ElidedLabel::showFullWrapText(){
    qDebug() << "elidedlabel showFullWrapText start ";
    m_simpleWrapMode = false;
    QFontMetrics fm = fontMetrics();
    setFixedHeight(fm.lineSpacing() * m_texts.length() + m_texts.length() * 2);
    setText(m_fullWrapText);
    qDebug() << "elidedlabel showFullWrapText end";
}
