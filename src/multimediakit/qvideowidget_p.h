/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVIDEOWIDGET_P_H
#define QVIDEOWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtmultimediakitdefs.h>
#include "qvideowidget.h"

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

#include "qpaintervideosurface_p.h"

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QMediaService;

class QVideoWidgetControlInterface
{
public:
    virtual ~QVideoWidgetControlInterface() {}

    virtual void setBrightness(int brightness) = 0;
    virtual void setContrast(int contrast) = 0;
    virtual void setHue(int hue) = 0;
    virtual void setSaturation(int saturation) = 0;

    virtual void setFullScreen(bool fullScreen) = 0;

    virtual Qt::AspectRatioMode aspectRatioMode() const = 0;
    virtual void setAspectRatioMode(Qt::AspectRatioMode mode) = 0;
};

class QVideoWidgetBackend : public QObject, public QVideoWidgetControlInterface
{
    Q_OBJECT
public:
    virtual QSize sizeHint() const = 0;

    virtual void showEvent() = 0;
    virtual void hideEvent(QHideEvent *event) = 0;
    virtual void resizeEvent(QResizeEvent *event) = 0;
    virtual void moveEvent(QMoveEvent *event) = 0;
    virtual void paintEvent(QPaintEvent *event) = 0;
};

class QVideoWidgetControl;

class QVideoWidgetControlBackend : public QObject, public QVideoWidgetControlInterface
{
    Q_OBJECT
public:
    QVideoWidgetControlBackend(QMediaService *service, QVideoWidgetControl *control, QWidget *widget);

    void releaseControl();

    void setBrightness(int brightness);
    void setContrast(int contrast);
    void setHue(int hue);
    void setSaturation(int saturation);

    void setFullScreen(bool fullScreen);

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode(Qt::AspectRatioMode mode);

private:
    QMediaService *m_service;
    QVideoWidgetControl *m_widgetControl;
};


class QVideoRendererControl;

class QRendererVideoWidgetBackend : public QVideoWidgetBackend
{
    Q_OBJECT
public:
    QRendererVideoWidgetBackend(QMediaService *service, QVideoRendererControl *control, QWidget *widget);
    ~QRendererVideoWidgetBackend();

    void releaseControl();
    void clearSurface();

    void setBrightness(int brightness);
    void setContrast(int contrast);
    void setHue(int hue);
    void setSaturation(int saturation);

    void setFullScreen(bool fullScreen);

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode(Qt::AspectRatioMode mode);

    QSize sizeHint() const;

    void showEvent();
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void fullScreenChanged(bool fullScreen);
    void brightnessChanged(int brightness);
    void contrastChanged(int contrast);
    void hueChanged(int hue);
    void saturationChanged(int saturation);

private Q_SLOTS:
    void formatChanged(const QVideoSurfaceFormat &format);
    void frameChanged();

private:
    void updateRects();

    QMediaService *m_service;
    QVideoRendererControl *m_rendererControl;
    QWidget *m_widget;
    QPainterVideoSurface *m_surface;
    Qt::AspectRatioMode m_aspectRatioMode;
    QRect m_boundingRect;
    QRectF m_sourceRect;
    QSize m_nativeSize;
    bool m_updatePaintDevice;
};

class QVideoWindowControl;

class QWindowVideoWidgetBackend : public QVideoWidgetBackend
{
    Q_OBJECT
public:
    QWindowVideoWidgetBackend(QMediaService *service, QVideoWindowControl *control, QWidget *widget);
    ~QWindowVideoWidgetBackend();

    void releaseControl();

    void setBrightness(int brightness);
    void setContrast(int contrast);
    void setHue(int hue);
    void setSaturation(int saturation);

   void setFullScreen(bool fullScreen);

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode(Qt::AspectRatioMode mode);

    QSize sizeHint() const;

    void showEvent();
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    void paintEvent(QPaintEvent *event);

#if defined(Q_WS_WIN)
    bool winEvent(MSG *message, long *result);
#endif

private:
    QMediaService *m_service;
    QVideoWindowControl *m_windowControl;
    QWidget *m_widget;
    Qt::AspectRatioMode m_aspectRatioMode;
    QSize m_pixelAspectRatio;
};

class QMediaService;
class QVideoOutputControl;

class QVideoWidgetPrivate
{
    Q_DECLARE_PUBLIC(QVideoWidget)
public:
    QVideoWidgetPrivate()
        : q_ptr(0)
        , mediaObject(0)
        , service(0)        
        , widgetBackend(0)
        , windowBackend(0)
        , rendererBackend(0)
        , currentControl(0)
        , currentBackend(0)
        , brightness(0)
        , contrast(0)
        , hue(0)
        , saturation(0)
        , aspectRatioMode(Qt::KeepAspectRatio)
        , nonFullScreenFlags(0)
        , wasFullScreen(false)
    {
    }

    QVideoWidget *q_ptr;
    QPointer<QMediaObject> mediaObject;
    QMediaService *service;
    QVideoWidgetControlBackend *widgetBackend;
    QWindowVideoWidgetBackend *windowBackend;
    QRendererVideoWidgetBackend *rendererBackend;
    QVideoWidgetControlInterface *currentControl;
    QVideoWidgetBackend *currentBackend;
    int brightness;
    int contrast;
    int hue;
    int saturation;
    Qt::AspectRatioMode aspectRatioMode;
    Qt::WindowFlags nonFullScreenFlags;
    bool wasFullScreen;

    bool createWidgetBackend();
    bool createWindowBackend();
    bool createRendererBackend();

    void setCurrentControl(QVideoWidgetControlInterface *control);
    void clearService();

    void _q_serviceDestroyed();
    void _q_brightnessChanged(int brightness);
    void _q_contrastChanged(int contrast);
    void _q_hueChanged(int hue);
    void _q_saturationChanged(int saturation);
    void _q_fullScreenChanged(bool fullScreen);
    void _q_dimensionsChanged();
};

QT_END_NAMESPACE

#endif
