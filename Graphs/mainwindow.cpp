#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "owens.h"
#include <cmath>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <iomanip>
using namespace std;

double skewness;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pictHeight = 370;
    pictWidth = 540;
    step = 0.1;
    leftX = -50; rightX = 50;
    leftY = -50; rightY = 50;
    ui->chooseDistribution->addItem("Normal");
    ui->chooseDistribution->addItem("Skew-normal");
    ui->chooseLimitsType->addItem("Symmetric");
    ui->chooseLimitsType->addItem("Best up");
    drawGraph();
}

MainWindow::~MainWindow()
{
    delete ui;
}

double Normal(double x, double a, double sigma) //using cumulative distribution
{
    return 0.5 * (1 + erf((x - a) / sigma / sqrt(2)));
}

double SkewNormal(double x, double a, double sigma) //using cumulative distribution
{
    return Normal(x, a, sigma) - 2 * t((x - a)/sigma, skewness);
}

double UpLimit(double Distribution(double, double, double), double estimate, double CL, double a0, double sigma)
{
    estimate = max(estimate, a0);

    double p0 = estimate + 2;
    double p1 = estimate;
    double p2;

    double alpha = 1.0 - (1.0 - CL) / 2;

    while (fabs(Distribution(p1, estimate, sigma)-alpha) > 0.001) {
        p2 = p1 - (Distribution(p1, estimate, sigma)-alpha)*(p1-p0)/(Distribution(p1, estimate, sigma)-Distribution(p0, estimate, sigma));
        p0 = p1;
        p1 = p2;
    }

    return p1;
}

double DownLimit(double Distribution(double, double, double), double estimate, double CL, double a0, double sigma)
{
    double p0 = estimate - 2;
    double p1 = estimate;
    double p2;

    double q0 = estimate - 2;
    double q1 = estimate;
    double q2;

    double alpha = (1.0 - CL) / 2;
    double beta = 2 * alpha;

    while (fabs(Distribution(p1, estimate, sigma) - alpha) > 0.001) {
        p2 = p1 - (Distribution(p1, estimate, sigma) - alpha)*(p1-p0)/(Distribution(p1, estimate, sigma) - Distribution(p0, estimate, sigma));
        p0 = p1;
        p1 = p2;
    }

    while (fabs(Distribution(q1, estimate, sigma) - beta) > 0.001) {
            q2 = q1 - (Distribution(q1, estimate, sigma) - beta)*(q1 - q0) / (Distribution(q1, estimate, sigma) - Distribution(q0, estimate, sigma));
            q0 = q1;
            q1 = q2;
        }

    double r = UpLimit(Distribution, a0, CL, a0, sigma);

    if (a0 > q2) return a0;
    if (q2 > r) return max(p2, r);
    return q2;
}

double BestUpLimit(double Distribution(double, double, double), double estimate, double CL, double a0, double sigma)
{
    estimate = max(estimate, a0);

    double p0 = estimate + 2;
    double p1 = estimate;
    double p2;

    double alpha = CL;

    while (fabs(Distribution(p1, estimate, sigma)-alpha) > 0.001) {
        p2 = p1 - (Distribution(p1, estimate, sigma)-alpha)*(p1-p0)/(Distribution(p1, estimate, sigma)-Distribution(p0, estimate, sigma));
        p0 = p1;
        p1 = p2;
    }

    return p1;
}

double BestUpDownLimit(double Distribution(double, double, double), double estimate, double CL, double a0, double sigma)
{
    double p0 = estimate - 2;
    double p1 = estimate;
    double p2;

    double alpha = 1.0 - CL;

    while (fabs(Distribution(p1, estimate, sigma) - alpha) > 0.001) {
        p2 = p1 - (Distribution(p1, estimate, sigma) - alpha)*(p1-p0)/(Distribution(p1, estimate, sigma) - Distribution(p0, estimate, sigma));
        p0 = p1;
        p1 = p2;
    }

    double r = BestUpLimit(Distribution, a0, CL, a0, sigma);

    if (a0 > p2) return a0;
    return min(r, p2);
}


double MainWindow::f(double x)
{
    return (-1)*DownLimit(Distr, x, CL, parameter, sigma);
}

