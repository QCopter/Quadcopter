/******************************************************
Library designed to manage, update, and control the
state of quadcopter

Author  : Brandon Riches
Date    : May 2013


    *Intructions to use*
    1) Call initSensor() in setup().
    2) Call get_Initial_Offsets in setup().
    3) At the very least, call updateData() in loop().


******************************************************/


#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Control.h>
#include <MMA8453_n0m1.h>
#include <OseppGyro.h>
#include <I2C.h>
#include <math.h>
#include <Servo.h>
#include <PID_v1.h>


/***************************************************************************
 CONTROL
 ***************************************************************************/
/***************************************************************************
 *! @PRIVATE FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief Reads the settings into the structure. Private!
*/
/**************************************************************************/
void Control::getSettings()
{
    Settings.d_ScaleRange = d_ScaleRange;          // This function is just for
    Settings.g_ScaleRange = g_ScaleRange;          // generality. Allows user
    Settings.DLPF = DLPF;                         					// changes of settings.
    Settings.HighDef = HighDef;
    Settings.g_threshold = g_threshold;
    Settings.d_threshold = d_threshold;
};

/**************************************************************************/
/*!
    @brief Gets the initial offsets in both sensors to accomodate starting
*/
/**************************************************************************/
void Control::get_Initial_Offsets()
{
    int offset_counter = 10;                       // # of data sets to consider when finding offsets
    int counter = 1;
    double acceldata[3];
    double gyrodata[3];

    while(counter <= offset_counter)
    {
        accel.update();                            // Updates the accelerometer registers
        acceldata[0] = accel.x();
        acceldata[1] = accel.y();
        acceldata[2] = accel.z();
        gyro.update();                             // Updates the gyro output registers
        gyrodata[0] = gyro.x();
        gyrodata[1] = gyro.y();
        gyrodata[2] = gyro.z();
        Offsets.ax = (Offsets.ax + acceldata[0] ); // Sum
        Offsets.ay = (Offsets.ay + acceldata[1] );
        Offsets.az = (Offsets.az + acceldata[2] );
        Offsets.wx = (Offsets.wx + gyrodata[0] );
        Offsets.wy = (Offsets.wy + gyrodata[1] );
        Offsets.wz = (Offsets.wz + gyrodata[2] );
        counter = counter + 1 ;

        delayMicroseconds(10);                     // Delay to ensure accel/gyro can physically keep up
    }

    Serial.println(" ");
    Offsets.ax = (Offsets.ax)/offset_counter;      // Store the average of the readings into the Offsets struct
    Serial.print("accelerometer x-offset: ");
    Serial.println(Offsets.ax);
    Offsets.ay = (Offsets.ay)/offset_counter;
    Serial.print("accelerometer y-offset: ");
    Serial.println(Offsets.ay);
    Offsets.az = ((Offsets.az)/offset_counter) - 256;
    Serial.print("accelerometer z-offset: ");
    Serial.println(Offsets.az);
    Offsets.wx = (Offsets.wx)/offset_counter;
    Serial.print("gyro x-offset: ");
    Serial.println(Offsets.wx);
    Offsets.wy = (Offsets.wy)/offset_counter;
    Serial.print("gyro y-offset: ");
    Serial.println(Offsets.wy);
    Offsets.wz = (Offsets.wz)/offset_counter;
    Serial.print("gyro z-offset: ");
    Serial.println(Offsets.wz);
};

