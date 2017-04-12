#ifndef MUSICMESSAGEVIEW_H
#define MUSICMESSAGEVIEW_H

#include <QFrame>

class QLabel;
class MusicMessageView : public QFrame
{
    Q_OBJECT
public:
    explicit MusicMessageView(const QString& uri = "", QWidget *parent = 0);
    void initUI();
    void updateElidedText();

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_uri;
    QLabel* m_titleLabel;
    QLabel* m_artistLabel;
    QLabel* m_albumLabel;
    QLabel* m_imgLabel;

    QString m_title;
    QString m_artist;
    QString m_album;
    int m_margins;
};

#endif // MUSICMESSAGEVIEW_H
