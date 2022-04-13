#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <glm/vec3.hpp>

#include "camera.h"
#include "raytracer.h"
#include "scenedata.h"

class GLWidget;
class QCheckBox;
class QLabel;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);

private slots:
    void cameraMoved(); 
    void createSceneStructure(int levels);
    void antiAliasingChecked(bool state);
    
private:
    GLWidget *glWidget;
    QLabel *cameraPosLbl;      
    QLabel *cameraRotationLbl;
    QLabel *spheresCountLbl;
    QSpinBox *levelsSpin;
    QCheckBox *antiAliasingCheckbox;

    MyRaytracer::Camera camera_;
    MyRaytracer::RayTracer rayTracer_;
    MyRaytracer::SceneData sceneData_;
       
    void setupWidgets();
};

#endif // MAINWINDOW_H
