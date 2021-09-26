#define BLINKER_WIFI                       //官方wifi协议库
#define BLINKER_MIOT_LIGHT                 //小爱灯类库
#define BLINKER_DUEROS_LIGHT               //小度灯类库
#define BLINKER_PRINT Serial               //串口协议库
#include <ESP8266WiFi.h>
#include <Blinker.h>                       //官方库

char auth[] = ".......";
char ssid[] = "AX3600";
char pswd[] = "shen0000.";
bool wsState;


//软重启定义
uint32_t con_time = 0;                      //断网记时
int con_flag = 0;                           //断网标记，1为断网
void(*resetFunc) (void) = 0;                //定义一个reset函数（用于重启）
int relay = 0;                              //继电器输出
int LocalSwitch = 2;                        //本地轻触开关，io2与gnd接时动作
//新建组件对象
BlinkerButton ButtonOne("btn-abc");           //设置app按键的键名

//app上按下按键即会执行该函数app里按键有2种模式3钟不同样式，下面写出所有的开关状态。
void ButtonOne_callback(const String & state)  //用state来存储组键的值按键 : "tap"(点按); "pre"(长按); "pup"(释放)开关 : "on"(打开); "off"(关闭)
{
  BLINKER_LOG("app操作了11!: ", state);       
  if (digitalRead(relay) == LOW) {              
    BLINKER_LOG("灭灯!");                    
    digitalWrite(relay, HIGH);
    ButtonOne.color("#00FFFF");                 //设置app按键是深蓝色
    ButtonOne.text("灭");
    ButtonOne.print("off");                     //反馈回按键状态是开
    wsState = false;
  }
  else if (digitalRead(relay) == HIGH) {     
    BLINKER_LOG("亮灯!");                         
    digitalWrite(relay, LOW);
    ButtonOne.color("#0000FF");                 //设置app按键是浅蓝色
    ButtonOne.text("亮");
    ButtonOne.print("on");                      //反馈回按键状态是关
    wsState = true;
  }
}

//如果小爱有对设备进行操作就执行下面
void miotPowerState(const String & state)
{
  BLINKER_LOG("小爱语音操作!");
  if (state == BLINKER_CMD_ON) {
    digitalWrite(relay, LOW);
    BlinkerMIOT.powerState("on");
    BLINKER_LOG("亮灯!");
    BlinkerMIOT.print();
    wsState = true;
  }
  else if (state == BLINKER_CMD_OFF) {
    digitalWrite(relay, HIGH);
    BlinkerMIOT.powerState("off");
    BLINKER_LOG("灭灯!");
    BlinkerMIOT.print();
    wsState = false;
  }
}

//如果小度有对设备进行操作就执行下面
void duerPowerState(const String & state)
{
  BLINKER_LOG("小度语音操作!");
  if (state == BLINKER_CMD_ON) {
    digitalWrite(relay, LOW);
    BlinkerDuerOS.powerState("on");
    BLINKER_LOG("亮灯!");
    BlinkerDuerOS.print();
    wsState = true;
  }
  else if (state == BLINKER_CMD_OFF) {
    digitalWrite(relay, HIGH);
    BlinkerDuerOS.powerState("off");
    BLINKER_LOG("灭灯!");
    BlinkerDuerOS.print();
    wsState = false;
  }
}

//心跳包
void heartbeat()
{
  BLINKER_LOG("状态同步!");
  if (digitalRead(relay) == LOW)
  {
    ButtonOne.print("on");
    ButtonOne.color("#0000FF");              //设置app按键是浅蓝色
    ButtonOne.text("亮");
  }
  else
  {
    ButtonOne.print("off");
    ButtonOne.color("#00FFFF");              //设置app按键是深蓝色
    ButtonOne.text("灭");
  }
}

///如果本地开关有动作执行下面手动模式
void sdms() {
  if (digitalRead(relay) == HIGH && digitalRead(LocalSwitch) == LOW) { //
    Blinker.delay(150);  //延时150ms不能太少标准按键成功率
    if (digitalRead(LocalSwitch) == HIGH) {
      BLINKER_LOG("本地开关动作!");
      BLINKER_LOG("亮灯!");                         //串口打印
      digitalWrite(relay, LOW);
      ButtonOne.color("#0000FF");                  //设置app按键是浅蓝色
      ButtonOne.text("亮");
      ButtonOne.print("on");
      wsState = true;
    }
  }
  if (digitalRead(relay) == LOW && digitalRead(LocalSwitch) == LOW) {
    Blinker.delay(150);
    if (digitalRead(LocalSwitch) == HIGH) {
      BLINKER_LOG("本地开关动作!");
      BLINKER_LOG("灭灯!");                           //串口打印
      digitalWrite(relay, HIGH);
      ButtonOne.color("#00FFFF");                     //设置app按键是深蓝色
      ButtonOne.text("灭");
      ButtonOne.print("off");
      wsState = false;
    }
  }
}

void miotQuery(int32_t queryCode)//小爱设备查询接口
{
  BLINKER_LOG("MIOT Query codes: ", queryCode);
  BLINKER_LOG("小爱查询设备状态");
  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
      BLINKER_LOG("小爱查询设备状态BLINKER_CMD_QUERY_ALL_NUMBER", wsState);
      BlinkerMIOT.powerState(wsState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("小爱查询设备状态BLINKER_CMD_QUERY_POWERSTATE_NUMBER");
      BlinkerMIOT.powerState(wsState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    default :
      BlinkerMIOT.powerState(wsState ? "on" : "off");
      BLINKER_LOG("小爱查询设备状态default", wsState);
      BlinkerMIOT.print();
      break;
  }
}

void setup()
{
  // 初始化串口
  Serial.begin(115200);
  wifi_station_set_hostname("light01");//设置主机名
  BLINKER_DEBUG.stream(Serial);
  BLINKER_DEBUG.debugAll();
  // 初始化有LED的IO
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);              //默认关闭  LOW 时的是接通继电器，HIGH 时的是断开继电器
  pinMode(LocalSwitch, INPUT_PULLUP);        //输入上拉
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  ButtonOne.attach(ButtonOne_callback);              //app上操作必须的注册回调函数关联按键名“ButtonOne”和判断程序“ButtonOne_callback"
  BlinkerMIOT.attachPowerState(miotPowerState);              //小爱语音操作注册函数
  BlinkerMIOT.attachQuery(miotQuery);//小爱查询电源状态注册函数
  BlinkerDuerOS.attachPowerState(duerPowerState);
  //BlinkerDuerOS.attachQuery(duerQuery);
  Blinker.attachHeartbeat(heartbeat);              //app定时向设备发送心跳包, 设备收到心跳包后会返回设备当前状态进行语音操作和app操作同步。
  wsState = false;//默认小爱状态为关
}

void loop() {
  sdms();//本地开关手动模式函数
  //**软重启
  if (Blinker.connected())//Blinker.connected()函数反馈网络连接状态
  {
    con_flag = 0;
  } else
  {
    if (con_flag == 0)
    {
      con_time = millis();    //给断网时间赋初始值
      con_flag = 1;
    } else
    {
      if ((millis() - con_time) >= 90000)    //判断断网时间超90秒后执行重启，这个时间可根据实际需要调整
      {
        resetFunc();//启用这个函数就软重启设备
      }
    }
  }
  Blinker.run();
}
