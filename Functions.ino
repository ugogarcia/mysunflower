long labs(long num)
{
  return (num>=0)?num:num*-1; 
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code moves the stepper motors
void moveToPosition(long altsteps, long azsteps)
{
 /* Serial.print("Movimiento AZ: ");
  Serial.print(azsteps);  
  Serial.print(". Movimiento AL: ");
  Serial.println(altsteps);
*/
  moveMotorWithAccel(azsteps, azimuthStepPin, azimuthDirPin, azSpeed, azAccel);
  moveMotorWithAccel(altsteps, altitudeStepPin, altitudeDirPin, altSpeed, altAccel); 
}  


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code moves the stepper motors with acceleration
void moveMotorWithAccel(long numOfSteps,int stepPin, int DirPin, float maxspeed,float Accel)
{
  long pasos=0;
  //Serial.print("1. Steps: ");
  //Serial.println(numOfSteps);
  if ((long)numOfSteps>=0) {digitalWrite(DirPin, HIGH);} else {digitalWrite(DirPin, LOW);}  
  
  float minMotorDelay  = (1000000/((maxspeed*steps)/60))/2;
  float halfWay = abs(long(numOfSteps/2));
  float maxMotorDelay=((1000000/((maxspeed*steps)/60))*100)/2;
  float motorDelay=maxMotorDelay;  

 // Serial.print("Mw voy a mover: ");
 // Serial.println(numOfSteps);
  
  float motorDelayAdjust=((maxMotorDelay-minMotorDelay)*(maxMotorDelay-minMotorDelay))/(Accel*1000000);               
  float numberOfStepsToReachMinFromMax=long((maxMotorDelay-minMotorDelay)/motorDelayAdjust);    
  if(-0.00001<Accel&&Accel<0.00001){motorDelay=minMotorDelay;}
     
  if (labs(numOfSteps)>numberOfStepsToReachMinFromMax*2)
  {//This code runs if the accleration slope "flat lines"
//    Serial.println("ENTRO POR A");
    for (long doSteps=1; doSteps <= labs(numOfSteps); doSteps++)
    {
      pasos++;
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, HIGH);
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, LOW);
      if ((motorDelay>minMotorDelay)&&(doSteps<halfWay)) {motorDelay=motorDelay-motorDelayAdjust;}//Accelerates the motor
      if ((motorDelay<minMotorDelay)) {motorDelay=minMotorDelay;}//Holds the motor at its max speed
      if ((doSteps>(labs(numOfSteps)-numberOfStepsToReachMinFromMax))) {motorDelay=motorDelay+motorDelayAdjust;}//Deccelerates the motor after it gets close to the end of its move
    }
  }
  else
  {//This code runs if the acceleration slope is an upside down V.
  //  Serial.println("ENTRO POR B");
    for (long doSteps=1; doSteps <= labs(numOfSteps); doSteps++)
    {
      pasos++;
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, HIGH);     
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, LOW);
      if ((motorDelay>minMotorDelay)&&(doSteps<halfWay)) {motorDelay=motorDelay-motorDelayAdjust;}//Accelerates the motor
      if ((motorDelay<minMotorDelay)) {motorDelay=minMotorDelay;}//Holds the motor at its max speed
      if (doSteps>halfWay) {motorDelay=motorDelay+motorDelayAdjust;}//Deccelerates the motor after it gets close to the end of its move
      if (motorDelay>maxMotorDelay) {motorDelay=maxMotorDelay;}   
    }
  } 
  
  
  
  Serial.print("Me he movido de pasos: ");
  Serial.println(pasos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code is used to delay between stepper motor steps
void delayInMicroseconds(long delayInMicrosec)
{
  long t1,t2;
  t1=micros(); 
  t2=micros();
  while ((t1+delayInMicrosec)>t2)
  {
    t2=micros();
    if (t2<t1) {t2=t1+delayInMicrosec+1;}//Check if micros() rolled over
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code resets the machine on the limit switches
void findLimits(int altOrAz, int motorDirection, float limitAngle)
{
  if (altOrAz==1) 
    searchForLimit(limitAngle, altitudeDirPin, altitudeStepPin, altResetSpeed, altLimitPin, altitudeMax, motorDirection );

  if (altOrAz==2)
    searchForLimit(limitAngle, azimuthDirPin, azimuthStepPin, azResetSpeed, azLimitPin, azimuthMax, motorDirection );
}

void searchForLimit(float limitAngle, int DirPin, int stepPin, int ResetSpeed, int LimitPin, long maxResetSteps, int motorDirection)
{
  long x;
  int whichDir;
  if (motorDirection!=0)
  {//A motorDirection of 0 will skip the reset  
    if (abs(limitAngle) != limitAngle) {whichDir = 1;} else {whichDir = -1;}
  //Serial.println("OK2");
    if (motorDirection*whichDir != 1) {digitalWrite(DirPin, LOW);} else {digitalWrite(DirPin, HIGH);}
  //Serial.println("OK3");
    float MotorDelay = ( 1000000 * ( 60 / (steps * ResetSpeed) ) ) / 2; 
    //Serial.println("EMPEZAMOS A MOVER");
    Serial.println("Numero maximo de pasos: ");Serial.print(maxResetSteps);
    Serial.println("X: ");Serial.print(x);
    
    delay(1000);
    while(x<maxResetSteps)
    {
      digitalWrite(stepPin, HIGH);
      delayInMicroseconds(MotorDelay);          
      digitalWrite(stepPin, LOW); 
      delayInMicroseconds(MotorDelay);
      if (digitalRead(LimitPin)==HIGH) {Serial.println("Ha tocado el micro!!!"); x=maxResetSteps;}
       x+=1;
    //   if (x>=maxResetSteps) {Serial.print("Alcanzado numero maximo de pasos: ");Serial.print(maxResetSteps);}
     }
    
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the angle to move the machine to after the limit switch has been triggered.
float positionAfterReset(float limitAngle)
{
  float endAltAndAz;
  if (abs(limitAngle) == limitAngle) {endAltAndAz = limitAngle - moveAwayFromLimit;} else {endAltAndAz = limitAngle + moveAwayFromLimit;} 
  return endAltAndAz; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the angles for the heliostat (returnaltaz = 1 will return alt, 2 returns az)
void FindHeliostatAltAndAz(float SunsAltitude, float SunsAzimuth, float targetalt, float targetaz, float &machinealt, float &machineaz)
{
  float x,y,z,z1,z2,x1,x2,y1,y2,hyp,dist;
  
  z1 = sin(to_rad(SunsAltitude));
  hyp = cos(to_rad(SunsAltitude));
  x1 = hyp*cos(to_rad(SunsAzimuth*-1));
  y1 = hyp*sin(to_rad(SunsAzimuth*-1));

  z2 = sin(to_rad(targetalt));
  hyp = cos(to_rad(targetalt));
  x2 = hyp*cos(to_rad(targetaz*-1));
  y2 = hyp*sin(to_rad(targetaz*-1));  
  
  x=(x1-x2)/2+x2;
  y=(y1-y2)/2+y2;
  z=(z1-z2)/2+z2;
  
  dist=sqrt(x*x+y*y+z*z);
  if ((dist>-0.0001) && (dist <0.0001)) dist=0.0001;

  machinealt=to_deg(asin(z/dist));
  machineaz=to_deg(atan2(y*-1,x));
}

float to_rad(float angle)
{
  return angle*(pi/180);
}

float to_deg(float angle)
{
  return angle*(180/pi);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This function returns a float number typed into the serial monitor
float getFloatFromSerialMonitor()
{
  float f=0;    
  int x=0;  
  while (x<1)
  {  
    String str;   
    // when characters arrive over the serial port...
    if (Serial.available())
    {
      // wait a bit for the entire message to arrive
      delay(100);
      int i=0;
      // read all the available characters
      while (Serial.available() > 0)
      {
       char inByte = Serial.read();
        str=str+inByte;
        inData[i]=inByte;
        i+=1;
        x=2;
      }
      f = atof(inData);
      memset(inData, 0, sizeof(inData));  
    }
    if (digitalRead(manualModeOnOffPin)==LOW) x=2;
  }//END WHILE X<1  
  return f; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ManualControlThroughSerial()
{
  float newAlt,newAz, newTargetAlt, newTargetAz; 
  MachineOff(0); // Apagamos los motores porque de lo contrario se sobrecalientan si mantenemos el modo manual activo mucho tiempo  
  Serial.println("");
  Serial.print("Target actual (");
  Serial.print(machineTargetAz,4);      
  Serial.print(" AZ, ");
  Serial.print(machineTargetAlt,4);      
  Serial.println(" AL)");      
  Serial.println("Nueva AZ Target:"); 
  newTargetAz = getFloatFromSerialMonitor();
  Serial.println(newTargetAz,4);   
  Serial.println("Nueva AL Target:");    
  newTargetAlt = getFloatFromSerialMonitor();
  Serial.println(newTargetAlt,4);   
   
  Serial.println("Confirma datos? Si=1, No=0");
  int yesOrNo = getFloatFromSerialMonitor(); 
  Serial.println(yesOrNo); 
  if (yesOrNo==1)
  {
    machineTargetAlt = newTargetAlt;
    machineTargetAz = newTargetAz;
    eepromWriteFloat(2*16*1-1, machineTargetAlt);  
    eepromWriteFloat(4+2*16*1-1, machineTargetAz);  
    Serial.println("Configuracion almacenada en memoria");
  } 
  MachineOn(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float eepromReadFloat(int address)
{
  union u_tag 
  {
    byte b[4];
    float fval;
  } u;   
  
  u.b[0] = EEPROM.read(address);
  u.b[1] = EEPROM.read(address+1);
  u.b[2] = EEPROM.read(address+2);
  u.b[3] = EEPROM.read(address+3);
  return u.fval;
}
 
void eepromWriteFloat(int address, float value)
{
  union u_tag
  {
    byte b[4];
    float fval;
  } u;
  u.fval=value;
 
  EEPROM.write(address  , u.b[0]);
  EEPROM.write(address+1, u.b[1]);
  EEPROM.write(address+2, u.b[2]);
  EEPROM.write(address+3, u.b[3]);
}

float eepromReadByte(int address)
{
  return EEPROM.read(address);
}
 
void eepromWriteByte(int address, byte value)
{
  EEPROM.write(address  , value);
}

byte machineToByte(int MachineNumber)
{
  byte x=2;
  if(MachineNumber==0) x=1;
  if(MachineNumber>0)
  {
    for (int i=1;i<MachineNumber;i++)
    {
      x=2*x;
    }
  }
  return x;
}

void MachineOn(int number)
{
  digitalWrite(EnablePin, HIGH);
}
 
void MachineOff(int number)
{
  digitalWrite(EnablePin, LOW);
}
