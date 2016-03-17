#include "fileitem.h"
#include <QLabel>
#include <QSvgRenderer>
#include <QImage>
#include <QGraphicsEffect>
#include <QSvgRenderer>
#include <QPen>
#include <QPainter>
#include <QVBoxLayout>
#include "widgets/growingelidetextedit.h"
#include "utils/utils.h"
#include "widgets/themeappicon.h"


int FileItem::OneTextHeight = 26;
int FileItem::DoubleTextHeight = 44;


FileItem::FileItem(const QString &name, const QString &icon, QWidget *parent):
    QFrame(parent)
{
    m_fileName = name;
    m_fileIcon = icon;
    init();
}

FileItem::~FileItem()
{

}

QPixmap FileItem::turnSvgToQPixmap(const QString &svgFile, int width, int height)
{
    QPixmap pixmap(width, height);
    QSvgRenderer renderer(svgFile);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

QPixmap FileItem::applyShadowToPixmap(const QString filename)
{
    QImage source(filename);
    QSize imageSize = source.size();
    QRect borderRect(0, 0, imageSize.width() + 4, imageSize.height() + 4);
    QPixmap ret(borderRect.size());

    QPainter painter;
    painter.begin(&ret);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(borderRect, Qt::white);
    painter.drawImage(2, 2, source);
    painter.end();

    return ret;
}

void FileItem::init()
{
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("FileItem");
    initUI();
    initConnect();
    if (m_fileIcon.count() == 0){
        setFileIcon(ThemeAppIcon::getThemeIconPath("application-default-icon"));
    }else{
        setFileIcon(m_fileIcon);
    }
    setFileName(m_fileName);
}

void FileItem::initUI()
{
    m_iconLabel = new QLabel;
    m_iconLabel->setObjectName("Icon");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(44, 44);
    m_iconLabel->setScaledContents(false);

    QVBoxLayout* iconLayout = new QVBoxLayout;
    iconLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    iconLayout->setSpacing(0);
    iconLayout->setContentsMargins(4, 4, 4, 4);

    m_textedit = new GrowingElideTextEdit();
    m_textedit->setAlignment(Qt::AlignTop);
    m_textedit->setObjectName("GrowingElideTextEdit");
    m_textedit->setFixedWidth(100 - 10);
    m_textedit->setAcceptRichText(false);
    addTextShadow();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(iconLayout);
    mainLayout->addWidget(m_textedit, 0, Qt::AlignHCenter);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 4);
    setLayout(mainLayout);

    initPermissionLabel();
    initReadableLabel();
}

void FileItem::initReadableLabel()
{
    m_unReadableIndicatorLabel = new QLabel(this);
    m_unReadableIndicatorLabel->setFixedSize(24, 24);
    m_unReadableIndicatorLabel->hide();
    QPixmap pixmap = FileItem::turnSvgToQPixmap(":/images/images/unreadable.svg");
    m_unReadableIndicatorLabel->setPixmap(pixmap);
}

void FileItem::initPermissionLabel()
{
    m_permissionIndicatorLabel = new QLabel(this);
    m_permissionIndicatorLabel->setFixedSize(24, 24);
    m_permissionIndicatorLabel->hide();
    QPixmap pixmap = FileItem::turnSvgToQPixmap(":/images/images/readonly.svg");
    m_permissionIndicatorLabel->setPixmap(pixmap);
}

void FileItem::initConnect()
{
    connect(m_textedit, SIGNAL(heightChanged(int)), this, SLOT(updateHeight(int)));
//    connect(m_textedit, SIGNAL(renameFinished()), signalManager, SIGNAL(renameFinished()));
    connect(m_textedit, SIGNAL(menuShowed(QPoint)),this, SLOT(showContextMenu(QPoint)));
//    connect(m_textedit, SIGNAL(returnEnterPressed()), signalManager, SIGNAL(returnEnterPressed()));
}

QString FileItem::getFileIcon()
{
    return m_fileIcon;
}

QPixmap FileItem::getFileIconPixmap()
{
    return m_fileIconPixmap;
}

QString FileItem::getFileName()
{
    return m_fileName;
}

QString FileItem::getUrl()
{
    return m_url;
}

QString FileItem::getRawUrl()
{
    return m_rawUrl;
}

bool FileItem::isChecked()
{
    return m_checked;
}

bool FileItem::isHover()
{
    return m_hover;
}

bool FileItem::isCuted()
{
    return m_isCuted;
}

bool FileItem::isEditing()
{
    return m_isEditing;
}

bool FileItem::isShowSimpleMode()
{
    return m_textedit->isSimpleWrapMode();
}

FileItemInfo &FileItem::getFileItemInfo()
{
    return m_fileItemInfo;
}

