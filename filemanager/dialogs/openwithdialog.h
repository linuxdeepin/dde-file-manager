#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include <QObject>
#include <dwidget.h>


DWIDGET_USE_NAMESPACE

class QListWidget;
class QPushButton;

class OpenWithDialog : public DWidget
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const QString& url, QWidget *parent = 0);
    ~OpenWithDialog();

    void initUI();
    void initConnect();
    void addItems();

signals:

public slots:
    void openFileByApp();

private:
    QListWidget* m_listWidget = NULL;
    QPushButton* m_cancelButton = NULL;
    QPushButton* m_chooseButton = NULL;
    QString m_url;
};

#endif // OPENWITHDIALOG_H
