#ifndef MESSAGETOOBAR_H
#define MESSAGETOOBAR_H

#include <QFrame>

class QLabel;
class MessageTooBar : public QFrame
{
    Q_OBJECT
public:
    explicit MessageTooBar(const QString& file = "", QWidget *parent = 0);

    void initUI();

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_fileName;
    QString m_imgSize;

    QLabel* m_sizeLabel;
    QLabel* m_fileNameLabel;
};

#endif // MESSAGETOOBAR_H
