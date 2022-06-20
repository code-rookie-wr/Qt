#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "qcustomplot.h"
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class SerialPort; }
QT_END_NAMESPACE

class SerialPort : public QMainWindow
{
    Q_OBJECT

public:
    SerialPort(QWidget *parent = nullptr);
    ~SerialPort();

    void serialPort_readyread();
    QSerialPort serial;
    QSerialPortInfo info;
    double x=0.05;//数据读取间隔(s)
    double MaxY=2;
    double MinY=1.5;
    QVector<double> data;
    QVector<double> peaks;
    double IBI=0;//记录两次脉搏信号的间隔时间
    int IBM=0;//记录1min内心跳的次数



private slots:
    void on_btn_checkport_clicked();

    void on_btn_openport_clicked();

    void on_btn_close_clicked();

    void on_btn_clear_clicked();

private:
    Ui::SerialPort *ui;
};
#endif // SERIALPORT_H