/**************************************************************************/
/*!
    @brief Converts the raw data from sensors to SI units
*/
/**************************************************************************/
void Control::SI_convert()
{
  //Convert accelerometer readouts to m/s^2
    switch(g_ScaleRange) {
        case FULL_SCALE_RANGE_2g:
            Data.ax = Data.ax * SI_CONVERT_2g;
            Data.ay = Data.ay * SI_CONVERT_2g;
            Data.az = Data.az * SI_CONVERT_2g;
            break;

        case FULL_SCALE_RANGE_4g:
            Data.ax = Data.ax * SI_CONVERT_4g;
            Data.ay = Data.ay * SI_CONVERT_4g;
            Data.az = Data.az * SI_CONVERT_4g;
            break;

        case FULL_SCALE_RANGE_8g:
            Data.ax = Data.ax * SI_CONVERT_8g;
            Data.ay = Data.ay * SI_CONVERT_8g;
            Data.az = Data.az * SI_CONVERT_8g;
            break;
    }
    // Convert gyro readouts to degrees/s
    switch(d_ScaleRange) {

        case FULL_SCALE_RANGE_250:
            Data.wx = Data.wx * SI_CONVERT_250;
            Data.wy = Data.wy * SI_CONVERT_250;
            Data.wz = Data.wz * SI_CONVERT_250;
            break;

        case FULL_SCALE_RANGE_500:
            Data.wx = Data.wx * SI_CONVERT_500;
            Data.wy = Data.wy * SI_CONVERT_500;
            Data.wz = Data.wz * SI_CONVERT_500;
            break;

        case FULL_SCALE_RANGE_1000:
            Data.wx = Data.wx * SI_CONVERT_1000;
            Data.wy = Data.wy * SI_CONVERT_1000;
            Data.wz = Data.wz * SI_CONVERT_1000;
            break;

        case FULL_SCALE_RANGE_2000:
            Data.wx = Data.wx * SI_CONVERT_2000;
            Data.wy = Data.wy * SI_CONVERT_2000;
            Data.wz = Data.wz * SI_CONVERT_2000;
            break;
    }
};


/***************************************************************************
 CONTROL
 ***************************************************************************/
/***************************************************************************
 *! @PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief Initializes the sensors, much like in the original setup
*/
/**************************************************************************/
bool Control::initSensor()
{
    getSettings();                                             // Settings struct
	byte x=0x0;
	Serial.print("Intializing gyro ....  ");                      // See OseppGyro.h
    gyro.setI2CAddr(Gyro_Address);                             // Set the I2C address in OseppGyro class
    gyro.dataMode(Settings.d_ScaleRange, Settings.DLPF);       // Set the dataMode in the OseppGyro Class

    while(x != B100000)
    {
        gyro.regRead(USER_CTRL,&x);                            // See the data sheet for the MPU3050 gyro
        gyro.regWrite(USER_CTRL,B00100000);                    // http://invensense.com/mems/gyro/documents/RM-MPU-3000A.pdf
    }
    Serial.println("Done!");
    delayMicroseconds(10);

    Serial.print("Initializing accel ....  ");                    // Same as the gyro initialization, but the accel isnt an ass
    accel.setI2CAddr(Accel_Address);                           // See the data sheet for the MMA8452Q Accelerometer registers
    accel.dataMode(Settings.HighDef, Settings.g_ScaleRange);   // http://cache.freescale.com/files/sensors/doc/data_sheet/MMA8452Q.pdf?fpsp=1
	Serial.println("Done!");
    delayMicroseconds(10);

	Serial.print("Getting initial offset vals ....  ");
    get_Initial_Offsets();                                     // Initial offsets private function in Control class
    Serial.println("Done!");

    return true;
};

/**************************************************************************/
/*!
    @brief Initializes the motors, code from Andrew's script
*/
/**************************************************************************/
bool Control::initMotors()
{
    MotorSpeeds.motor1s = 0;                         // Initialize all motor speeds to 0
    MotorSpeeds.motor2s = 0;                         // This might be useful, later
    MotorSpeeds.motor3s = 0;
    MotorSpeeds.motor4s = 0;
    motor1.attach(11);                      // Attach motor 1 to D11
    motor2.attach(10);                      // Attach motor 2 to D10
    motor3.attach(9);                       // Attach motor 3 to D9
    motor4.attach(6);                       // Attach motor 4 to D8

    // initializes motor1
    for(MotorSpeeds.motor1s = 0; MotorSpeeds.motor1s < 50; MotorSpeeds.motor1s += 1)
    {
      motor1.write(MotorSpeeds.motor1s);
      motor2.write(MotorSpeeds.motor1s);
      motor3.write(MotorSpeeds.motor1s);
      motor4.write(MotorSpeeds.motor1s);
      MotorSpeeds.motor2s = MotorSpeeds.motor1s;
      MotorSpeeds.motor3s = MotorSpeeds.motor1s;
      MotorSpeeds.motor4s = MotorSpeeds.motor1s;
      delay(50);
    }

    return true;
}


