#ifndef DSEARCHBAR_H
#define DSEARCHBAR_H

#include <QLineEdit>
#include <QListWidget>
#include <QListView>
#include <QMenu>
#include <QHBoxLayout>
#include <QCompleter>

class DSearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DSearchBar(QWidget *parent = 0);
    ~DSearchBar();
    QListWidget * getPopupList();
    QAction * setClearAction();
    QAction * removeClearAction();

private:
    QListWidget * m_list;
    QCompleter * m_completer;
    QAction * m_clearAction;
    void initConnections();
public slots:
    void doTextChanged(QString text);
protected:
    void focusInEvent(QFocusEvent *e);
signals:
    void searchBarFocused();
};

#endif // DSEARCHBAR_H
