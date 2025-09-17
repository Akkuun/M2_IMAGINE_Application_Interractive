
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE

class GLWidget;
class MainWindow;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(MainWindow *mw);

    GLWidget *getGLWidget() { return glWidget; }

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void dockUndock();
    // slider slots to change rotation
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    // signals to update sliders when rotation changes in GLWidget
    void updateXSlider(int angle);
    void updateYSlider(int angle);
    void updateZSlider(int angle);

private:
    QSlider *createSlider();

    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
};

#endif
