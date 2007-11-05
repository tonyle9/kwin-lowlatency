//////////////////////////////////////////////////////////////////////////////
// oxygenclient.cpp
// -------------------
// Oxygen window decoration for KDE
// -------------------
// Copyright (c) 2003, 2004 David Johnson
// Copyright (c) 2006, 2007 Casper Boemann <cbr@boemann.dk>
// Copyright (c) 2006, 2007 Riccardo Iaconelli <riccardo@kde.org>
//
// Please see the header file for copyright and license information.
//////////////////////////////////////////////////////////////////////////////
// #ifndef OXYGENCLIENT_H
// #define OXYGENCLIENT_H

#include <KConfig>
#include <KGlobal>
#include <KLocale>
#include <KDebug>
#include <KColorUtils>

#include <qbitmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QPainterPath>
#include <QTimer>
#include <QCache>

#include "math.h"

#include "oxygenclient.h"
#include "oxygenclient.moc"
#include "oxygenbutton.h"
#include "oxygen.h"

namespace Oxygen
{

K_GLOBAL_STATIC_WITH_ARGS(OxygenHelper, globalHelper, ("OxygenDeco"))

void renderDot(QPainter *p, const QPointF &point, qreal diameter)
{
    p->drawEllipse(QRectF(point.x()-diameter/2, point.y()-diameter/2, diameter, diameter));
}


OxygenClient::OxygenClient(KDecorationBridge *b, KDecorationFactory *f)
    : KCommonDecoration(b, f)
    , helper_(*globalHelper)
    , colorCacheInvalid_(true)
{
}

OxygenClient::~OxygenClient()
{
}
QString OxygenClient::visibleName() const
{
    return i18n("Oxygen");
}

QString OxygenClient::defaultButtonsLeft() const
{
    return "M";
}

QString OxygenClient::defaultButtonsRight() const
{
    return "HIAX";
}

void OxygenClient::init()
{
    KCommonDecoration::init();

    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent);
    widget()->setAttribute(Qt::WA_PaintOnScreen, false);
}

bool OxygenClient::decorationBehaviour(DecorationBehaviour behaviour) const
{
    switch (behaviour) {
        case DB_MenuClose:
            return true;//Handler()->menuClose();

        case DB_WindowMask:
            return false;

        default:
            return KCommonDecoration::decorationBehaviour(behaviour);
    }
}

int OxygenClient::layoutMetric(LayoutMetric lm, bool respectWindowState, const KCommonDecorationButton *btn) const
{
    bool maximized = maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows();

    switch (lm) {
        case LM_BorderLeft:
        case LM_BorderRight:
        case LM_BorderBottom:
        {
            if (respectWindowState && maximized) {
                return 0;
            } else {
                return BFRAMESIZE;
            }
        }

        case LM_TitleEdgeTop:
        {
            if (respectWindowState && maximized) {
                return 0;
            } else {
                return TFRAMESIZE;
            }
        }

        case LM_TitleEdgeBottom:
        {
            return 0;
        }

        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
        {
            if (respectWindowState && maximized) {
                return 0;
            } else {
                return 6;
            }
        }

        case LM_TitleBorderLeft:
        case LM_TitleBorderRight:
            return 5;

        case LM_ButtonWidth:
        case LM_ButtonHeight:
        case LM_TitleHeight:
        {
            if (respectWindowState && isToolWindow()) {
                return OXYGEN_BUTTONSIZE;
            } else {
                return OXYGEN_BUTTONSIZE;
            }
        }

        case LM_ButtonSpacing:
            return 1;

        case LM_ButtonMarginTop:
            return 0;

        case LM_ExplicitButtonSpacer:
            return 3;

        default:
            return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
    }
}


KCommonDecorationButton *OxygenClient::createButton(::ButtonType type)
{
    switch (type) {
        case MenuButton:
            return new OxygenButton(*this, i18n("Menu"), ButtonMenu);

        case HelpButton:
            return new OxygenButton(*this, i18n("Help"), ButtonHelp);

        case MinButton:
            return new OxygenButton(*this, i18n("Minimize"), ButtonMin);

        case MaxButton:
            return new OxygenButton(*this, i18n("Minimize"), ButtonMax);

        case CloseButton:
            return new OxygenButton(*this, i18n("Minimize"), ButtonClose);

        default:
            return 0;
    }
}


