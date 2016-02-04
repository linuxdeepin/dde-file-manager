#include "growingtextedit.h"
#include <QDebug>

GrowingTextEdit::GrowingTextEdit(QWidget *parent) : QTextEdit(parent)
{
    connect(document(), SIGNAL(contentsChanged()), this, SLOT(sizeChange()));
}

QString GrowingTextEdit::getUrl(){
    return m_url;
}

void GrowingTextEdit::setUrl(QString url){
    m_url = url;
}

void GrowingTextEdit::sizeChange(){
    int docHeight = document()->size().height();
    if (docHeight >= m_minHeight && docHeight <= m_maxHeight){
       setFixedHeight(docHeight);
    }
}

void GrowingTextEdit::keyPressEvent(QKeyEvent *event){
    qDebug() << event->modifiers() << event->key();
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape){
        emit escaped(m_url);
        hide();
        return;
    }else if ((event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Return)
              || (event->modifiers() == Qt::KeypadModifier && event->key() == Qt::Key_Enter)){
        emit editedFinished();
        hide();
        return;
    }
    QTextEdit::keyPressEvent(event);
}

GrowingTextEdit::~GrowingTextEdit()
{

}

