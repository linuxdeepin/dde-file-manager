#ifndef BMLISTWIDGET_H
#define BMLISTWIDGET_H

#include <QListWidget>
#include <QButtonGroup>

class DCheckableButton;

class BMListWidget : public QListWidget
{
    Q_OBJECT
public:
    BMListWidget(QWidget * parent = 0);
    void setItemWidget(QListWidgetItem *item, DCheckableButton *widget);
    void addSeparator(const QSize &size);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
private:
    QButtonGroup * m_group;
signals:
    void urlSelected(QString url);
public slots:
    void buttonSelected(int id);
};

#endif // BMLISTWIDGET_H
