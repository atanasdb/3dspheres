#include <QApplication>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QTimer>

#include <glm/glm.hpp>

#include "glwidget.h"
#include "mainwindow.h"
#include "raytracer.h"
#include "settings.h"
#include "scenedata.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      camera_(kSceneWidth, kSceneHeight, glm::dvec3(0, 0, -5)),
      rayTracer_(sceneData_)
{
    setupWidgets();
    setWindowTitle(tr("Sphereflake renderer"));
    
    levelsSpin->setRange(1, 7);
    levelsSpin->setValue(7);
    antiAliasingCheckbox->setChecked(true);

    glWidget->setFocus();
    connect(glWidget, SIGNAL(cameraMoved()), this, SLOT(cameraMoved()));
    connect(levelsSpin, SIGNAL(valueChanged(int)), this, SLOT(createSceneStructure(int)));
    connect(antiAliasingCheckbox, SIGNAL(toggled(bool)), this, SLOT(antiAliasingChecked(bool)));

    rayTracer_.setAntiAliasing(true);
    rayTracer_.setZoomLevel(kSceneWidth > kSceneHeight ? kSceneWidth : kSceneHeight);

    sceneData_.setLightPos(glm::dvec3(-0.6, 5, -10));
    createSceneStructure(7);
}

void MainWindow::setupWidgets()
{
    QWidget *mainWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QGroupBox *toolboxWidget = new QGroupBox;

    QVBoxLayout *vbox = new QVBoxLayout;
    cameraPosLbl = new QLabel();
    cameraRotationLbl = new QLabel();
    vbox->addWidget(cameraPosLbl);
    vbox->addWidget(cameraRotationLbl);

    levelsSpin = new QSpinBox();
    levelsSpin->setFocusPolicy(Qt::NoFocus);
    vbox->addWidget(new QLabel("Levels: "));
    vbox->addWidget(levelsSpin);
    antiAliasingCheckbox = new QCheckBox("Anti-aliasing");
    vbox->addWidget(antiAliasingCheckbox);
    spheresCountLbl = new QLabel("Spheres: ");
    vbox->addWidget(spheresCountLbl);
    
    vbox->addStretch();
    toolboxWidget->setLayout(vbox);

    glWidget = new GLWidget(this, camera_, rayTracer_);
    mainLayout->addWidget(glWidget);
    mainLayout->addWidget(toolboxWidget);

    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

void MainWindow::cameraMoved() 
{
    sceneData_.transformPoints(camera_.getViewMatrix());
    glWidget->repaint();

    cameraPosLbl->setText(QString("camera pos : [%1, %2, %3]").
        arg(QString::number(camera_.getPosition().x, 'f', 2)).
        arg(QString::number(camera_.getPosition().y, 'f', 2)).
        arg(QString::number(camera_.getPosition().z, 'f', 2)));

    cameraRotationLbl->setText(QString("camera rot : [%1, %2, %3]").
        arg(QString::number(camera_.getRotation().x, 'f', 2)).
        arg(QString::number(camera_.getRotation().y, 'f', 2)).
        arg(QString::number(camera_.getRotation().z, 'f', 2)));
}

void MainWindow::createSceneStructure(int levels)
{
    if (sceneData_.buildStructure(levels)) {
        camera_.reset(glm::dvec3(0, 0, -5));
        cameraMoved();
    } else {
        QMessageBox::information(this, tr("Warning"), 
            tr("Failed to create a structure with %1 levels").arg(levels));
    }

    spheresCountLbl->setText(QString("Spheres : %1").arg(sceneData_.getSpheresCount()));
}

void MainWindow::antiAliasingChecked(bool state)
{
    rayTracer_.setAntiAliasing(state);
    glWidget->repaint();
}
