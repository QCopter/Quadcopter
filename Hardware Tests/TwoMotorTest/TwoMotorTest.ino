#include <Servo.h>

Servo Motor1;
Servo Motor2;
Servo Motor3;
Servo Motor4;

int x;

void setup()
{
  Serial.begin(9600);
  Motor1.attach(11);
  Motor2.attach(10);
  Motor3.attach(9);
  Motor4.attach(6);
}

void loop()
{
if (x == 0)
{
  for(x = 0; x <= 90  ; x += 1)
  {
    Motor1.write(x);
    Motor2.write(x);
    Motor3.write(x);
    Motor4.write(x);
    delay(100);
  }
}
}