double MainWindow::f1(double x)
{
    return (-1)*UpLimit(Distr, x, CL, parameter, sigma);
}

double MainWindow::f2(double x)
{
    return (-1)*BestUpLimit(Distr, x, CL, parameter, sigma);
}

double MainWindow::f3(double x)
{
    return (-1)*BestUpDownLimit(Distr, x, CL, parameter, sigma);
}

void MainWindow::recountPixels()
{
    onePixelX = 540.0/(rightX-leftX);
    onePixelY = 370.0/(rightY-leftY);
    Ox = -1*leftX; Oy = rightY;
}

void MainWindow::getData()
{
    bool isNumber = true;

    leftX = ui->inputLeftX->value();
    rightX = ui->inputRightX->value();
    if (leftX > rightX) throw 1;

    leftY = ui->inputLeftY->value();
    rightY = ui->inputRightY->value();
    if (leftY > rightY) throw 1;

    step = 1.0/ui->inputAccuracy->value();
    CL = ui->inputCL->value();

    if(ui->inputParameter->text().isEmpty()) throw 3;
    parameter = ui->inputParameter->text().toDouble(&isNumber);
    if (!isNumber) throw 5;

    if(ui->inputEstimate->text().isEmpty()) throw 3;
    estimate = ui->inputEstimate->text().toDouble(&isNumber);
    if (!isNumber) throw 5;

    if(ui->inputSigma->text().isEmpty()) throw 3;
    sigma = ui->inputSigma->text().toDouble(&isNumber);
    if (!isNumber) throw 5;
    if (sigma < 0.5) throw 2;

    if(ui->inputSkewness->text().isEmpty()) throw 3;
    skewness = ui->inputSkewness->text().toDouble(&isNumber);
    if (!isNumber) throw 5;
    if (skewness < -3 || skewness > 3) throw 4;

    if (ui->chooseDistribution->currentText() == "Normal") Distr = Normal;
    else Distr = SkewNormal;
}