/**************************************************************************/
/*!
    @brief Reads data from sensors
*/
/**************************************************************************/
void Control::update()
{
    accel.update();                                                 // Update the registers storing data INSIDE the sensors
    gyro.update();

    Data.ax = accel.x() - Offsets.ax;                         // Store Raw values from the sensor registers
    Data.ay = accel.y() - Offsets.ay;                         // and removes the initial offsets
    Data.az = accel.z() - Offsets.az;
    Data.wx = gyro.y() - Offsets.wy;
    Data.wy = gyro.x() - Offsets.wx;
    Data.wz = gyro.z() - Offsets.wz;

    SI_convert();                                           // Convert to SI units from raw data type

    if(fabs(Data.ax) < Settings.g_threshold) {Data.ax = 0;}     // Check if the data is less than the threshold
    if(fabs(Data.ay) < Settings.g_threshold) {Data.ay = 0;}
    if(fabs(Data.az) < Settings.g_threshold) {Data.az = 0;}
    if(fabs(Data.wx) < Settings.d_threshold) {Data.wx = 0;}
    if(fabs(Data.wy) < Settings.d_threshold) {Data.wy = 0;}
    if(fabs(Data.wz) < Settings.d_threshold) {Data.wz = 0;}

	/*  Updates the prev_data by bumping up each data set */
    for (int i = 30; i <= 35; i ++)
    {
    	Data.prev_data[i+6] = Data.prev_data[i];
    }
	for (int i = 24; i <=29; i++)
	{
		Data.prev_data[i+6] = Data.prev_data[i];
	}
	for (int i = 18; i <=23; i++)
	{
		Data.prev_data[i+6] = Data.prev_data[i];
	}
	for (int i = 12; i <=17; i++)
	{
		Data.prev_data[i+6] = Data.prev_data[i];
	}
	for (int i = 6; i <=11; i++)
	{
		Data.prev_data[i+6] = Data.prev_data[i];
	}
	for (int i = 0; i <=5; i++)
	{
		Data.prev_data[i+6] = Data.prev_data[i];
	}
	Data.prev_data[0] = Data.ax;
	Data.prev_data[1] = Data.ay;
	Data.prev_data[2] = Data.az;
	Data.prev_data[3] = Data.wx;
	Data.prev_data[4] = Data.wy;
	Data.prev_data[5] = Data.wz;					// Buffer is updated, now a moving average can be calculated
																			// If the buffer size has to be increased then we can do that.

	Data.ax = 0; Data.ay = 0; Data.az = 0; Data.wx = 0; Data.wy = 0; Data.wz = 0; // ready for average calculation

	for (int i = 0; i <=36; i += 6)
	{
		Data.ax = Data.ax + Data.prev_data[i];
	}
	for (int i = 1; i <=37 ; i += 6)
	{
		Data.ay = Data.ay + Data.prev_data[i];
	}
	for (int i = 2; i <=38 ; i += 6)
	{
		Data.az = Data.az + Data.prev_data[i];
	}
	for (int i = 3; i <=39 ; i += 6)
	{
		Data.wx = Data.wx + Data.prev_data[i];
	}
	for (int i = 4; i <=40 ; i += 6)
	{
		Data.wy = Data.wy + Data.prev_data[i];
	}
	for (int i = 5; i <=41 ; i += 6)
	{
		Data.wz = Data.wz + Data.prev_data[i];
	}

	Data.ax /= 7;
	Data.ay /= 7;
	Data.az /= 7;
	Data.wx /= 7;
	Data.wy /= 7;
	Data.wz /= 7;



    Data.t_previous = Data.t_current;                   // Update the timestamps
    Data.t_current = micros();
    double time = (Data.t_current - Data.t_previous) / (1000000); // Converts time from microseconds to seconds
    Data.freq = 1/time;

    heading = heading + Data.wz * (time);// Integrates wz to find the angular displacement
    // Accelerometer code to find angles
    alpha = atan2(Data.ax, Data.az) *180/Pi ; // Arctan of the two values returns the angle,
    beta = atan2(Data.ay, Data.az) *180/Pi;   // in rads, and convert to degrees

};

/**************************************************************************/
/*!
    @brief Sets the motors to a new speed
*/
/**************************************************************************/

