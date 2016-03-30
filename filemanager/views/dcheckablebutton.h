#ifndef DCHECKABLEBUTTON_H
#define DCHECKABLEBUTTON_H

#include <QPushButton>

class QLabel;

class DCheckableButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DCheckableButton(const QString& normal,
                              const QString& hover,
                              const QString& text,
                              QWidget *parent = 0);
    ~DCheckableButton();
    void initUI();
    void setUrl(const QString &url);
    QString getUrl();
protected:
    void nextCheckState();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:

public slots:

private:
    QString m_icon;
    QString m_text;
    QLabel* m_iconLabel;
    QLabel* m_textLabel;
    QString m_normal;
    QString m_hover;
    QString m_url;
};

#endif // DCHECKABLEBUTTON_H
