#include "mainwindow.h"
#include "window.h"
#include "glwidget.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
MainWindow::MainWindow()
{
    QMenuBar *menuBar = new QMenuBar;
    QMenu *menuWindow = menuBar->addMenu(tr("&Window"));
    QAction *addNew = new QAction(menuWindow);
    addNew->setText(tr("Add new window"));
    menuWindow->addAction(addNew);
    connect(addNew, &QAction::triggered, this, &MainWindow::onAddNew);

    // menu mesh

    QMenu *menuMesh = menuBar->addMenu(tr("&Mesh"));
    QAction *addNewMesh = new QAction(menuMesh);
    addNewMesh->setText(tr("Add new mesh"));
    menuMesh->addAction(addNewMesh);
    connect(addNewMesh, &QAction::triggered, this, &MainWindow::onAddNewMesh);

    setMenuBar(menuBar);

    onAddNew();
}

void MainWindow::onAddNewMesh()
{
    // On récupère la fenetre
    Window *window = qobject_cast<Window *>(centralWidget());
    if (!window)
    {
        QMessageBox::information(this, tr("No Window"),
                                 tr("Please add a window first before adding a mesh."));
        return;
    }
    else
    {
        // Sélectionner un fichier .off
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open OFF File"),
                                                        "",
                                                        tr("OFF Files (*.off);;All Files (*)"));

        if (!fileName.isEmpty())
        {
            std::cout << "Loading mesh from file: " << fileName.toStdString() << std::endl;
            window->getGLWidget()->loadMesh(fileName);
        }
    }
}

void MainWindow::onAddNew()
{
    if (!centralWidget())
        setCentralWidget(new Window(this));
    else
        QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}
