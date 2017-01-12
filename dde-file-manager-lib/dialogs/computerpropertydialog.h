#ifndef COMPUTERPROPERTYDIALOG_H
#define COMPUTERPROPERTYDIALOG_H

#include <QWidget>
#include <QMap>
#include <QDialog>

class ComputerPropertyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ComputerPropertyDialog(QWidget *parent = 0);
    void initUI();
    QHash<QString, QString> getMessage(const QStringList& data);

signals:
    void closed();

public slots:
protected:
    void closeEvent(QCloseEvent *e);
private:
    QString getComputerName();
    QString getBrand();
    QString getArch();
    QString getProcessor();
    QString getMemory();
    QString getDisk();
};

#endif // COMPUTERPROPERTYDIALOG_H
