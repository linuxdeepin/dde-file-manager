#ifndef DTITLEBAR_H
#define DTITLEBAR_H

#include <QFrame>

class QIcon;
class QPushButton;

class DTitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit DTitleBar(QWidget *parent = 0);
    ~DTitleBar();

    static const int ButtonHeight;

    void initData();
    void initUI();
    void initConnect();

signals:
    void maximumed();
    void minimuned();
    void normaled();
    void closed();
    void switchMaxNormal();

public slots:
    void setNormalIcon();
    void setMaxIcon();

protected:
    void  mouseDoubleClickEvent(QMouseEvent* event);

private:
    QIcon* m_settingsIcon;
    QIcon* m_minIcon;
    QIcon* m_maxIcon;
    QIcon* m_normalIcon;
    QIcon* m_closeIcon;

    QPushButton* m_settingButton;
    QPushButton* m_minButton;
    QPushButton* m_maxNormalButton;
    QPushButton* m_closeButton;
};

#endif // DTITLEBAR_H