void Control::setMotorSpeed(int motor, int speed)
{
	int m1d;		// Directions that motor speed needs to increment
	int m2d;		// +1 for increase, -1 for decrease
	int m3d;
	int m4d;

	switch (motor)
	{
		/* MOTOR 1*/
		case 1:

			if (speed !=  MotorSpeeds.motor1s)										// Change required, not the same
			{
				if (speed > MotorSpeeds.motor1s)										// Need to go up
				{
					for (int x = MotorSpeeds.motor1s; x <= speed; x += 1)	// Ease into the new speed
					{
						motor1.write(x);
						MotorSpeeds.motor1s = x;
						delay(25);
					}
				}
				else																					// Need to decrease speed
				{
					for (int x = MotorSpeeds.motor1s; x >= speed; x -= 1)	// Ease into the new speed
					{
						motor1.write(x);
						MotorSpeeds.motor1s = x;
						delay(25);
					}
				}
			}
			break;

			/* MOTOR 2 */
			case 2:

			if (speed !=  MotorSpeeds.motor2s)										// Change required, not the same
			{
				if (speed > MotorSpeeds.motor2s)										// Need to go up
				{
					for (int x = MotorSpeeds.motor2s; x <= speed; x += 1)	// Ease into the new speed
					{
						motor2.write(x);
						MotorSpeeds.motor2s = x;
						delay(25);
					}
				}
				else																					// Need to decrease speed
				{
					for (int x = MotorSpeeds.motor2s; x >= speed; x -= 1)	// Ease into the new speed
					{
						motor2.write(x);
						MotorSpeeds.motor2s = x;
						delay(25);
					}
				}
			}
			break;

			/* MOTOR 3 */
			case 3:

			if (speed !=  MotorSpeeds.motor3s)										// Change required, not the same
			{
				if (speed > MotorSpeeds.motor3s)										// Need to go up
				{
					for (int x = MotorSpeeds.motor3s; x <= speed; x += 1)	// Ease into the new speed
					{
						motor3.write(x);
						MotorSpeeds.motor3s = x;
						delay(25);
					}
				}
				else																					// Need to decrease speed
				{
					for (int x = MotorSpeeds.motor3s; x >= speed; x -= 1)	// Ease into the new speed
					{
						motor3.write(x);
						MotorSpeeds.motor3s = x;
						delay(25);
					}
				}
			}
			break;

			/* MOTOR 4 */
			case 4:

			if (speed !=  MotorSpeeds.motor4s)										// Change required, not the same
			{
				if (speed > MotorSpeeds.motor4s)										// Need to go up
				{
					for (int x = MotorSpeeds.motor4s; x <= speed; x += 1)	// Ease into the new speed
					{
						motor4.write(x);
						MotorSpeeds.motor4s = x;
						delay(25);
					}
				}
				else																					// Need to decrease speed
				{
					for (int x = MotorSpeeds.motor4s; x >= speed; x -= 1)	// Ease into the new speed
					{
						motor4.write(x);
						MotorSpeeds.motor4s = x;
						delay(25);
					}
				}
			}
			break;

			/* ALL MOTORS REQUIRE CHANGE */
			case 5:

			if  ((speed > MotorSpeeds.motor1s )&& (speed != MotorSpeeds.motor1s)) {m1d = 1;}
			else {m1d = -1;}

			if  ((speed > MotorSpeeds.motor2s )&& (speed != MotorSpeeds.motor2s)) {m2d = 1;}
			else {m2d = -1;}

			if  ((speed > MotorSpeeds.motor3s )&& (speed != MotorSpeeds.motor3s)) {m3d = 1;}
			else {m3d= -1;}

			if  ((speed > MotorSpeeds.motor4s )&& (speed != MotorSpeeds.motor4s)) {m4d = 1;}
			else {m4d= -1;}

			while ( (MotorSpeeds.motor1s != speed)
				  ||  (MotorSpeeds.motor2s != speed)
				  ||  (MotorSpeeds.motor3s != speed)
				  ||  (MotorSpeeds.motor4s != speed) )
				  {
						if (MotorSpeeds.motor1s != speed )
						{
							MotorSpeeds.motor1s += m1d;
							motor1.write(MotorSpeeds.motor1s);
						}
						if (MotorSpeeds.motor2s != speed )
						{
							MotorSpeeds.motor2s += m2d;
							motor2.write(MotorSpeeds.motor2s);
						}
						if (MotorSpeeds.motor3s != speed )
						{
							MotorSpeeds.motor3s += m3d;
							motor3.write(MotorSpeeds.motor3s);
						}
						if (MotorSpeeds.motor4s != speed )
						{
							MotorSpeeds.motor4s += m4d;
							motor4.write(MotorSpeeds.motor4s);
						}

						delay(25);
				  }


			break;
	}
}
