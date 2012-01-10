/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

attribute highp vec4 qt_Vertex;
attribute highp vec4 qt_Normal;
uniform highp mat3 qt_NormalMatrix;
uniform highp mat4 qt_ModelViewProjectionMatrix;

uniform highp mat4 objectLinearTexgenMatrix;
uniform highp vec4 projectorDirection;

varying highp vec4 texCoord;
varying float facingProjector;

void main(void)
{
    texCoord = objectLinearTexgenMatrix * qt_Vertex;
    vec4 normal = normalize(vec4(qt_NormalMatrix * vec3(qt_Vertex), qt_Vertex.w));
    // We're facing the projector if the normal and projector direction
    // are pointing in opposite directions, i.e. if their dot product
    // is negative.
    facingProjector = -dot(normalize(normal), normalize(projectorDirection));
    facingProjector = clamp(facingProjector, 0.0, 1.0);
    gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
};
