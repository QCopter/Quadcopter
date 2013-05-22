 /*=========================================================================
Name: QCopterMain.ino
Authors: Brandon Riches, Patrick Fairbanks, Andrew Coulthard
Date: May 2013
    -----------------------------------------------------------------------*/

#include <MMA8453_n0m1.h>
#include <OseppGyro.h>
#include <I2C.h>
#include <math.h>

unsigned long initial_time = 0;
unsigned long final_time = 0;
#define pi = 3.14159F;

 /*=========================================================================
    Class instances
    -----------------------------------------------------------------------*/
MMA8453_n0m1 accel;
OseppGyro gyro;


 /*=========================================================================
    State variables
    -----------------------------------------------------------------------*/
double heading = 0;
double altitude = 0;


 /*=========================================================================
    Device settings (Options for sensors)
    -----------------------------------------------------------------------*/
const int d_ScaleRange = FULL_SCALE_RANGE_250; // x250,x500,x1000,x2000
const int g_ScaleRange = FULL_SCALE_RANGE_2g; // x2g,x4g,x8g
const int DLPF = 0; // 0,1,2,3,4,5,6,7 // See data sheet
const bool HighDef = true;
const int g_threshold = 10; //Upper threshold for set zero from accel data 
const int d_threshold = 10;


 /*=========================================================================
    Data arrays
    -----------------------------------------------------------------------*/
double init_offset[6] = {0,0,0,0,0,0};
double acceldata[3] = {0,0,0};
double gyrodata[3] = {0,0,0};
double maincount = 0;



 /*=========================================================================
    Function declarations
    -----------------------------------------------------------------------*/
void MainTask(); // Operates the basic 
void get_baseline(); // Gets the baseline offsets for the two sensors
void update_sensors(); // Updates the accel/gyro data arrays
void SI_convert(); // Converts into workable SI units


 /*=========================================================================
    Main Initialization
    -----------------------------------------------------------------------*/
void setup()
{
  //Start the serial port for output
  Serial.begin(9600);
  
  /************************ Gyro initialization ************************/
  Serial.println(" ");
  
  Serial.println("Initializing gyro:  "); Serial.println(" ");
  
  Serial.print("  - Setting gyro I2C addr... ");
  gyro.setI2CAddr(0x69);
  Serial.println("   Gyro I2C addr set!"); Serial.println(" ");
  
  Serial.print("  - Setting gyro data mode... ");
  gyro.dataMode(d_ScaleRange, DLPF);
  Serial.println("   Gyro data mode set!"); Serial.println(" ");

  Serial.print("  - Setting gyro 3rd party accel enable... ");  
  byte x = 0x1;
  while(x != B100000) {
    gyro.regRead(0x3D,&x);
    gyro.regWrite(0x3D,B00100000); // Sets USER_CTRL to 00100000 to allow 3rd party accel on I2C
  }
  Serial.println("   Gryo 3rd party accel enabled!"); Serial.println(" ");
  
  Serial.println("Gyro initialization complete!"); Serial.println(" ");
  delay(10);
  
  /******************** End Gyro initialization ************************/
  
  /*********************** Accel initialization ************************/

  Serial.println("Initializing accel:  "); Serial.println(" ");
  Serial.print("  - Setting accel I2C addr... ");  
  accel.setI2CAddr(0x1D); // I2C address of the acceleromter
  Serial.println("   Accel I2C addr set!"); Serial.println(" ");
  Serial.print("  - Setting accel data mode... ");
  accel.dataMode(HighDef,g_ScaleRange); //
  Serial.println("   Accel data mode set!"); Serial.println(" ");
  Serial.println("Accel initialization complete!"); Serial.println(" ");
  delay(10);
  /******************* End Accel initialization ************************/
  
  /*=========================================================================
      Housekeeping business
      -----------------------------------------------------------------------*/
  get_baseline(); // Sets the offset registers in both sensors to accomodate the starting point
  // MAKE SURE IT STARTS LEVEL =O
  Serial.println(" Did this! ");

}


void loop() 
{
  update_sensors();
  SI_convert();
  
  altitude = analogRead(0); // 
  
  
  //Fix axis orientations
  acceldata[2] = -acceldata[2]; // z' = -z
  double wx = gyrodata[0];
  double wy = gyrodata[1];
  double wz = gyrodata[2];
  
  gyrodata[0] = wy;
  gyrodata[1] = -wx;
  gyrodata[2] = -wz;
  MainTask();
}

 /*=========================================================================
    void MainTask();
    -----------------------------------------------------------------------*/

void MainTask() 
{
  double uc = pow(10,6);
  
  double ax = acceldata[0];
  double ay = acceldata[1];
  double az = acceldata[2];
  
  double wx = gyrodata[0];
  double wy = gyrodata[1];
  double wz = gyrodata[2];
  
  
  double alpha = atan2(ax,az)*(180/ pi);
  double beta = atan2(ay,az)*(180/pi);

  
  final_time = micros();
  double time = ((double)final_time-(double)initial_time)/uc;
  // Integration
  heading = heading + wz*time;
  
  
  initial_time = micros();
  
  Serial.print(" a: "); Serial.print(alpha);
  Serial.print(" b: "); Serial.print(beta);
  Serial.print(" hdg: "); Serial.println(heading);
}

 /*=========================================================================
    void SI_convert() Converts the data in the data 
    -----------------------------------------------------------------------*/
