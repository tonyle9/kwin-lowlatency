/**************************************************************************
 * KWin - the KDE window manager                                          *
 * This file is part of the KDE project.                                  *
 *                                                                        *
 * Copyright (C) 2013 Antonis Tsiapaliokas <kok3rs@gmail.com>             *
 *                                                                        *
 * This program is free software; you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation; either version 2 of the License, or      *
 * (at your option) any later version.                                    *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 **************************************************************************/


#ifndef COMPOSITING_H
#define COMPOSITING_H

#include <QAbstractItemModel>
#include <QObject>

class OrgKdeKwinCompositingInterface;

namespace KWin {
namespace Compositing {

class OpenGLPlatformInterfaceModel;

class Compositing : public QObject
{

    Q_OBJECT
    Q_PROPERTY(int animationSpeed READ animationSpeed WRITE setAnimationSpeed NOTIFY animationSpeedChanged)
    Q_PROPERTY(int windowThumbnail READ windowThumbnail WRITE setWindowThumbnail NOTIFY windowThumbnailChanged)
    Q_PROPERTY(int glScaleFilter READ glScaleFilter WRITE setGlScaleFilter NOTIFY glScaleFilterChanged)
    Q_PROPERTY(bool xrScaleFilter READ xrScaleFilter WRITE setXrScaleFilter NOTIFY xrScaleFilterChanged)
    Q_PROPERTY(bool unredirectFullscreen READ unredirectFullscreen WRITE setUnredirectFullscreen NOTIFY unredirectFullscreenChanged)
    Q_PROPERTY(int glSwapStrategy READ glSwapStrategy WRITE setGlSwapStrategy NOTIFY glSwapStrategyChanged)
    Q_PROPERTY(int compositingType READ compositingType WRITE setCompositingType NOTIFY compositingTypeChanged)
    Q_PROPERTY(bool compositingEnabled READ compositingEnabled WRITE setCompositingEnabled NOTIFY compositingEnabledChanged)
    Q_PROPERTY(KWin::Compositing::OpenGLPlatformInterfaceModel *openGLPlatformInterfaceModel READ openGLPlatformInterfaceModel CONSTANT)
    Q_PROPERTY(int openGLPlatformInterface READ openGLPlatformInterface WRITE setOpenGLPlatformInterface NOTIFY openGLPlatformInterfaceChanged)
    Q_PROPERTY(bool windowsBlockCompositing READ windowsBlockCompositing WRITE setWindowsBlockCompositing NOTIFY windowsBlockCompositingChanged)
    Q_PROPERTY(int animationCurve READ animationCurve WRITE setAnimationCurve NOTIFY animationCurveChanged)
    Q_PROPERTY(int latencyControl READ latencyControl WRITE setLatencyControl NOTIFY latencyControlChanged)
    Q_PROPERTY(int maxLatency READ maxLatency WRITE setMaxLatency NOTIFY maxLatencyChanged)
    Q_PROPERTY(int minLatency READ minLatency WRITE setMinLatency NOTIFY minLatencyChanged)
    Q_PROPERTY(bool compositingRequired READ compositingRequired CONSTANT)
public:
    explicit Compositing(QObject *parent = 0);

    Q_INVOKABLE bool OpenGLIsUnsafe() const;
    Q_INVOKABLE bool OpenGLIsBroken();
    Q_INVOKABLE void reenableOpenGLDetection();
    int animationSpeed() const;
    int windowThumbnail() const;
    int glScaleFilter() const;
    bool xrScaleFilter() const;
    bool unredirectFullscreen() const;
    int glSwapStrategy() const;
    int compositingType() const;
    bool compositingEnabled() const;
    int openGLPlatformInterface() const;
    bool windowsBlockCompositing() const;
    int animationCurve() const;
    int latencyControl() const;
    int maxLatency() const;
    int minLatency() const;
    bool compositingRequired() const;

    OpenGLPlatformInterfaceModel *openGLPlatformInterfaceModel() const;

    void setAnimationSpeed(int speed);
    void setWindowThumbnail(int index);
    void setGlScaleFilter(int index);
    void setXrScaleFilter(bool filter);
    void setUnredirectFullscreen(bool unredirect);
    void setGlSwapStrategy(int strategy);
    void setCompositingType(int index);
    void setCompositingEnabled(bool enabled);
    void setOpenGLPlatformInterface(int interface);
    void setWindowsBlockCompositing(bool set);
    void setAnimationCurve(int curve);
    void setLatencyControl(int index);
    void setMaxLatency(int val);
    void setMinLatency(int val);
    void save();

public Q_SLOTS:
    void reset();
    void defaults();

Q_SIGNALS:
    void changed();
    void animationSpeedChanged(int);
    void windowThumbnailChanged(int);
    void glScaleFilterChanged(int);
    void xrScaleFilterChanged(int);
    void unredirectFullscreenChanged(bool);
    void glSwapStrategyChanged(int);
    void compositingTypeChanged(int);
    void compositingEnabledChanged(bool);
    void openGLPlatformInterfaceChanged(int);
    void windowsBlockCompositingChanged(bool);
    void animationCurveChanged(int);
    void latencyControlChanged(int);
    void maxLatencyChanged(int);
    void minLatencyChanged(int);

private:
    int m_animationSpeed;
    int m_windowThumbnail;
    int m_glScaleFilter;
    bool m_xrScaleFilter;
    bool m_unredirectFullscreen;
    int m_glSwapStrategy;
    int m_compositingType;
    bool m_compositingEnabled;
    bool m_changed;
    OpenGLPlatformInterfaceModel *m_openGLPlatformInterfaceModel;
    int m_openGLPlatformInterface;
    bool m_windowsBlockCompositing;
    int m_animationCurve;
    int m_latencyControl;
    int m_maxLatency;
    int m_minLatency;
    bool m_windowsBlockingCompositing;
    OrgKdeKwinCompositingInterface *m_compositingInterface;
};


struct CompositingData;

class CompositingType : public QAbstractItemModel
{

    Q_OBJECT
    Q_ENUMS(CompositingTypeIndex)

public:

    enum CompositingTypeIndex {
        OPENGL31_INDEX = 0,
        OPENGL20_INDEX,
        XRENDER_INDEX
    };

    enum CompositingTypeRoles {
        NameRole = Qt::UserRole +1,
        TypeRole = Qt::UserRole +2
    };

    explicit CompositingType(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QHash< int, QByteArray > roleNames() const override;

    Q_INVOKABLE int compositingTypeForIndex(int row) const;
    Q_INVOKABLE int indexForCompositingType(int type) const;

private:
    void generateCompositing();
    QList<CompositingData> m_compositingList;

};

struct CompositingData {
    QString name;
    CompositingType::CompositingTypeIndex type;
};

class OpenGLPlatformInterfaceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit OpenGLPlatformInterfaceModel(QObject *parent = nullptr);
    virtual ~OpenGLPlatformInterfaceModel();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex indexForKey(const QString &key) const;

    QHash< int, QByteArray > roleNames() const override;

private:
    QStringList m_keys;
    QStringList m_names;
};

}//end namespace Compositing
}//end namespace KWin

Q_DECLARE_METATYPE(KWin::Compositing::OpenGLPlatformInterfaceModel*)
#endif
