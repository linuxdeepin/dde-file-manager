#ifndef DADVANCEDINFOWIDGET_H
#define DADVANCEDINFOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QUrl>
#include <QPair>

typedef QPair<QString, QString> QStringPair;
class DAdvancedInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DAdvancedInfoWidget(QWidget *parent = 0, const QString& file = "");
    void initData();
    void initUI();

signals:

public slots:

private:
    QListWidget* m_listWidget;
    QList<QStringPair> m_InfoList;
    QUrl m_url;
};

#endif // DADVANCEDINFOWIDGET_H
