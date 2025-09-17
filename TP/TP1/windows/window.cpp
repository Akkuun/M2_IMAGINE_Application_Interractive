#include "glwidget.h"
#include "window.h"
#include "mainwindow.h"
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>

Window::Window(MainWindow *mw)
    : mainWindow(mw)
{
    glWidget = new GLWidget;

    // A completer, connecter les sliders de cette classe avec le glWidget pour mettre à jour la rotation
    //  et inversement

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(glWidget);

    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);
    dockBtn = new QPushButton(tr("Undock"), this);
    connect(dockBtn, &QPushButton::clicked, this, &Window::dockUndock);
    mainLayout->addWidget(dockBtn);

    setLayout(mainLayout);

    setWindowTitle(tr("Qt OpenGL"));
}

QSlider *Window::createSlider()
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}
// function to set rotation in GLWidget when slider is moved
void Window::setXRotation(int angle)
{
    glWidget->setXRotation(angle);
}
void Window::setYRotation(int angle)
{
    glWidget->setYRotation(angle);
}
void Window::setZRotation(int angle)
{
    glWidget->setZRotation(angle);
}

// functions to update sliders when rotation changes in GLWidget
void Window::updateXSlider(int angle)
{
    xSlider->setValue(angle);
}
void Window::updateYSlider(int angle)
{
    ySlider->setValue(angle);
}
void Window::updateZSlider(int angle)
{
    zSlider->setValue(angle);
}

void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void Window::dockUndock()
{
    if (parent())
    {
        setParent(0);
        setAttribute(Qt::WA_DeleteOnClose);
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int screenHeight = screenGeometry.height();
        int screenWidth = screenGeometry.width();
        move(screenWidth / 2 - width() / 2,
             screenHeight / 2 - height() / 2);
        dockBtn->setText(tr("Dock"));
        show();
    }
    else
    {
        if (!mainWindow->centralWidget())
        {
            if (mainWindow->isVisible())
            {
                setAttribute(Qt::WA_DeleteOnClose, false);
                dockBtn->setText(tr("Undock"));
                mainWindow->setCentralWidget(this);
            }
            else
            {
                QMessageBox::information(0, tr("Cannot dock"), tr("Main window already closed"));
            }
        }
        else
        {
            QMessageBox::information(0, tr("Cannot dock"), tr("Main window already occupied"));
        }
    }
}
