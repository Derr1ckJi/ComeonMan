#include "SensorSample_2.h"
#include "ui_SensorSample_2.h"
#include "qcustomplot.h"
#include <QPen>
#include <QSqlDatabase>
#include <QSqlError>

SensorSample_2::SensorSample_2(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SensorSample_2)
{
//    QString path = QDir::currentPath();
//    QApplication::addLibraryPath(path+QString("/plugins"));
//    QPluginLoader loader(path+QString("/plugins/sqldrivers/qsqlite4.dll"));

    ui->setupUi(this);
    init_plots();
    init_serialport();
    init_database();

    for(int i=0; i<8; i++){
        QCheckBox *box = new QCheckBox;
        box->setText("CHANNEL"+QString::number(i+1));
        ui->verticalLayout->addWidget(box);
        boxes->push_back(box);
    }//代码实现复选框创建
    QPushButton *okBtn = new QPushButton("确认");
    ui->verticalLayout->addWidget(okBtn);//按钮创建
    ui->label->setStyleSheet("color:red;");//设置颜色
    ui->channelbox->addItem("请选择通道");
    for(int i=1;i<=8;i++){
        ui->channelbox->addItem(QString::number(i));
    }
//    QPushButton *clearBtn = new QPushButton("清空");
//    ui->verticalLayout->addWidget(clearBtn);

    connect(ui->liveTBTN,&QToolButton::pressed,[=](){
       ui->logTBTN->setChecked(false);
       ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->logTBTN,&QToolButton::pressed,[=](){
        ui->liveTBTN->setChecked(false);
        ui->stackedWidget->setCurrentIndex(1);
    });//stackwidget选择

    connect(ui->searchBtn, SIGNAL(clicked()),this, SLOT(searchEvent()));
    connect(ui->channelbox,SIGNAL(currentIndexChanged(int)),this,SLOT(setcombo(int)));
    connect(okBtn, &QPushButton::clicked, this, &SensorSample_2::chooseEvent);
//    connect(clearBtn, &QPushButton::clicked, this, &SensorSample_2::clearEvent);
}

void SensorSample_2::chooseEvent()
{
    index->clear();
    int number=0;
    for(int i=0; i<8; i++){
        index->push_back(boxes->at(i)->isChecked());
    }
    for(int i=0; i<8; i++){
    if(index->at(i)==true)
        number++;
    }//读取复选框选择数据
    ui->scrollAreaWidgetContents->setFixedWidth(400*number);
    for(int i=0; i<8; i++){
        if(index->at(i)==true && ui->horizontalLayout_3->indexOf(plots->at(i)) == -1){
//            qDebug()<<111;
            plots->at(i)->addGraph();
            QPen pen;
            pen.setColor(QColor(250,200,20,200));
            pen.setWidth(1);
            plots->at(i)->graph(0)->setPen(pen);
            plots->at(i)->graph(0)->setLineStyle(QCPGraph::lsLine);
            plots->at(i)->graph(0)->setScatterStyle(QCPScatterStyle::ssStar);
            plots->at(i)->graph(0)->setBrush(QColor(255,200,20,70));

            plots->at(i)->plotLayout()->insertRow(0);
            plots->at(i)->plotLayout()->
                        addElement(0,0,new QCPTextElement(plots->at(i),
                        "Data from CHANNEL"+QString::number(i+1),QFont("sans", 12, QFont::Bold)));
            plots->at(i)->xAxis->setLabel("Time");
            plots->at(i)->yAxis->setLabel("Values/N");
            plots->at(i)->xAxis->setRange(0,10);
            plots->at(i)->yAxis->setRange(0,100);
            plots->at(i)->axisRect()->setBackground(
                        QPixmap(":/image/schoolbadge.jpg"),true,Qt::KeepAspectRatio);
            plots->at(i)->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom
                                          | QCP::iSelectPlottables);
            plots->at(i)->setFixedSize(400,380);
            ui->horizontalLayout_3->addWidget(plots->at(i),1,Qt::AlignLeft);
            plots->at(i)->show();
        }
        if(index->at(i)==false && ui->horizontalLayout_3->indexOf(plots->at(i)) != (-1))
        {
            plots->at(i)->removeGraph(0);
            plots->at(i)->hide();
            ui->horizontalLayout_3->removeWidget(plots->at(i));
        }
    }
}

void SensorSample_2::init_plots()
{
    for(int i=0; i<8; i++){
        QCustomPlot *plot = new QCustomPlot;
        plots->push_back(plot);
    }
}

