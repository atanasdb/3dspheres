#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QImage>


namespace MyRaytracer 
{
    class Camera;
    class RayTracer;
    class SceneData;
};

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent, MyRaytracer::Camera &camera, MyRaytracer::RayTracer &rayTracer_);

signals:
    void cameraMoved();

protected:
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent* event);
    void focusOutEvent(QFocusEvent* event);

private:
    MyRaytracer::Camera &camera_;
    MyRaytracer::RayTracer &rayTracer_;
    
    QImage imageData_;
    
    int prevMouseX_;
    int prevMouseY_;
};


#endif // GLWIDGET_H
