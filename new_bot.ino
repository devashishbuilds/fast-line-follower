#include <QTRSensors.h>

#define Kp 0.0005
#define Kd 0.092
#define MaxSpeed 230
#define BaseSpeed 210


#define speedturn 180
#define rightMotor1 A3 
#define rightMotor2 A2
#define rightMotorPWM 10
#define leftMotor1 A0
#define leftMotor2 A1
#define leftMotorPWM 11
#define motorPower 8

QTRSensors qtr;

const uint8_t SensorCount = 6;
uint16_t sensorValues[SensorCount];
void setup()
{
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){8, 7, 6, 5, 4, 3}, SensorCount);
  //qtr.setEmitterPin(12);

  Serial.begin(9600);
  pinMode(rightMotor1, OUTPUT);
  pinMode(rightMotor2, OUTPUT);
  pinMode(rightMotorPWM, OUTPUT);
  pinMode(leftMotor1, OUTPUT);
  pinMode(leftMotor2, OUTPUT);
  pinMode(leftMotorPWM, OUTPUT);
  pinMode(motorPower, OUTPUT);
  
  delay(3000);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  int i;
  for (int i = 0; i < 300; i++)
  {
    qtr.calibrate();
    digitalWrite(13, LOW);   
    delay(20); 
  }
  //wait();
  delay(500); 
}  

unsigned int sensors[6];
int position = qtr.readLineBlack(sensors);
int lastError = 0;
void loop()
{  
  position = qtr.readLineBlack(sensors);
  
  if(position>4700){
    
    move(1, speedturn, 1);
    move(0, speedturn, 0);
    return;    
  }
  if(position<300){ 
    move(1, speedturn, 0);
    move(0, speedturn, 1);
    return;
  }
  
  int error = position - 2500;
  int motorSpeed = Kp * error + Kd * (error - lastError);
  lastError = error;

  int rightMotorSpeed = BaseSpeed + motorSpeed;
  int leftMotorSpeed = BaseSpeed - motorSpeed;
  
  if (rightMotorSpeed > MaxSpeed ) rightMotorSpeed = MaxSpeed;
  if (leftMotorSpeed > MaxSpeed ) leftMotorSpeed = MaxSpeed; 
  if (rightMotorSpeed < 0)rightMotorSpeed = 0;    
  if (leftMotorSpeed < 0)leftMotorSpeed = 0;
    
  move(1, rightMotorSpeed, 1);
  move(0, leftMotorSpeed, 1);
}
  
void wait(){
  digitalWrite(motorPower, LOW);
}

void move(int motor, int speed, int direction){
  digitalWrite(motorPower, HIGH);

  boolean inPin1=HIGH;
  boolean inPin2=LOW;
  
  if(direction == 1){
    inPin1 = HIGH;
    inPin2 = LOW;
  }  
  if(direction == 0){
    inPin1 = LOW;
    inPin2 = HIGH;
  }

  if(motor == 0){
    digitalWrite(leftMotor1, inPin1);
    digitalWrite(leftMotor2, inPin2);
    analogWrite(leftMotorPWM, speed);
  }
  if(motor == 1){
    digitalWrite(rightMotor1, inPin1);
    digitalWrite(rightMotor2, inPin2);
    analogWrite(rightMotorPWM, speed);
  }  
}
