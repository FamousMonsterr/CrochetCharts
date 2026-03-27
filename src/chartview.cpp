/****************************************************************************\
 Copyright (c) 2010-2014 Stitch Works Software
 Brian C. Milco <bcmilco@gmail.com>

 This file is part of Crochet Charts.

 Crochet Charts is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Crochet Charts is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Crochet Charts. If not, see <http://www.gnu.org/licenses/>.

 \****************************************************************************/
#include "chartview.h"
#include "settings.h"

#include <QDebug>
#include <QGestureEvent>
#include <QNativeGestureEvent>
#include <QPainter>
#include <QPinchGesture>
#include <QScrollBar>
#include <QWheelEvent>

namespace {

bool isTrackpadScrollEvent(const QWheelEvent *event)
{
    if(!event)
        return false;

    return !event->pixelDelta().isNull() || event->phase() != Qt::NoScrollPhase;
}

}

ChartView::ChartView(QWidget* parent)
    : QGraphicsView(parent)
	
{
    setAcceptDrops(true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    viewport()->grabGesture(Qt::PinchGesture);

    const bool lowGraphicsMode = Settings::inst()->value("lowGraphicsMode").toBool();
    if (lowGraphicsMode) {
        setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        setCacheMode(QGraphicsView::CacheBackground);
        setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        setRenderHint(QPainter::Antialiasing, false);
        setRenderHint(QPainter::SmoothPixmapTransform, false);
        setRenderHint(QPainter::TextAntialiasing, true);
    } else {
        setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
        setRenderHint(QPainter::TextAntialiasing, true);
    }
}

ChartView::~ChartView()
{
}

bool ChartView::viewportEvent(QEvent *event)
{
    if(!event)
        return false;

    if(event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent *gestureEvent = static_cast<QNativeGestureEvent*>(event);
        if(gestureEvent->gestureType() == Qt::ZoomNativeGesture) {
            applyZoomFactor(1.0 + gestureEvent->value());
            gestureEvent->accept();
            return true;
        }
    } else if(event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
        if(QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture *pinchGesture = static_cast<QPinchGesture*>(gesture);
            const qreal lastFactor = pinchGesture->lastScaleFactor();
            const qreal scaleFactor = pinchGesture->scaleFactor();
            if(lastFactor > 0.0 && scaleFactor > 0.0) {
                applyZoomFactor(scaleFactor / lastFactor);
                gestureEvent->accept(gesture);
                return true;
            }
        }
    }

    return QGraphicsView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent* event)
{    
    QGraphicsView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent* event)
{
    int deltaX = 0;
    int deltaY = 0;

    if(event->buttons() & Qt::LeftButton) {

        if(event->pos().x() < 5) {
            int diff = horizontalScrollBar()->value() - horizontalScrollBar()->minimum();
            if(diff < deltaX)
                deltaX = -diff;
            else
                deltaX = -5;
            
        } else if (event->pos().x() > viewport()->width() - 5) {
            int diff = horizontalScrollBar()->maximum() - horizontalScrollBar()->value();
            if(diff < deltaX)
                deltaX = diff;
            else
                deltaX = 5;
        }

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + deltaX);
        
        if(event->pos().y() < 5) {
            int diff = verticalScrollBar()->value() - verticalScrollBar()->minimum();
            if(diff < deltaY)
                deltaY = -diff;
            else
                deltaY = -5;
            
        } else if( event->pos().y() > viewport()->height() - 5) {
            int diff = verticalScrollBar()->maximum() - verticalScrollBar()->value();
            if(diff < deltaY)
                deltaY = diff;
            else
                deltaY = 5;
        }

        verticalScrollBar()->setValue(verticalScrollBar()->value() + deltaY);
        
        bool isHorizLimit = false;
        isHorizLimit = (horizontalScrollBar()->value() == horizontalScrollBar()->minimum()) ? true : isHorizLimit;
        isHorizLimit = (horizontalScrollBar()->value() == horizontalScrollBar()->maximum()) ? true : isHorizLimit;

        bool isVertLimit = false;
        isVertLimit = (verticalScrollBar()->value() == verticalScrollBar()->minimum()) ? true : isVertLimit;
        isVertLimit = (verticalScrollBar()->value() == verticalScrollBar()->maximum()) ? true : isVertLimit;

        if((deltaX != 0 && !isHorizLimit) || (deltaY != 0 && !isVertLimit))
            emit scrollBarChanged(deltaX, deltaY);
    }
	
	QGraphicsView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void ChartView::wheelEvent(QWheelEvent* event)
{
    if((event->modifiers() & Qt::ControlModifier) || !isTrackpadScrollEvent(event)) {
        zoom(event->angleDelta().y());
        event->accept();
        return;
    }

    QGraphicsView::wheelEvent(event);
}

void ChartView::zoomIn()
{
    zoomLevel((transform().m11()*100) + 5);
    emitZoomLevel();
}

void ChartView::zoomOut()
{
    zoomLevel((transform().m11()*100) - 5);
    emitZoomLevel();
}

void ChartView::zoom(int mouseDelta)
{
    double scroll = mouseDelta / 120;
    int delta = 5 * scroll;
    zoomLevel((transform().m11()*100) + delta);
    emitZoomLevel();
}

void ChartView::zoomLevel(int percent)
{
    qreal pcent = percent / 100.0;
    if(pcent <= 0)
        pcent = 0.01;
    qreal diff = pcent / transform().m11();
    scale(diff, diff);
}

void ChartView::emitZoomLevel()
{
    emit zoomLevelChanged(qRound(transform().m11() * 100.0));
}

void ChartView::applyZoomFactor(qreal factor)
{
    if(factor <= 0.0)
        factor = 0.1;

    zoomLevel(qRound(transform().m11() * factor * 100.0));
    emitZoomLevel();
}