GrowingElideTextEdit *FileItem::getTextEdit()
{
    return m_textedit;
}

QLabel *FileItem::getIconLabel()
{
    return m_iconLabel;
}

void FileItem::addImageShadow()
{
    QGraphicsDropShadowEffect *coverShadow = new QGraphicsDropShadowEffect;
    coverShadow->setBlurRadius(6);
    coverShadow->setColor(QColor(0, 0, 0, 76));
    coverShadow->setOffset(0, 2);
    m_iconLabel->setGraphicsEffect(coverShadow);
}

void FileItem::addTextShadow()
{
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
    textShadow->setBlurRadius(5);
    textShadow->setColor(QColor(0, 0, 0, 200));
    textShadow->setOffset(0, 2);
    m_textedit->setGraphicsEffect(textShadow);
}

void FileItem::disabledTextShadow()
{
    m_textedit->graphicsEffect()->setEnabled(false);
}

void FileItem::enableTextShadow()
{
    m_textedit->graphicsEffect()->setEnabled(true);
}

void FileItem::setFileIcon(QString icon)
{
    m_fileIcon = icon;
    if (icon.endsWith(".svg")){
            m_fileIconPixmap = QPixmap(m_iconLabel->size());
            QSvgRenderer renderer(icon);
            m_fileIconPixmap.fill(Qt::transparent);
            QPainter painter;
            painter.begin(&m_fileIconPixmap);
            renderer.render(&painter);
            painter.end();
        }else if (icon.startsWith("data:image/")){
            // icon is a string representing an inline image.
            QStringList strs = icon.split("base64,");
            if (strs.length() == 2) {
                QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
                m_fileIconPixmap.loadFromData(data);
            }
        }else{
            QMimeDatabase mimeDataBae;
            QMimeType mimeType = mimeDataBae.mimeTypeForFile(deleteFilePrefix(m_url));
            QString cacheIcon = joinPath(getThumbnailsPath(), QFileInfo(icon).fileName());
            if (mimeType.genericIconName() == "image-x-generic"){
                if (QFileInfo(cacheIcon).exists()){
                    m_fileIconPixmap = QPixmap(cacheIcon);
                }else{
                    m_fileIconPixmap = applyShadowToPixmap(icon);
                    m_fileIconPixmap.save(cacheIcon);
                }
            }else{
                if (icon.length() > 0){
                    m_fileIconPixmap = QPixmap(icon);
                }else{
                    m_fileIconPixmap = QPixmap(ThemeAppIcon::getThemeIconPath("application-default-icon"));
                }
            }
        }
        setFileIcon(m_fileIconPixmap);
}

void FileItem::setFileIcon(QPixmap icon)
{
     emit fileIconChanged(icon);
     m_fileIconPixmap = m_fileIconPixmap.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
     m_iconLabel->setPixmap(m_fileIconPixmap);
}

void FileItem::setFileName(QString name)
{
    m_fileName = name;
    emit fileNameChanged(name);
    m_textedit->setFullText(name);
    if (m_textedit->getTexts().length()  == 1 && !name.contains("\n")){
        m_textedit->setFixedHeight(FileItem::DoubleTextHeight);
    }else{
        m_textedit->setFixedHeight(FileItem::DoubleTextHeight);
    }
}

void FileItem::setChecked(bool checked, bool isExpanded)
{
    if (m_checked != checked){
       if (checked){
           setObjectName(QString("Checked"));
           if (isShowSimpleMode() && isExpanded){
                showFullWrapName();
           }
       }else{
           setObjectName(QString("Normal"));
           if (!isEditing()){
               showSimpWrapName();
           }
       }
       m_checked = checked;
       m_hover = false;
       update();
       emit checkedChanged(checked);
    }
}

void FileItem::setHover(bool hover)
{
    if (m_hover != hover){

    }
    m_hover = hover;
}

void FileItem::setCuted()
{
    if (!m_isCuted){
        disabledTextShadow();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.5);
        setGraphicsEffect(effect);
    }
    m_isCuted = true;
}

void FileItem::cancelCuted()
{
    if (m_isCuted){
        QGraphicsOpacityEffect * effect = static_cast<QGraphicsOpacityEffect*>(graphicsEffect());
        effect->setOpacity(1);
        effect->setEnabled(false);
        enableTextShadow();
    }
    m_isCuted = false;
}

void FileItem::setEditing(bool flag)
{
    m_isEditing = flag;
}

void FileItem::showEditing()
{
    setEditing(true);
    setChecked(false);
    m_textedit->showEditing();
    m_textedit->setFocus();
    if (m_textedit->getTexts().length()  == 1 && !m_fileName.contains("\n")){
        m_textedit->setFixedHeight(FileItem::OneTextHeight);
    }
}


