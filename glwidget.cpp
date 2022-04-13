#include <QDebug>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>

#include "camera.h"
#include "glwidget.h"
#include "raytracer.h"
#include "settings.h"

GLWidget::GLWidget(QWidget *parent, 
                   MyRaytracer::Camera& camera, 
                   MyRaytracer::RayTracer &rayTracer_)
    : QOpenGLWidget(parent),
      imageData_(kSceneWidth, kSceneHeight, QImage::Format_Indexed8),
      camera_(camera),
      rayTracer_(rayTracer_)

{
    setFixedSize(kSceneWidth, kSceneHeight);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocus();

    QVector<QRgb> grayScaleTable;
    for(int i = 0; i < 256; i++)
        grayScaleTable.push_back(qRgb(i,i,i));
    imageData_.setColorTable(grayScaleTable);
    imageData_.fill(0);

    rayTracer_.setFrameBuffer(imageData_.bits());
}

void GLWidget::paintGL()
{
    QPainter painter(this);

    // TODO : research more
    // Call to .bits() seems necessary because it "invalidates" the buffer
    // and drawImage knows that it has something new there.
    // It's not expensive though.
    imageData_.bits();

    rayTracer_.traceFrame();
    painter.drawImage(this->rect(), imageData_);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        prevMouseX_ = event->x();
        prevMouseY_ = event->y();
    }
}

void GLWidget::focusOutEvent(QFocusEvent*)
{
    setFocus();
}

void GLWidget::wheelEvent(QWheelEvent* event)
{
    if (event->delta() > 0)
        camera_.zoomIn();
    else 
        camera_.zoomOut();

    repaint();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {
        camera_.mouseMove(event->x() - prevMouseX_, prevMouseY_ - event->y());
        prevMouseX_ = event->x();
        prevMouseY_ = event->y();

        emit cameraMoved();
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        camera_.move(true, false, false, false);
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        camera_.move(false, true, false, false);
        break;

    case Qt::Key_A:
    case Qt::Key_Left:
        camera_.move(false, false, true, false);
        break;

    case Qt::Key_D:
    case Qt::Key_Right:
        camera_.move(false, false, false, true);
        break;

    case Qt::Key_Plus:
        camera_.zoomIn();
        rayTracer_.setZoomLevel(camera_.getZoom());
        repaint();
        // no need to transform points => don't emit cameraMoved
        return;

    case Qt::Key_Minus:
        camera_.zoomOut();
        rayTracer_.setZoomLevel(camera_.getZoom());
        repaint();
        // no need to transform points => don't emit cameraMoved
        return;

    default:
        // don't emit cameraMoved
        return;
    }

    emit cameraMoved();
}
