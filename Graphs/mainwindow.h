#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void drawGraph(bool notEmpty = 0);
    void recountPixels();
    void getData();
    void countLimits();
    friend double Normal(double x, double a);
    friend double SkewNormal(double x, double a);
    friend double UpLimit(double Distribution(double, double), double estimate, double CL, double a0);
    friend double DownLimit(double Distribution(double, double), double estimate, double CL, double a0);
    double f(double x);
    double f1(double x);
    double f2(double x);
    double f3(double x);

private slots:
    void on_exit_clicked();

    void on_clear_clicked();

    void on_draw_clicked();

    void on_savePicture_clicked();

    void on_saveBelt_clicked();

private:
    Ui::MainWindow *ui;
    int leftX,rightX;
    int leftY,rightY;
    int pictWidth,pictHeight;
    double step;
    double onePixelX,onePixelY;
    double Ox,Oy;
    double CL;
    double estimate;
    double sigma;
    double parameter;
    double (*Distr)(double, double, double);
};

#endif // MAINWINDOW_H