void FileItem::setUrl(QString url)
{
    m_url = url;
}


void FileItem::setRaWUrl(QString url)
{
    m_rawUrl = url;
}


void FileItem::setFileItemInfo(FileItemInfo &fileItemInfo)
{
    m_fileItemInfo = fileItemInfo;
}


void FileItem::showFullWrapName()
{
    m_textedit->showFullWrapText();
}

void FileItem::showSimpWrapName()
{
    m_textedit->showSimpleElideText();
}

void FileItem::updateHeight(int textHeight)
{
    int h = m_iconLabel->height() + textHeight;
    m_textedit->setFixedHeight(textHeight);
    if (!m_textedit->isSimpleWrapMode() && h>=100){
        setFixedHeight(h + 15);
    }else{
        if (m_textedit->fontMetrics().width(m_textedit->toPlainText()) > m_textedit->width() - 10){
            m_textedit->setFixedHeight(FileItem::DoubleTextHeight);
            setFixedHeight(100);
        }else{
            if (isEditing()){
                m_textedit->setFixedHeight(FileItem::OneTextHeight);
            }else{
                m_textedit->setFixedHeight(FileItem::DoubleTextHeight);
            }
            setFixedHeight(100);
        }
    }
}

void FileItem::showContextMenu(QPoint pos)
{
    qDebug() << pos;
//    emit signalManager->showTextEditMenuBySelectContent(m_url, m_textedit->toPlainText(), m_textedit->textCursor().selectedText(), pos);
}

void FileItem::handleCut(QString url)
{
    if (url == m_url){
        m_textedit->cut();
    }
}

void FileItem::handleCopy(QString url)
{
    if (url == m_url){
        m_textedit->copy();
    }
}

void FileItem::handlePaste(QString url)
{
    if (url == m_url){
        m_textedit->paste();
    }
}

void FileItem::handleSelectAll(QString url)
{
    if (url == m_url){
        m_textedit->selectAll();
    }
}

void FileItem::setUserReadOnly(bool isUserReadOnly)
{
    m_isUserReadOnly = isUserReadOnly;
}

void FileItem::setUserReadPermisson_000(bool isUserPermisson_000)
{
    m_isUserPermisson_000 = isUserPermisson_000;
}

void FileItem::setReadable(bool isReadable)
{
    m_isReadable = isReadable;
}

void FileItem::show()
{
    QFrame::show();
    if (QFileInfo(m_url).isSymLink()){
        qDebug() << m_url << "isSymLink";
    }else{
        if (!m_isReadable){
            m_unReadableIndicatorLabel->move(m_iconLabel->mapToParent(QPoint(23, 23)));
            m_unReadableIndicatorLabel->show();
            if (m_isUserPermisson_000){
                m_permissionIndicatorLabel->move(m_iconLabel->mapToParent(QPoint(23, -3)));
                m_permissionIndicatorLabel->show();
            }else{
                m_permissionIndicatorLabel->hide();
            }
        }else{
            m_unReadableIndicatorLabel->hide();
            if (m_isUserReadOnly){
                m_permissionIndicatorLabel->move(m_iconLabel->mapToParent(QPoint(23, 23)));
                m_permissionIndicatorLabel->show();
            }else{
                m_permissionIndicatorLabel->hide();
            }
        }
    }
}

void FileItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton){
        qDebug() << event;
    }
    QFrame::mousePressEvent(event);
}

void FileItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isEditing){
        showSimpWrapName();
        m_textedit->tryRenamed();
    }

    QFrame::mouseReleaseEvent(event);
}

void FileItem::enterEvent(QEvent *event)
{
    setHover(true);
    QFrame::enterEvent(event);
}

void FileItem::leaveEvent(QEvent *event)
{
    setHover(false);
    QFrame::enterEvent(event);
}

void FileItem::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

#include <QLabel>
#include <QTextEdit>

FileIconItem::FileIconItem(QWidget *parent) :
    DVBoxWidget(parent)
{
    icon = new QLabel;
    edit = new QTextEdit;

    icon->setAlignment(Qt::AlignCenter);
    icon->setFrameShape(QFrame::NoFrame);
    icon->setStyleSheet("background: transparent");
    edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);

    layout()->addWidget(icon, 0, Qt::AlignHCenter);
    addWidget(edit);
    setFrameShape(QFrame::NoFrame);
    setFocusProxy(edit);
}

bool FileIconItem::event(QEvent *ee)
{
    if(!canDeferredDelete && ee->type() == QEvent::DeferredDelete) {
        ee->accept();
        return true;
    }

    return DVBoxWidget::event(ee);
}
