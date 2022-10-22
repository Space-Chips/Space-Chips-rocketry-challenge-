#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;
int state = 0;
int flight_time = 0;
int min_acceleration = 1000;
int null_acceleration = 0;
int abort_fligth = 0;
int GUIDANCE_No_GO = 0;
int CAPCOM_NO_GO = 0;
int GO = 0;
bool GONOGO = false;

void gyroscopsetup()
{

  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    GUIDANCE_No_GO += 1;
  }
}

void readMPU()
{
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  mpu.getEvent(&a, &g, &temp);
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");
  delay(100);

}
void Acceleration_null()
{
  if (a.acceleration.x * a.acceleration.y * a.acceleration.z)
  {
    null_acceleration = 1;
  }
}

void Go_No_Go_sequence()
{
  Serial.print("Go No GO Sequence, ");
  delay(800);
  Serial.println("Stand by...");
  delay(5000);
  Serial.print("GUIDANCE : ");
  delay(2000);
  if (GUIDANCE_No_GO == 0)
  {
    Serial.println(" GO");
    delay(2000);
    Serial.print("CAPCOM :");
    delay(2000);
    if (CAPCOM_NO_GO == 0)
    {
      Serial.println(" GO");
      delay(2000);
      Serial.println(" We're go for launch");
      delay(1000);
    }
    else
    {
      GO += 1;
      Serial.print(" We are No GO for launch.");
      delay(1500);
    }
  }
  else
  {
    GO += 1;
    Serial.print(" We are No GO for launch.");
    delay(1500);
  }
}

void Min_Acceleration()
{

  if (min_acceleration > a.acceleration.z)
  {
    min_acceleration -= 1;
  }
}

void Abort()
{
  if (a.gyro.pitch >= 90)
  {
    if (a.acceleration.v[0] >= 5.0)
      abort_fligth += 1;
    digitalWrite(2, HIGH);
    Serial.print("Flight Aborted... Stand By");
  }
}
void Timer()
{
  flight_time += 1;
  delay(1000);
}

void IdleState()
{
  if (GONOGO == false)
  {
    Go_No_Go_sequence();
    GONOGO = true;
  }
  
  if (GO == 0)
  {

    if (a.acceleration.z >= 12)
    {
      delay(100);
      if (a.acceleration.z >= 12)
      {
        state += 1;
      }
    }
    else
    {
      readMPU();

    }
  }
}

void Liftof()
{
  Timer();
  readMPU();
  Abort();
  if (a.acceleration.z < 16)
  {
    state += 1;
  }
}

void Burnout()
{
  Timer();
  readMPU();
  if (min_acceleration > a.acceleration.z)
  {
    delay(100);
    if (min_acceleration > a.acceleration.z)
    {
      state += 1;
    }
  }
}

void Apogee()
{
  Timer();
  readMPU();
  delay(1000);
  state += 1;
}

void ballistic_descent()
{
  Timer();
  readMPU();
  delay(2000);
  digitalWrite(2, HIGH);
  delay(1500);
  digitalWrite(2, LOW);
  state += 1;
}

void Slowed_descent()
{
  Timer();
  readMPU();
  if (a.acceleration.z <= 0.5)
  {
    delay(1000);
    if (a.acceleration.z <= 0.5)
    {
      state += 1;
    }
  }
}

void Touch_down()
{
  readMPU();
}
void States()
{
  IdleState();
  Serial.println("IdleState...");
  if (state == 1)
  {
    Liftof();
    Serial.println("Liftof...");
    if (state == 2)
    {
      void Burnout();
      Serial.println("Burnout...");
      Serial.println("Event occured at : T+");
      Serial.print(flight_time);
      if (state == 3)
      {
        Apogee();
        Serial.println("Apogee reached...");
        Serial.println("Event occured at : T+");
        Serial.print(flight_time);
        if (state == 4)
        {
          ballistic_descent();
          Serial.println("ballistic descent...");
          Serial.println("Event occured at : T+");
          Serial.print(flight_time);
          if (state == 5)
          {
            Slowed_descent();
            Serial.println("Slowed descent...");
            Serial.println("Event occured at : T+");
            Serial.print(flight_time);
            if (state == 6)
            {
              Touch_down();
              Serial.println("Touch Down");
              Serial.println("Event occured at : T+");
              Serial.print(flight_time);
              //              Serial.println("Flight Time :");
              //              Serial. print(flight_time);
            }
          }
        }
      }
    }
  }
}

void setup()
{
  gyroscopsetup();
  delay(1000);
  pinMode(2, OUTPUT);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpu.getEvent(&a, &g, &temp);
}

void loop()
{
  States();
  if (abort_fligth == 1)
  {
    Serial.println("Speed :");
    Serial.print(a.acceleration.z);
  }
}
