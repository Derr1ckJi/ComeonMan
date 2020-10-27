#ifndef SENSORSAMPLE_2_H
#define SENSORSAMPLE_2_H

#include <QDialog>
#include <QCheckBox>
#include "qcustomplot.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui { class SensorSample_2; }
QT_END_NAMESPACE

class SensorSample_2 : public QDialog
{
    Q_OBJECT

public:
    SensorSample_2(QWidget *parent = nullptr);
    ~SensorSample_2();
    void init_plots();
    bool init_serialport(int BaudRate=9600);
    void init_database();
public slots:
    void chooseEvent();
    void NewInput();
    void setcombo(int);
    void searchEvent();

private:
    Ui::SensorSample_2 *ui;
    QVector<QCheckBox*> boxes[8];
    QVector<bool> index[8];
    QVector<QCustomPlot*> plots[8];
    QPushButton *okBtn;
    QPushButton *clearBtn;
    QSerialPort com1;
    int numbers[8] = {0,0,0,0,0,0,0,0};
    QString datetime;


};
#endif // SENSORSAMPLE_2_H