void MainWindow::countLimits()
{
    QString Limits = "Estimate: "+QString::number(estimate, 'f', 2)+"            Down limit: ";
    if (ui->chooseLimitsType->currentText() == "Symmetric") {
        Limits.append(QString::number(DownLimit(Distr, estimate, CL, parameter, sigma), 'f', 2));
        Limits.append("            Up limit: ");
        Limits.append(QString::number(UpLimit(Distr, estimate, CL, parameter, sigma),'f', 2));
    } else {
        Limits.append(QString::number(BestUpDownLimit(Distr, estimate, CL, parameter, sigma),'f', 2));
        Limits.append("            Up limit: ");
        Limits.append(QString::number(BestUpLimit(Distr, estimate, CL, parameter, sigma),'f', 2));
    }
    ui->limits->setText(Limits);
}
void MainWindow::drawGraph(bool notEmpty)
{
    QPixmap graph(540,370);
    QPainter paint;
    paint.begin(&graph);
    paint.eraseRect(0,0,540,370);
    if(!notEmpty) {
        paint.end();
        ui->outputGraph->setPixmap(graph);
        return;
    }

    if (leftX <= 0 && rightX >= 0) {
        paint.drawLine(Ox*onePixelX,0,Ox*onePixelX,pictHeight);
        paint.drawText(Ox*onePixelX+15, 15, "θ");
    }
    if (leftY <= 0 && rightY >= 0) {
        paint.drawLine(0,Oy*onePixelY,pictWidth,Oy*onePixelY);
        paint.drawText(pictWidth-15, Oy*onePixelY-15, "θ");
        paint.drawText(pictWidth-16, Oy*onePixelY-22, "^");
    }
    paint.setPen(QPen(Qt::black,3));
    if (leftY <= 0 && rightY >= 0)
        for(double i = leftX;i<=rightX;i+=1.0) {
            paint.drawPoint((i+Ox)*onePixelX,Oy*onePixelY);
            paint.drawText((i+Ox)*onePixelX-15,Oy*onePixelY+15, QString::number(i, 'g', 2));
        }
    if (leftX <= 0 && rightX >= 0)
        for(double i = -1*rightY;i<=-1*leftY;i+=1.0) {
            paint.drawPoint(Ox*onePixelX,(i+Oy)*onePixelY);
            paint.drawText(Ox*onePixelX-15, (i+Oy)*onePixelY+15, QString::number(-1*i, 'g', 2));
        }

    paint.setPen(QPen(Qt::green,1,Qt::SolidLine));
    paint.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath p[4];
    bool first[4] = {1,1,1,1};

    for(double i = (double)leftX+step;i<=(double)rightX;i+=step) {
            if(!isnan(f(i))) {
                if(first[0]) {
                    p[0].moveTo((i+Ox)*onePixelX,(f(i)+Oy)*onePixelY);
                    first[0] = false;
                }
                else
                    p[0].lineTo((i+Ox)*onePixelX,(f(i)+Oy)*onePixelY);
            }
            if(!isnan(f1(i))) {
                if(first[1]) {
                    p[1].moveTo((i+Ox)*onePixelX,(f1(i)+Oy)*onePixelY);
                    first[1] = false;
                }
                else
                    p[1].lineTo((i+Ox)*onePixelX,(f1(i)+Oy)*onePixelY);
            }
            if(!isnan(f2(i))) {
                if(first[2]) {
                    p[2].moveTo((i+Ox)*onePixelX,(f2(i)+Oy)*onePixelY);
                    first[2] = false;
                }
                else
                    p[2].lineTo((i+Ox)*onePixelX,(f2(i)+Oy)*onePixelY);
            }
            if(!isnan(f3(i))) {
                if(first[3]) {
                    p[3].moveTo((i+Ox)*onePixelX,(f3(i)+Oy)*onePixelY);
                    first[3] = false;
                }
                else
                    p[3].lineTo((i+Ox)*onePixelX,(f3(i)+Oy)*onePixelY);
            }
    }
    if(ui->chooseLimitsType->currentText() == "Symmetric") {
        paint.setPen(QPen(Qt::darkMagenta,1,Qt::SolidLine));
        paint.drawPath(p[0]);
        paint.drawPath(p[1]);
    }
    if(ui->chooseLimitsType->currentText() == "Best up") {
        paint.setPen(QPen(Qt::magenta,1,Qt::SolidLine));
        paint.drawPath(p[2]);
        paint.drawPath(p[3]);
    }

    if(ui->chooseLimitsType->currentText() == "Symmetric") {
        paint.setPen(QPen(Qt::black,3));
        paint.drawPoint((estimate+Ox)*onePixelX,(Oy-DownLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
        paint.drawPoint((estimate+Ox)*onePixelX,(Oy-UpLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
        paint.setPen(QPen(Qt::green,1,Qt::SolidLine));
        paint.drawLine((estimate+Ox)*onePixelX,(Oy-UpLimit(Distr, estimate, CL, parameter, sigma))*onePixelY,(estimate+Ox)*onePixelX,(Oy-DownLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
    } else {
        paint.setPen(QPen(Qt::black,3));
        paint.drawPoint((estimate+Ox)*onePixelX,(Oy-BestUpDownLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
        paint.drawPoint((estimate+Ox)*onePixelX,(Oy-BestUpLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
        paint.setPen(QPen(Qt::green,1,Qt::SolidLine));
        paint.drawLine((estimate+Ox)*onePixelX,(Oy-BestUpLimit(Distr, estimate, CL, parameter, sigma))*onePixelY,(estimate+Ox)*onePixelX,(Oy-BestUpDownLimit(Distr, estimate, CL, parameter, sigma))*onePixelY);
    }

    paint.end();
    ui->outputGraph->setPixmap(graph);
    return;
}

void MainWindow::on_exit_clicked()
{
    this->close();
}

void MainWindow::on_clear_clicked()
{
    recountPixels();
    drawGraph();
}

void MainWindow::on_draw_clicked()
{
    try {
     getData();
    } catch (int signal) {
        if (signal == 1) {
            QMessageBox::warning(this, "Warning!", "The right border should be more or equal left border!");
            return;
        }
        if (signal == 2) {
            QMessageBox::warning(this, "Warning!", "The sigma should be more or equal 0.5!\n(Program limit)");
            return;
        }
        if (signal == 3) {
            QMessageBox::warning(this, "Warning!", "There should be empty boxes!");
            return;
        }
        if (signal == 4) {
            QMessageBox::warning(this, "Warning!", "The skewness should be more or equal -3\nand less or equal 3!\n(Program limit)");
            return;
        }
        if (signal == 5) {
            QMessageBox::warning(this, "Warning!", "Uncorrect input of numbers.\nDon't use commas and spaces.");
            return;
        }
    }
    recountPixels();
    drawGraph(1);
    countLimits();
}

void MainWindow::on_savePicture_clicked()
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    QString name;
   if(date.day()<10)
        name += "0";
    name += QString::number(date.day())+".";
    if(date.month()<10)
        name += "0";
    name += QString::number(date.month())+".";
    name += QString::number(date.year())+"_";
    if(time.hour()<10)
        name += "0";
    name += QString::number(time.hour())+"-";
    if(time.minute()<10)
        name += "0";
    name += QString::number(time.minute())+"-";
    if(time.second()<10)
        name += "0";
    name += QString::number(time.second());
    QFile file(name+".png");
    qDebug() << name;
    file.open(QIODevice::WriteOnly);
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Ok);
    if(ui->outputGraph->pixmap()->save(&file,"PNG")) {
        msgBox.setText("Saved to program folder with name: "+name+".png");
        msgBox.setWindowTitle("Saved!");
    }
    else {
        msgBox.setText("Error saving.");
        msgBox.setWindowTitle("Error!");
    }
    msgBox.exec();
}

void MainWindow::on_saveBelt_clicked()
{
    try {
        getData();
    } catch (int signal) {
        if (signal == 1) {
            QMessageBox::warning(this, "Warning!", "The right border should be more or equal left border!");
            return;
        }
        if (signal == 2) {
            QMessageBox::warning(this, "Warning!", "The sigma should be more or equal 0.5!\n(Program limit)");
            return;
        }
        if (signal == 3) {
            QMessageBox::warning(this, "Warning!", "There should be empty boxes!");
            return;
        }
        if (signal == 4) {
            QMessageBox::warning(this, "Warning!", "The skewness should be more or equal -3\nand less or equal 3!\n(Program limit)");
            return;
        }
        if (signal == 5) {
            QMessageBox::warning(this, "Warning!", "Uncorrect input of numbers.\nDon't use commas and spaces.");
            return;
        }
    }
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    QString name;
   if(date.day()<10)
        name += "0";
    name += QString::number(date.day())+".";
    if(date.month()<10)
        name += "0";
    name += QString::number(date.month())+".";
    name += QString::number(date.year())+"_";
    if(time.hour()<10)
        name += "0";
    name += QString::number(time.hour())+"-";
    if(time.minute()<10)
        name += "0";
    name += QString::number(time.minute())+"-";
    if(time.second()<10)
        name += "0";
    name += QString::number(time.second());
    QFile file(name+".txt");
    qDebug() << name;
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << "CL: " << fixed <<
           qSetRealNumberPrecision(2) << qSetFieldWidth(6) << left << CL << "Sigma: " << fixed << qSetFieldWidth(6) << left << sigma;
    if(ui->chooseDistribution->currentText() == "Normal")
        out << endl;
    else
        out << " Skewness: " << skewness << endl;
    for (double i = (double)leftX; i <= (double)rightY; i+=0.1)
        if(ui->chooseLimitsType->currentText() == "Symmetric")
            out << fixed << qSetRealNumberPrecision(1) << qSetFieldWidth(6) << left << i  <<
                   qSetRealNumberPrecision(2) << qSetFieldWidth(6) << left << DownLimit(Distr, i, CL, parameter, sigma) <<
                   qSetRealNumberPrecision(2) << qSetFieldWidth(6) << left << UpLimit(Distr, i, CL, parameter, sigma) << endl;
        else
            out << fixed << qSetRealNumberPrecision(1) << qSetFieldWidth(6) << left << i <<
                   qSetRealNumberPrecision(2) << qSetFieldWidth(6) << left << BestUpDownLimit(Distr, i, CL, parameter, sigma) <<
                   qSetRealNumberPrecision(2) << qSetFieldWidth(6) << left << BestUpLimit(Distr, i, CL, parameter, sigma) << endl;

    QMessageBox msgBox;
    msgBox.setText("Saved to program folder with name: "+name+".txt");
    msgBox.setWindowTitle("Saved!");
    msgBox.exec();
}
