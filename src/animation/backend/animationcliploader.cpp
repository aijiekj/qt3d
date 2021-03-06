/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "animationcliploader_p.h"
#include <Qt3DAnimation/qanimationcliploader.h>
#include <Qt3DAnimation/private/qanimationcliploader_p.h>
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DRender/private/qurlhelper_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

AnimationClipLoader::AnimationClipLoader()
    : BackendNode(ReadWrite)
    , m_source()
    , m_status(QAnimationClipLoader::NotReady)
    , m_name()
    , m_channels()
    , m_duration(0.0f)
{
}

void AnimationClipLoader::initializeFromPeer(const Qt3DCore::QNodeCreatedChangeBasePtr &change)
{
    const auto typedChange = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<QAnimationClipLoaderData>>(change);
    const auto &data = typedChange->data;
    m_source = data.source;
    if (!m_source.isEmpty())
        setDirty(Handler::AnimationClipDirty);
}

void AnimationClipLoader::cleanup()
{
    setEnabled(false);
    m_handler = nullptr;
    m_source.clear();
    m_status = QAnimationClipLoader::NotReady;
    m_channels.clear();
    m_duration = 0.0f;

    clearData();
}

void AnimationClipLoader::setStatus(QAnimationClipLoader::Status status)
{
    if (status != m_status) {
        m_status = status;
        Qt3DCore::QPropertyUpdatedChangePtr e = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("status");
        e->setValue(QVariant::fromValue(m_status));
        notifyObservers(e);
    }
}

void AnimationClipLoader::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    switch (e->type()) {
    case Qt3DCore::PropertyUpdated: {
        const auto change = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(e);
        if (change->propertyName() == QByteArrayLiteral("source")) {
            m_source = change->value().toUrl();
            setDirty(Handler::AnimationClipDirty);
        }
        break;
    }

    default:
        break;
    }
    QBackendNode::sceneChangeEvent(e);
}

/*!
    \internal
    Called by LoadAnimationClipJob on the threadpool
 */
void AnimationClipLoader::loadAnimation()
{
    qCDebug(Jobs) << Q_FUNC_INFO << m_source;
    clearData();

    // TODO: Handle remote files
    QString filePath = Qt3DRender::QUrlHelper::urlToLocalFileOrQrc(m_source);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not find animation clip:" << filePath;
        setStatus(QAnimationClipLoader::Error);
        return;
    }

    QByteArray animationData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(animationData);
    QJsonObject rootObject = document.object();

    // TODO: Allow loading of a named animation from a file containing many
    QJsonArray animationsArray = rootObject[QLatin1String("animations")].toArray();
    qCDebug(Jobs) << "Found" << animationsArray.size() << "animations:";
    for (int i = 0; i < animationsArray.size(); ++i) {
        QJsonObject animation = animationsArray.at(i).toObject();
        qCDebug(Jobs) << "Animation Name:" << animation[QLatin1String("animationName")].toString();
    }

    // For now just load the first animation
    // TODO: Allow loading a named animation from within the file analogous to QMesh
    QJsonObject animation = animationsArray.at(0).toObject();
    m_name = animation[QLatin1String("animationName")].toString();
    QJsonArray channelsArray = animation[QLatin1String("channels")].toArray();
    const int channelCount = channelsArray.size();
    m_channels.resize(channelCount);
    for (int i = 0; i < channelCount; ++i) {
        const QJsonObject group = channelsArray.at(i).toObject();
        m_channels[i].read(group);
    }

    const float t = findDuration();
    setDuration(t);

    m_channelComponentCount = findChannelComponentCount();

    if (qFuzzyIsNull(t) || m_channelComponentCount == 0)
        setStatus(QAnimationClipLoader::Error);
    else
        setStatus(QAnimationClipLoader::Ready);

    qCDebug(Jobs) << "Loaded animation data:" << *this;
}

void AnimationClipLoader::setDuration(float duration)
{
    if (qFuzzyCompare(duration, m_duration))
        return;

    m_duration = duration;

    // Send a change to the frontend
    auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(peerId());
    e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
    e->setPropertyName("duration");
    e->setValue(m_duration);
    notifyObservers(e);
}

int AnimationClipLoader::channelIndex(const QString &channelName) const
{
    const int channelCount = m_channels.size();
    for (int i = 0; i < channelCount; ++i) {
        if (m_channels[i].name == channelName)
            return i;
    }
    return -1;
}

/*!
    \internal

    Given the index of a channel, \a channelIndex, calculates
    the base index of the first channelComponent in this group. For example, if
    there are two channel groups each with 3 channels and you request
    the channelBaseIndex(1), the return value will be 3. Indices 0-2 are
    for the first group, so the first channel of the second group occurs
    at index 3.
 */
int AnimationClipLoader::channelComponentBaseIndex(int channelIndex) const
{
    int index = 0;
    for (int i = 0; i < channelIndex; ++i)
        index += m_channels[i].channelComponents.size();
    return index;
}

void AnimationClipLoader::clearData()
{
    m_name.clear();
    m_channels.clear();
}

float AnimationClipLoader::findDuration()
{
    // Iterate over the contained fcurves and find the longest one
    double tMax = 0.0;
    for (const Channel &channel : qAsConst(m_channels)) {
        for (const ChannelComponent &channelComponent : qAsConst(channel.channelComponents)) {
            const float t = channelComponent.fcurve.endTime();
            if (t > tMax)
                tMax = t;
        }
    }
    return tMax;
}

int AnimationClipLoader::findChannelComponentCount()
{
    int channelCount = 0;
    for (const Channel &channel : qAsConst(m_channels))
        channelCount += channel.channelComponents.size();
    return channelCount;
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
