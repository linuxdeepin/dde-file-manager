#ifndef FRAME_H
#define FRAME_H

#include <dblureffectwidget.h>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class WallpaperList;
class WallpaperListView;
class AppearanceDaemonInterface;
class DeepinWM;
class DBusXMouseArea;
class Frame : public DBlurEffectWidget
{
    Q_OBJECT

public:
    Frame(QFrame *parent = 0);
    ~Frame();

    void show();

signals:
    void done();

public slots:
    void handleNeedCloseButton(QString path, QPoint pos);

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    WallpaperList *m_wallpaperList = NULL;
    DImageButton * m_closeButton = NULL;

    AppearanceDaemonInterface * m_dbusAppearance = NULL;
    DeepinWM * m_dbusDeepinWM = NULL;
    DBusXMouseArea * m_dbusMouseArea = NULL;

    QString m_formerWallpaper;
    QString m_mouseAreaKey;
    QMap<QString, bool> m_deletableInfo;

    void initSize();
    void initListView();
    void refreshList();
    QStringList processListReply(const QString &reply);
};

#endif // FRAME_H
