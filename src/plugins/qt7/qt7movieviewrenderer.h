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

#ifndef QT7MOVIEVIEWRENDERER_H
#define QT7MOVIEVIEWRENDERER_H

#include <QtCore/qobject.h>
#include <QtCore/qmutex.h>

#include <qvideowindowcontrol.h>
#include <qmediaplayer.h>

#include <QtGui/qmacdefines_mac.h>
#include "qt7videooutput.h"
#include <qvideoframe.h>

QT_BEGIN_NAMESPACE

class QVideoFrame;

class QT7PlayerSession;
class QT7PlayerService;

class QT7MovieViewRenderer : public QT7VideoRendererControl
{
public:
    QT7MovieViewRenderer(QObject *parent = 0);
    ~QT7MovieViewRenderer();

    void setMovie(void *movie);
    void updateNaturalSize(const QSize &newSize);

    QAbstractVideoSurface *surface() const;
    void setSurface(QAbstractVideoSurface *surface);

    void renderFrame(const QVideoFrame &);

protected:
    bool event(QEvent *event);

private:
    void setupVideoOutput();

    void *m_movie;
    void *m_movieView;
    QSize m_nativeSize;
    QAbstractVideoSurface *m_surface;
    QVideoFrame m_currentFrame;
    bool m_pendingRenderEvent;
    QMutex m_mutex;
};

QT_END_NAMESPACE

#endif