bool SensorSample_2::init_serialport(int BaudRate)
{
    com1.setPortName("COM1");
    com1.setBaudRate(BaudRate);
    com1.setDataBits(QSerialPort::Data8);
    com1.setParity(QSerialPort::NoParity);
    com1.setStopBits(QSerialPort::OneStop);
    com1.setFlowControl(QSerialPort::NoFlowControl);

    connect(&com1, &QSerialPort::readyRead, this, &SensorSample_2::NewInput);

    if(com1.open(QIODevice::ReadWrite))
        return true;
    else
    {   QMessageBox::warning(this, "WARNING",
                             "A falure occurred when opening the serialport.");
        return false;}
}

void SensorSample_2::NewInput()
{
    int channel,Fdata;
    QByteArray buf = com1.readAll();
//    qDebug()<<buf;
    QString str = buf;
    QString str1 = str.at(0);
    channel = str1.toInt();
    str[0]=' ';
    (void)str.trimmed();
    Fdata = str.toInt();

//    qDebug()<<channel<<Fdata;//从串口读取数据代码段

    if(channel>=1 && channel<=8 && plots->at(channel-1)->graph(0) != nullptr){
        numbers[channel-1]++;
        if(numbers[channel-1]<=10)
            plots->at(channel-1)->xAxis->setRange(0,10);
        else
            plots->at(channel-1)->xAxis->setRange(numbers[channel-1]-10,-10.0,Qt::AlignRight);
        plots->at(channel-1)->graph(0)->addData(numbers[channel-1], Fdata);
        plots->at(channel-1)->replot();
        ui->channeledit->setText(QString::number(channel));
        ui->valueedit->setText(QString::number(Fdata));
    }else{
        QMessageBox::warning(this,"warning","Wrong input or the channel is not selected!");
    }//利用数据绘图代码段

    //将数据插入数据库代码段
    QSqlQuery query;
    query.prepare("INSERT INTO D"+datetime+"(Channel, shijian, Fdata) values(?,?,?)");
    query.bindValue(0,channel);
    query.bindValue(1,numbers[channel-1]);
    query.bindValue(2,Fdata);
    bool success = query.exec();
    if(success){
//        ui->textBrowser->append("接收到新数据");
//根据端口制订不同输入
    }else {
        ui->textBrowser->append("接收出现错误");
        qDebug()<<query.lastError();
    }
}

void SensorSample_2::init_database()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("DESKTOP_R5TP303");
    db.setDatabaseName(":memory:");
    db.setUserName("DERRICK");//数据库初始化
    QSqlQuery query;

    if(!db.open()){
        QMessageBox::warning(this,"Warning!","数据库连接失败");
        qDebug()<<db.lastError();
    }else{
        ui->textBrowser->append("数据库连接成功");
    }//打开数据库

    QDateTime date;
    datetime = date.currentDateTime().toString("yyyymmddhhmmss");
//    qDebug()<<datetime;
//    query.exec("drop table timeandF");
    if(!query.exec("create table D"+datetime+"(Channel int, shijian int NOT NULL, Fdata int NOT NULL)")){
        QMessageBox::warning(this,"Warning!","数据表创建失败");
        qDebug()<<query.lastError();
    }else{
        ui->textBrowser->append("数据库创建成功");
    }
}

void SensorSample_2::setcombo(int a){
    ui->timebox->clear();
    for(int i=1;i<=numbers[a-1];i++)
    {
        ui->timebox->addItem(QString::number(i));
    }
}

void SensorSample_2::searchEvent()
{
    QString Channel = ui->channelbox->currentText();
    QString Time = ui->timebox->currentText();
//    qDebug()<<Channel<<Time;
    QSqlQuery query;
    QString order = QString("select Fdata from D"+datetime+
      " where Channel = '%1' and shijian = '%2'").arg(Channel.toInt()).arg(Time.toInt());
    bool search = query.exec(order);
    if(!search){
        ui->textBrowser->append("查询失败");
        qDebug()<<query.lastError();
    }else{
        if(query.first()) {
//            qDebug()<<111;
            int Fdata = query.value(0).toInt();
            ui->textBrowser->append("查询结果：");
            ui->textBrowser->append("Channel="+Channel+
                                        " time="+Time+
                                        " value="+QString::number(Fdata));

        }
    }
}

SensorSample_2::~SensorSample_2()
{
    delete ui;
}