void SI_convert() 
{
  //Convert accelerometer readouts to m/s^2
    switch(g_ScaleRange) {
        case FULL_SCALE_RANGE_2g:
            for (int i = 0; i <= 2; i++) {
                acceldata[i] = acceldata[i] *SI_CONVERT_2g; // readouts in m/s^2
            }
            break;
        case FULL_SCALE_RANGE_4g:
            for (int i = 0; i <= 2; i++){
                acceldata[i] = acceldata[i]*SI_CONVERT_4g; // readouts in m/s^2
            }
            break;
        case FULL_SCALE_RANGE_8g:
            for (int i = 0; i <= 2; i++){
                acceldata[i] = acceldata[i]*SI_CONVERT_4g; // readouts in m/s^2
            }
            break;
    }
    // Convert gyro readouts to degrees/s
    switch(d_ScaleRange) {

        case FULL_SCALE_RANGE_250:
            for (int i = 0; i <= 2; i++){
                gyrodata[i] = gyrodata[i]*SI_CONVERT_250; // readouts in deg/s
            }
            break;
        case FULL_SCALE_RANGE_500:
            for (int i = 0; i <= 2; i++){
                gyrodata[i] = gyrodata[i]*SI_CONVERT_500; // readouts in deg/s
            }
            break;
        case FULL_SCALE_RANGE_1000:
            for (int i = 0; i <= 2; i++){
                gyrodata[i] = gyrodata[i]*SI_CONVERT_1000; // readouts in deg/s
            }
            break;
        case FULL_SCALE_RANGE_2000:
            for (int i = 0; i <= 2; i++){
                gyrodata[i] = gyrodata[i]*SI_CONVERT_2000; // readouts in deg/s
            }
            break;
    }
}
        
 /*=========================================================================
    void update_sensors() Updates the sensor data arrays.
    -----------------------------------------------------------------------*/
    
void update_sensors() 
{
  accel.update();
  acceldata[0] = accel.x() - init_offset[0];
  acceldata[1] = accel.y() - init_offset[1];
  acceldata[2] = accel.z() - init_offset[2];
  
  gyro.update();
  gyrodata[0] = gyro.x() - init_offset[3];
  gyrodata[1] = gyro.y() - init_offset[4];
  gyrodata[2] = gyro.z() - init_offset[5];
  
  if (fabs(acceldata[0]) <= g_threshold) {acceldata[0] = 0;}
  if (fabs(acceldata[1]) <= g_threshold) {acceldata[1] = 0;}
  if (fabs(acceldata[2]) <= g_threshold) {acceldata[2] = 0;}
  
  if ( fabs(gyrodata[0]) <= d_threshold ) {gyrodata[0] = 0;}
  if ( fabs(gyrodata[1]) <= d_threshold ) {gyrodata[1] = 0;}
  if ( fabs(gyrodata[2]) <= d_threshold ) {gyrodata[2] = 0;}
}


 /*=========================================================================
    void get_baseline()
    Finds the baseline offsets for all sensors
    -----------------------------------------------------------------------*/
void get_baseline() 
{
  int offset_counter = 10;
  int counter = 1;

  Serial.println("Getting baseline offsets...");

  while(counter <= offset_counter)
  {
    accel.update();  // Updates the accelerometer registers
    acceldata[0] = accel.x();
    acceldata[1] = accel.y();
    acceldata[2] = accel.z();
    gyro.update();   // Updates the gyro output registers
    gyrodata[0] = gyro.x();
    gyrodata[1] = gyro.y();
    gyrodata[2] = gyro.z();
    init_offset[0] = (init_offset[0] + acceldata[0] ); // Sum
    init_offset[1] = (init_offset[1] + acceldata[1] );
    init_offset[2] = (init_offset[2] + acceldata[2] );
    init_offset[3] = (init_offset[3] + gyrodata[0] );
    init_offset[4] = (init_offset[4] + gyrodata[1] );
    init_offset[5] = (init_offset[5] + gyrodata[2] );
    counter = counter + 1 ;
    
    
    delayMicroseconds(10);
  }

  Serial.println(" ");
  init_offset[0] = (init_offset[0])/offset_counter;
  Serial.print("accelerometer x-offset: ");
  Serial.println(init_offset[0]);
  init_offset[1] = (init_offset[1])/offset_counter;
  Serial.print("accelerometer y-offset: ");
  Serial.println(init_offset[1]);
  init_offset[2] = ((init_offset[2])/offset_counter) + 256;
  Serial.print("accelerometer z-offset: ");
  Serial.println(init_offset[2]);
  init_offset[3] = (init_offset[3])/offset_counter;
  Serial.print("gyro x-offset: ");
  Serial.println(init_offset[3]);
  init_offset[4] = (init_offset[4])/offset_counter;
  Serial.print("gyro y-offset: ");
  Serial.println(init_offset[4]);
  init_offset[5] = (init_offset[5])/offset_counter;
  Serial.print("gyro z-offset: ");
  Serial.println(init_offset[5]);
  Serial.println(" ");
}