// c0 - background
// c1 - foreground
// t - target contrast ratio
QColor reduceContrast(const QColor &c0, const QColor &c1, double t)
{
    double s = KColorUtils::contrastRatio(c0, c1);
    if (s < t)
        return c1;

    double l = 0.0, h = 1.0;
    double x = s, a;
    QColor r = c1;
    for (int maxiter = 16; maxiter; --maxiter) {
        a = 0.5 * (l + h);
        r = KColorUtils::mix(c0, c1, a);
        x = KColorUtils::contrastRatio(c0, r);
        if (fabs(x - t) < 0.01)
            break;
        if (x > t)
            h = a;
        else
            l = a;
    }
    return r;
}


QColor OxygenClient::titlebarTextColor(const QPalette &palette)
{
    if (isActive())
        return palette.color(QPalette::Active, QPalette::WindowText);
    else {
        if(colorCacheInvalid_) {
            QColor ab = palette.color(QPalette::Active, QPalette::Window);
            QColor af = palette.color(QPalette::Active, QPalette::WindowText);
            QColor nb = palette.color(QPalette::Inactive, QPalette::Window);
            QColor nf = palette.color(QPalette::Inactive, QPalette::WindowText);

            colorCacheInvalid_ = false;
            cachedTitlebarTextColor_ = reduceContrast(nb, nf, qMax(2.5, KColorUtils::contrastRatio(ab, KColorUtils::mix(ab, af, 0.4))));
        }
        return cachedTitlebarTextColor_;
    }
}


