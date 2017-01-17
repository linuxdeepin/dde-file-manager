#ifndef ERRORPAGE_H
#define ERRORPAGE_H

#include <QFrame>
class QLabel;

class ErrorPage : public QFrame
{
    Q_OBJECT
public:
    explicit ErrorPage(const QString& errorStr = "", QWidget* parent = 0);
    void initUI();
    void setErrorMsg(const QString& errorMsg);

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    QString m_errorText;
    QLabel* m_errorLabel;
};

#endif // ERRORPAGE_H
