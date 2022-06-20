#include "serialport.h"
#include "ui_serialport.h"
#include <QSerialPortInfo>
#include <QPushButton>
#include <QDebug>
#include <QSerialPort>
#include <QTimer>
#include <QFile>

SerialPort::SerialPort(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SerialPort)
{
    ui->setupUi(this);
    //固定窗口大小
    setFixedSize(836,344);

    connect(&serial,&QSerialPort::readyRead,this,&SerialPort::serialPort_readyread);

    //图表初始化设置
    //设置背景
    ui->widget->setBackground(QBrush(Qt::white));
    //在坐标轴右侧和上方画线
    ui->widget->axisRect()->setupFullAxesBox();
    //设置图例字体和大小
    ui->widget->legend->setFont(QFont("Helvetica",9));
    //使能图例可见
    ui->widget->legend->setVisible(false);
    //设置X轴文字标注
    ui->widget->xAxis->setLabel("时间");
    //设置Y轴文字标注
    ui->widget->yAxis->setLabel("电压值");

    //设置Y轴范围
    ui->widget->yAxis->setRange(MinY,MaxY);

    ui->widget->addGraph();
    //设置画笔
    QPen pen(Qt::red,1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    ui->widget->graph(0)->setPen(pen);
    ui->widget->graph(0)->setName("脉搏曲线");
}

SerialPort::~SerialPort()
{
    delete ui;
}
void SerialPort::serialPort_readyread()
{
    //从接收缓冲区中读取数据
    QByteArray buffer = serial.readAll();
    double volt = buffer.toDouble();
    double threshold=0.2;
    volt = volt*3.3/4096;
//    qDebug()<<volt;

    //存取volt 15s 的数据
    if(data.size()>=200)
    {
        data.pop_front();
    }
    if(data.size()<200)
    {
        data.push_back(volt);
    }

    //记录脉搏峰值对应的时间
    if(data.size()>=3)
    {
        if( (data[data.size()-2]-data[data.size()-3]>threshold || data[data.size()-2]-data[data.size()-1]>threshold) && (data[data.size()-2]>data[data.size()-3]) && (data[data.size()-2]>data[data.size()-1]) )
        {
            peaks.push_back(x);
        }
    }

    //将峰值个数维持在16个以内
    if(peaks.size()>16)
    {
        peaks.pop_front();
    }

    double temp=0;//暂时存储两峰值的间隔
    if(peaks.size()==16)
    {
        for(int i=0; i<peaks.size();i+=2)
        {
            temp = temp+peaks[i+1]-peaks[i];
        }
        //计算IBI
        IBI=temp/peaks.size()*2;
        //算出心率
        IBM = round(60/IBI);
    }

    //设置动态Y轴
    if(volt > MaxY)
    {
        MaxY = volt;
        MinY = MaxY-0.8;
        ui->widget->yAxis->setRange(MinY,MaxY);
    }
    if(volt < MinY)
    {
        MinY = volt;
        MaxY = MinY+0.8;
        ui->widget->yAxis->setRange(MinY,MaxY);
    }

    QString str = QString("%1").arg(volt);
    ui->lbl_volt->setText(str);
    ui->lbl_pulse->setText(QString::number(IBM));
    ui->widget->graph(0)->addData(x,volt);
    //绘图超出默认范围，x轴右移
    if(x>5)
    {
        ui->widget->xAxis->setRange(x,5,Qt::AlignRight);
    }
    ui->widget->replot();
    //延时
    x+=0.1;
    //将数据储存到txt文件中
    //创建文件
    QFile file("D:/QtProject/SerialPort/data.txt");
    file.open(QIODevice::Append);
    QString voltage = QString::number(volt);
    file.write(voltage.toUtf8());
    file.write(" ");
}
void SerialPort::on_btn_checkport_clicked()
{
    ui->portnameBox->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->portnameBox->addItem(info.portName());
    }
}




void SerialPort::on_btn_openport_clicked()
{
    if(ui->btn_openport->text()==QString("打开串口"))
    {
        //设置串口名
        serial.setPortName(ui->portnameBox->currentText());
        //设置波特率
        serial.setBaudRate(ui->baudrateBox->currentText().toInt());
        //设置数据位数
        serial.setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        serial.setParity(QSerialPort::NoParity);
        //设置停止位
        serial.setStopBits((QSerialPort::OneStop));
        //设置流控制
        serial.setFlowControl(QSerialPort::NoFlowControl);
        //打开串口
        serial.open(QIODevice::ReadOnly);
        //控件失能
        ui->portnameBox->setEnabled(false);
        ui->baudrateBox->setEnabled(false);
        ui->btn_checkport->setEnabled(false);
        //改变按钮状态
        ui->btn_openport->setText("关闭串口");
        return;
    }
    if(ui->btn_openport->text()==QString("关闭串口"))
    {
        //关闭串口
        serial.close();
        //控件使能
        ui->portnameBox->setEnabled(true);
        ui->baudrateBox->setEnabled(true);
        ui->btn_checkport->setEnabled(true);
        ui->btn_openport->setText("打开串口");
        return;
    }
}


void SerialPort::on_btn_close_clicked()
{
    this->close();
}


void SerialPort::on_btn_clear_clicked()
{
    ui->widget->graph(0)->data().data()->clear();
    x=0;
    data.clear();
    peaks.clear();
    IBM=0;
    ui->widget->xAxis->setRange(0,5);
    QFile file("D:/QtProject/SerialPort/data.txt");
    file.open(QIODevice::WriteOnly);
    file.write(" ");
}

