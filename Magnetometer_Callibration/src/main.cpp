/*************************
  This is an example for the Adafruit SensorLab library
  It will look for a supported magnetometer and output
  PJRC Motion Sensor Calibration Tool-compatible serial data

  PJRC & Adafruit invest time and resources providing this open source code,
  please support PJRC and open-source hardware by purchasing products

#include <Adafruit_SensorLab.h>
  from PJRC!

  Written by PJRC, adapted by Limor Fried for Adafruit Industries.
 *************************/

#include <Adafruit_SensorLab.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>

Adafruit_SensorLab lab;

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
//

//

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

// Adafruit_Sensor *gyro = NULL, *accel = NULL;
// sensors_event_t gyro_event, accel_event;

int loopcount = 0;
//
 sensors_event_t mag_event;
void setup(void) {
  Serial.begin(9600);
  while (!Serial) delay(500);     // will pause Zero, Leonardo, etc until serial console opens
  
  Serial.println(F("Sensor Lab - IMU Calibration!"));
  lab.begin();

  Serial.println("Calibration filesys test");
  if (!cal.begin()) {
    Serial.println("Failed to initialize calibration helper");
    while (1) yield();
  }
  if (! cal.loadCalibration()) {
    Serial.println("No calibration loaded/found... will start with defaults");
  } else {
    Serial.println("Loaded existing calibration");
  }

  // Serial.println("Looking for a magnetometer");
  // mag = lab.getMagnetometer();
  // if (! mag) {
  //   Serial.println(F("Could not find a magnetometer, skipping!"));
  // } else {
  //   mag->printSensorDetails();
  // }
  mag.begin();
 // Serial.println("Looking for a gyroscope");
  // gyro = lab.getGyroscope();
  // if (! gyro) {
  //   Serial.println(F("Could not find a gyroscope, skipping!"));
  // } else {
  //   gyro->printSensorDetails();
  // }
  
//   Serial.println("Looking for a accelerometer");
//   accel = lab.getAccelerometer();
//   if (! accel) {
//     Serial.println(F("Could not find a accelerometer, skipping!"));
//   } else {
//     accel->printSensorDetails();
//   }
// }
}
void loop() {
  // if (mag && ! mag->getEvent(&mag_event)) {
  //   return;
  // }
 
mag.getEvent(&mag_event);

  // if (gyro && ! gyro->getEvent(&gyro_event)) {
  //   return;
  // }
  // if (accel && ! accel->getEvent(&accel_event)) {
  //   return;
  // }
  // 'Raw' values to match expectation of MOtionCal
  Serial.print("Raw:");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(int(mag_event.magnetic.x*10)); Serial.print(",");
  Serial.print(int(mag_event.magnetic.y*10)); Serial.print(",");
  Serial.print(int(mag_event.magnetic.z*10)); Serial.println("");

  // unified data
  Serial.print("Uni:");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(0); Serial.print(",");
  Serial.print(mag_event.magnetic.x); Serial.print(",");
  Serial.print(mag_event.magnetic.y); Serial.print(",");
  Serial.print(mag_event.magnetic.z); Serial.println("");
  loopcount++;
  receiveCalibration();

  // occasionally print calibration
  if (loopcount == 50 || loopcount > 100) {
    Serial.print("Cal1:");
    for (int i=0; i<3; i++) {
      Serial.print(cal.accel_zerog[i], 3); 
      Serial.print(",");
    }
    for (int i=0; i<3; i++) {
      Serial.print(cal.gyro_zerorate[i], 3);
      Serial.print(",");
    }  
    for (int i=0; i<3; i++) {
      Serial.print(cal.mag_hardiron[i], 3); 
      Serial.print(",");
    }  
    Serial.println(cal.mag_field, 3);
    loopcount++;
  }
  if (loopcount >= 100) {
    Serial.print("Cal2:");
    for (int i=0; i<9; i++) {
      Serial.print(cal.mag_softiron[i], 4); 
      if (i < 8) Serial.print(',');
    }
    Serial.println();
    loopcount = 0;
  }
 
  delay(10); 
}

/********************/

byte caldata[68]; // buffer to receive magnetic calibration data
byte calcount=0;

void receiveCalibration() {
  uint16_t crc;
  byte b, i;

  while (Serial.available()) {
    b = Serial.read();
    if (calcount == 0 && b != 117) {
      // first byte must be 117
      return;
    }
    if (calcount == 1 && b != 84) {
      // second byte must be 84
      calcount = 0;
      return;
    }
    // store this byte
    caldata[calcount++] = b;
    if (calcount < 68) {
      // full calibration message is 68 bytes
      return;
    }
    // verify the crc16 check
    crc = 0xFFFF;
    for (i=0; i < 68; i++) {
      crc = crc16_update(crc, caldata[i]);
    }
    if (crc == 0) {
      // data looks good, use it
      float offsets[16];
      memcpy(offsets, caldata+2, 16*4);
      cal.accel_zerog[0] = offsets[0];
      cal.accel_zerog[1] = offsets[1];
      cal.accel_zerog[2] = offsets[2];
      
      cal.gyro_zerorate[0] = offsets[3];
      cal.gyro_zerorate[1] = offsets[4];
      cal.gyro_zerorate[2] = offsets[5];
      
      cal.mag_hardiron[0] = offsets[6];
      cal.mag_hardiron[1] = offsets[7];
      cal.mag_hardiron[2] = offsets[8];

      cal.mag_field = offsets[9];
      
      cal.mag_softiron[0] = offsets[10];
      cal.mag_softiron[1] = offsets[13];
      cal.mag_softiron[2] = offsets[14];
      cal.mag_softiron[3] = offsets[13];
      cal.mag_softiron[4] = offsets[11];
      cal.mag_softiron[5] = offsets[15];
      cal.mag_softiron[6] = offsets[14];
      cal.mag_softiron[7] = offsets[15];
      cal.mag_softiron[8] = offsets[12];

      if (! cal.saveCalibration()) {
        Serial.println("*WARNING* Couldn't save calibration");
      } else {
        Serial.println("Wrote calibration");    
      }
      cal.printSavedCalibration();
      calcount = 0;
      return;
    }
    // look for the 117,84 in the data, before discarding
    for (i=2; i < 67; i++) {
      if (caldata[i] == 117 && caldata[i+1] == 84) {
        // found possible start within data
        calcount = 68 - i;
        memmove(caldata, caldata + i, calcount);
        return;
      }
    }
    // look for 117 in last byte
    if (caldata[67] == 117) {
      caldata[0] = 117;
      calcount = 1;
    } else {
      calcount = 0;
    }
  }
}


uint16_t crc16_update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; i++) {
    if (crc & 1) {
      crc = (crc >> 1) ^ 0xA001;
    } else {
      crc = (crc >> 1);
    }
  }
  return crc;
}