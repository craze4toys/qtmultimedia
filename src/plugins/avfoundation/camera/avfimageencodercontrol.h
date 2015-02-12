/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AVFIMAGEENCODERCONTROL_H
#define AVFIMAGEENCODERCONTROL_H

#include <QtMultimedia/qmediaencodersettings.h>
#include <QtMultimedia/qimageencodercontrol.h>

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

@class AVCaptureDeviceFormat;

QT_BEGIN_NAMESPACE

class AVFCameraService;

class AVFImageEncoderControl : public QImageEncoderControl
{
    Q_OBJECT

    friend class AVFCameraSession;
public:
    AVFImageEncoderControl(AVFCameraService *service);

    QStringList supportedImageCodecs() const Q_DECL_OVERRIDE;
    QString imageCodecDescription(const QString &codecName) const Q_DECL_OVERRIDE;
    QList<QSize> supportedResolutions(const QImageEncoderSettings &settings,
                                      bool *continuous) const Q_DECL_OVERRIDE;
    QImageEncoderSettings imageSettings() const Q_DECL_OVERRIDE;
    void setImageSettings(const QImageEncoderSettings &settings) Q_DECL_OVERRIDE;

private:
    AVFCameraService *m_service;
    QImageEncoderSettings m_settings;

    void applySettings();
    bool videoCaptureDeviceIsValid() const;
};

QSize qt_image_high_resolution(AVCaptureDeviceFormat *fomat);

QT_END_NAMESPACE

#endif
