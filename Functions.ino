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
  //long pasos=0;
  //Serial.print("1. Steps: ");
  //Serial.println(numOfSteps);
  if ((long)numOfSteps>=0) {digitalWrite(DirPin, HIGH);} else {digitalWrite(DirPin, LOW);}  
  
  float minMotorDelay  = (1000000/((maxspeed*steps)/60))/2;
  float halfWay = ABS(long(numOfSteps/2));
  float maxMotorDelay=((1000000/((maxspeed*steps)/60))*100)/2;
  float motorDelay=maxMotorDelay;  

  /*Serial.print("Me voy a mover: ");
  Serial.println(ABS(numOfSteps));*/
  
  float motorDelayAdjust=((maxMotorDelay-minMotorDelay)*(maxMotorDelay-minMotorDelay))/(Accel*1000000);               
  float numberOfStepsToReachMinFromMax=long((maxMotorDelay-minMotorDelay)/motorDelayAdjust);    
  if(-0.00001<Accel&&Accel<0.00001){motorDelay=minMotorDelay;}
  
  MachineOn();   
  if (ABS(numOfSteps)>numberOfStepsToReachMinFromMax*2)
  {//This code runs if the accleration slope "flat lines"
    //Serial.println("ENTRO POR A");
    for (long doSteps=1; doSteps <= ABS(numOfSteps); doSteps++)
    {
      //pasos++;
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, HIGH);
      delayInMicroseconds(motorDelay);
      digitalWrite(stepPin, LOW);
      if ((motorDelay>minMotorDelay)&&(doSteps<halfWay)) {motorDelay=motorDelay-motorDelayAdjust;}//Accelerates the motor
      if ((motorDelay<minMotorDelay)) {motorDelay=minMotorDelay;}//Holds the motor at its max speed
      if ((doSteps>(ABS(numOfSteps)-numberOfStepsToReachMinFromMax))) {motorDelay=motorDelay+motorDelayAdjust;}//Deccelerates the motor after it gets close to the end of its move
    }
  }
  else
  {//This code runs if the acceleration slope is an upside down V.
  //  Serial.println("ENTRO POR B");
    for (long doSteps=1; doSteps <= ABS(numOfSteps); doSteps++)
    {
      //pasos++;
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
  
  MachineOff();
  
  /*Serial.print("Me he movido de pasos: ");
  Serial.println(pasos);*/
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
    searchForLimit(altOrAz, limitAngle, altitudeDirPin, altitudeStepPin, altResetSpeed, altLimitPin, altitudeMax, motorDirection );

  if (altOrAz==2)
    searchForLimit(altOrAz, limitAngle, azimuthDirPin, azimuthStepPin, azResetSpeed, azLimitPin, azimuthMax, motorDirection );
}

void searchForLimit(int altOrAz, float limitAngle, int DirPin, int stepPin, int ResetSpeed, int LimitPin, long maxResetSteps, int motorDirection)
{
  long x=0;
  int microReached=0;
  int whichDir;
    
  if (motorDirection!=0)
  {//A motorDirection of 0 will skip the reset  
    if (ABS(limitAngle) != limitAngle) {whichDir = 1;} else {whichDir = -1;}
    if (motorDirection*whichDir != 1) {digitalWrite(DirPin, LOW);} else {digitalWrite(DirPin, HIGH);}
    float MotorDelay = ( 1000000 * ( 60 / (steps * ResetSpeed) ) ) / 2; 
    
    MachineOn();
    while(x<maxResetSteps)
    {
      digitalWrite(stepPin, HIGH);
      delayInMicroseconds(MotorDelay);          
      digitalWrite(stepPin, LOW); 
      delayInMicroseconds(MotorDelay);
      if (digitalRead(LimitPin)==HIGH) 
      {
        myLCD.clear();
        if (altOrAz==1)
          printToLCD("Click micro Elevation pos 0");
        else
          printToLCD("Click micro Azimuth pos 0");
        x=maxResetSteps;
        microReached=1;
      }
      x+=1;
     }
    MachineOff();
    
    if (microReached==0)
    {
      myLCD.clear();
      if (altOrAz==1)
        printToLCD("ERROR. Micro Elevation pos 0");
      else
        printToLCD("ERROR. Micro Azimuth pos 0");
      while(1);
    }
    
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the angle to move the machine to after the limit switch has been triggered.
float positionAfterReset(float limitAngle)
{
  float endAltAndAz;
  if (ABS(limitAngle) == limitAngle) {endAltAndAz = limitAngle - moveAwayFromLimit;} else {endAltAndAz = limitAngle + moveAwayFromLimit;} 
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

void MachineOn()
{
  digitalWrite(EnablePin, HIGH);
}
 
void MachineOff()
{
  digitalWrite(EnablePin, LOW);
}