void OxygenClient::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    if (!OxygenFactory::initialized()) return;

    doShape();

    QPalette palette = widget()->palette();
    QPainter painter(widget());

    // Set palette to the right group.
    // TODO - fix KWin to do this for us :-).
    if (isActive())
        palette.setCurrentColorGroup(QPalette::Active);
    else
        palette.setCurrentColorGroup(QPalette::Inactive);

    int x,y,w,h;
    QRect frame = widget()->frameGeometry();
    QColor color = palette.window();

    const int titleHeight = layoutMetric(LM_TitleHeight);
    const int titleTop = layoutMetric(LM_TitleEdgeTop) + frame.top();
    const int titleEdgeLeft = layoutMetric(LM_TitleEdgeLeft);
    const int marginLeft = layoutMetric(LM_TitleBorderLeft);
    const int marginRight = layoutMetric(LM_TitleBorderRight);

    const int titleLeft = frame.left() + titleEdgeLeft + buttonsLeftWidth() + marginLeft;
    const int titleWidth = frame.width() -
            titleEdgeLeft - layoutMetric(LM_TitleEdgeRight) -
            buttonsLeftWidth() - buttonsRightWidth() -
            marginLeft - marginRight;


    int splitY = qMin(300, 3*frame.height()/4);

    QPixmap tile = helper_.verticalGradient(color, splitY);
    painter.drawTiledPixmap(QRect(0, 0, frame.width(), titleHeight + TFRAMESIZE), tile);

    painter.drawTiledPixmap(QRect(0, 0, LFRAMESIZE, splitY), tile);
    painter.fillRect(0, splitY, LFRAMESIZE, frame.height() - splitY, helper_.backgroundBottomColor(color));

    painter.drawTiledPixmap(QRect(frame.width()-RFRAMESIZE, 0,
                                                        RFRAMESIZE, splitY), tile,
                                                        QPoint(frame.width()-RFRAMESIZE, 0));
    painter.fillRect(frame.width()-RFRAMESIZE, splitY, RFRAMESIZE, frame.height() - splitY, helper_.backgroundBottomColor(color));

    painter.fillRect(0, frame.height() - BFRAMESIZE, frame.width(), BFRAMESIZE, helper_.backgroundBottomColor(color));

    int radialW = qMin(600, frame.width());
    tile = helper_.radialGradient(color, radialW);
    QRect radialRect = QRect((frame.width() - radialW) / 2, 0, radialW, 64);
    painter.drawPixmap(radialRect, tile);

    //painter.setRenderHint(QPainter::Antialiasing,true);

    // draw title text
    painter.setFont(options()->font(isActive(), false));
    painter.setPen(titlebarTextColor(palette));
    painter.drawText(titleLeft, titleTop-1, titleWidth, titleHeight,  // -1 is to go into top resizearea
              OxygenFactory::titleAlign() | Qt::AlignVCenter, caption());

    painter.setRenderHint(QPainter::Antialiasing);

    // Draw dividing line
    frame = widget()->rect();
    frame.getRect(&x, &y, &w, &h);

    QColor light = helper_.calcLightColor(palette.window());
    QColor dark = helper_.calcDarkColor(palette.window());
    dark.setAlpha(120);

    if(!isActive()) {
        light.setAlpha(120);
        dark.setAlpha(50);
    }

    QLinearGradient lg(x,0,x+w,0);
    lg.setColorAt(0.5, dark);
    dark.setAlpha(0);
    lg.setColorAt(0.0, dark);
    lg.setColorAt(1.0, dark);
    painter.setPen(QPen(lg,1));

    painter.drawLine(QPointF(x, titleTop+titleHeight-1.5),
                            QPointF(x+w, titleTop+titleHeight-1.5));

    lg = QLinearGradient(x,0,x+w,0);
    lg.setColorAt(0.5, light);
    light.setAlpha(0);
    lg.setColorAt(0.0, light);
    lg.setColorAt(1.0, light);
    painter.setPen(QPen(lg,1));

    painter.drawLine(QPointF(x, titleTop+titleHeight-0.5),
                           QPointF(x+w, titleTop+titleHeight-0.5));

    // Draw shadows of the frame

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColor(255,255,255, 120));
    painter.drawLine(QPointF(0, 0.5), QPointF(w, 0.5));
    painter.setPen(QColor(128,128,128, 60));
    painter.drawLine(QPointF(0.5, 0), QPointF(0.5, h));
    painter.drawLine(QPointF(w-0.5, 0), QPointF(w-0.5, h));
    painter.setPen(QColor(0,0,0, 60));
    painter.drawLine(QPointF(0, h-0.5), QPointF(w, h-0.5));

    painter.setPen(QColor(0,0,0, 40));
    painter.drawPoint(QPointF(1.5, 1.5)); // top middle point
    painter.drawPoint(QPointF(w-1.5, 1.5));
    painter.drawPoint(QPointF(3.5, 0.5)); // top away points
    painter.drawPoint(QPointF(w-3.5, 0.5));
    painter.drawPoint(QPointF(0.5, 3.5));
    painter.drawPoint(QPointF(w-0.5, 3.5));
    painter.drawPoint(QPointF(1.5, h-1.5)); // bottom middle point
    painter.drawPoint(QPointF(w-1.5, h-1.5));

    // Draw the 3-dots resize handles
    qreal cenY = frame.height() / 2 + 0.5;
    qreal posX = frame.width() - 2.5;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 66));
    renderDot(&painter, QPointF(posX, cenY - 3), 1.8);
    renderDot(&painter, QPointF(posX, cenY), 1.8);
    renderDot(&painter, QPointF(posX, cenY + 3), 1.8);

    painter.translate(frame.width()-9, frame.height()-9);
    renderDot(&painter, QPointF(2.5, 6.5), 1.8);
    renderDot(&painter, QPointF(5.5, 5.5), 1.8);
    renderDot(&painter, QPointF(6.5, 2.5), 1.8);
}

void OxygenClient::doShape()
{
    bool maximized = maximizeMode()==MaximizeFull && !options()->moveResizeMaximizedWindows();
  int r=widget()->width();
  int b=widget()->height();
QRegion mask(0,0,r,b);

    if(maximized) {
        setMask(mask);
        return;
    }

    // Remove top-left corner.
    mask -= QRegion(0, 0, 3, 1);
    mask -= QRegion(0, 1, 1, 1);
    mask -= QRegion(0, 2, 1, 1);

    // Remove top-right corner.
    mask -= QRegion(r - 3, 0, 3, 1);
    mask -= QRegion(r - 1, 1, 1, 1);
    mask -= QRegion(r - 1, 2, 1, 1);

    // Remove bottom-left corner.
    mask -= QRegion(0, b-1-0, 3, b-1-1);
    mask -= QRegion(0, b-1-1, 1, b-1-1);
    mask -= QRegion(0, b-1-2, 1, b-1-1);

    // Remove bottom-right corner.
    mask -= QRegion(r - 3, b-1-0, 3, b-1-1);
    mask -= QRegion(r - 1, b-1-1, 1, b-1-1);
    mask -= QRegion(r - 1, b-1-2, 1, b-1-1);

    setMask(mask);
}

} //namespace Oxygen

//#include "oxygenclient.moc"

// #endif
