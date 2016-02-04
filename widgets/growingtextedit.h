#ifndef GROWINGTEXTEDIT_H
#define GROWINGTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>

class GrowingTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit GrowingTextEdit(QWidget *parent = 0);
    ~GrowingTextEdit();

    QString getUrl();

signals:
    void escaped(QString url);
    void editedFinished();

public slots:
    void sizeChange();
    void setUrl(QString url);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    int m_minHeight = 0;
    int m_maxHeight = 6000;
    QString m_url;
};

#endif // GROWINGTEXTEDIT_H
