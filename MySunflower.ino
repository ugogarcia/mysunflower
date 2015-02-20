// MySunflower 
// Ugo Garcia. www.remolinomental.com. ugo.garcia@gmail.com
// Programa para control de Heliostato (sol artificial) basado en la version V097_e del Software de Gabriel Miller (www.cerebralmeltdown.com)
// Es una version reducida de este programa en la que se han quitado muchas de las configuraciones para mayor simpleza del codigo
// MEJORAS
// Desactivar los motores independientemente para que solo este activo el que esta en movimiento (de lo contrario, el disipador del motor parado se calienta muchisimo)

/* BUGS PENDIENTES
- OK. PROBAR. Ver porque en la pantala cuando sale la posicion del ESPEJO sale mal (despues se posiciona bien, pero lo que pinta esta mal)
- OK. PROBAR EN LOS DISTINTOS ESTADOS (andando, durmiendo, esperando amanecer). Parar los motores cuando se este "esperando"
- OK. PARECE FUNCIONAR SOLO CON INTS. SE HA CREADO UN MACRO NUEVO LLAMADO ABS. Ver porque el abs con un long a veces devuelve un numero raro cuando el dato es mayor de 32000
- OK. HE INICIALIZADO VARIABLE A 0. ES LO UNICO QUE PARECIA ESTAR MAL. En Functions->SearchForLimits si quitamos el Print de "x" y "maxResetSteps" no entra en el bucle, vamos, que no se mueve el motor buscando el limite
- OK. SE ENVIA POR AMBOS SITIOS Y SE CONTROLA EL SERIAL POR UNA VARIABLE. Enviar la info por LCD y serial asi tenemos las dos opciones. Usar defines LCD_ON y SERIAL_ON
- OK. SE HA CALCULADO. PROBAR A RESETEAR EN LA POSICION MAXIMA A VER SI LLEGA CON LOS VALORES SETEADOS. AltitudeMax y AzimuthMax habria que setearlo a un valor mas realista que ahora mismo estan a 1000000
- linearActuatorMoveMotor devuelve numero negativo para altitud. Ver porque
*/

#include "config.h"
#include "Wire.h"  
#include <Flash.h>
#include <EEPROM.h>
#include "SunPositionAlgo_LowAc.h"
#include "RTC_Code.h"
#include <SoftwareSerial.h>

byte suntrackerOrHeliostat=2;            // Set 1. 1 - Suntracking, 2 - Heliostato
int lcdOption=0;
int updateLCD=0;

// Attach the serial display's RX line to digital pin 2
SoftwareSerial myLCD(0,9); // pin 3 = RX (unused), pin 9 = TX

// Configuracion inicial
void setup() 
{
  myLCD.begin(9600); // set up serial port for 9600 baud
  Serial.begin(9600);
  delay(500); // wait for display to boot up
  
  LCDClear();
  LCDWrite("MySunflower     CARGANDO...");
  delay(1000);
  
  // Solo configuracmos los pines de salida, pues por defecto, Arduino los configura como de entrada
  pinMode(altitudeStepPin, OUTPUT);
  pinMode(altitudeDirPin, OUTPUT);
  pinMode(azimuthStepPin, OUTPUT);
  pinMode(azimuthDirPin, OUTPUT);
  pinMode(EnablePin, OUTPUT);  

  // Botones del LCD
  pinMode(10, INPUT);
  digitalWrite(10, HIGH);
  pinMode(11, INPUT);
  digitalWrite(11, HIGH);  
  pinMode(12, INPUT);  
  digitalWrite(12, HIGH);
      
  // Configuracion puerto serie
  // MCP23017 SETUP CODE FROM http://tronixstuff.wordpress.com/tutorials > chapter 41
  // setup addressing style
  Wire.begin();
  Wire.beginTransmission(0x20);
  Wire.write((byte)0x12);
  Wire.write((byte)0x20); // use table 1.4 addressing
  Wire.endTransmission();
  // set I/O pins to outputs
  Wire.beginTransmission(0x20);
  Wire.write((byte)0x00); // IODIRA register
  Wire.write((byte)0x00); // set all of bank A to outputs
  Wire.write((byte)0x00); // set all of bank B to outputs
  Wire.endTransmission();
  // END MCP23017 SETUP CODE
  
  MachineOff();
   
  // Leemos de la memoria la posicion del Target donde hay que reflejar
  machineTargetAlt = eepromReadFloat(2*16*1-1);
  machineTargetAz  = eepromReadFloat(4+2*16*1-1);
  // ... y el modo Heliostato - Seguidor Solar
  suntrackerOrHeliostat = eepromReadByte(4+4+2*16*1-1);
  if (suntrackerOrHeliostat!=1 && suntrackerOrHeliostat!=2) suntrackerOrHeliostat=1;
  
  machinePendingReset=1;
  machineRunningState=1;
}
int iterations=0; 

void loop3()
{
  /*
  if (digitalRead(altLimitPin)==HIGH)
    Serial.println("PULSADO");
  else
    Serial.println("OFF");
   delay(1000);*/
   //MachineOn();
   
   //moveMotorWithAccel(33000, azimuthStepPin, azimuthDirPin, azSpeed, azAccel);
   //moveMotorWithAccel(-34000, altitudeStepPin, altitudeDirPin, altSpeed, altAccel); 
   /*
  float numOfSteps;
  numOfSteps=-34000;
  //numOfSteps=((numOfSteps)>0?(numOfSteps):-(numOfSteps));
  Serial.print("Numero1: ");
  Serial.println(numOfSteps);
  for (numOfSteps=32500; numOfSteps<=33000; numOfSteps++)
  {
    Serial.print("abs(");
    Serial.print(numOfSteps);
    Serial.print(")=");
    Serial.println(ABS(numOfSteps));
  }
  */
  /*long azsteps = linearActuatorMoveMotor(2, 45, -45, azGearRatio, azMotorDirection, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);
  long altsteps = linearActuatorMoveMotor(1, -30, +45, altGearRatio, altMotorDirection, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);
   
   Serial.println("AZSTEPS");
   Serial.println(azsteps);
   Serial.println("ALTSTEPS");
   Serial.println(altsteps);
   
   #ifdef LCD_ON 
     LCDClear();
     myLCD.write("LCD ON");
   #endif
   
   while(1);
  */
}

// Bucle principal
void loop()
{  
  int month, day;
  float hour, minute, second;
  byte secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC;
   
  // Obtenemos hora actual y la almacenamos en las varaibles correspondientes. Aqui podemos trucar para hacer pruebas
  RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC);
  month = monthRTC;
  day = dayOfMonthRTC;
  hour = hourRTC;
  minute = minuteRTC;
  second = secondRTC;

  // Controlamos la hora para saber si debemos funcionar, resetear o esperar al amanecer
  if ((digitalRead(manualModeOnOffPin)!=HIGH))
  {    
    if (machineRunningState==0 && hour>=hourStart)
    { // Ha amanecido, empezamos a mover el Heliostato
      machineRunningState=1;
      LCDClear();
      LCDWrite("Hora de comienzoArrancando...");
      delay(1000);
    }
    else if (machineRunningState==1 && hour>=hourReset)
    { // Ha anochecido, reseteamos maquina y esperamos a que sea el dia siguiente
      machineRunningState=2;
      machinePendingReset=1;
      LCDClear();
      LCDWrite("Hora final.     Espera nuevo dia");
      delay(1000);
    }
    else if (machineRunningState==2 && hour<=hourStart)
    { // Hemos cambiado de dia, esperamos entonces a que amanezca
      machineRunningState=0;
      LCDClear();
      LCDWrite("Nuevo dia.      Espera amanecer");
      delay(1000);
    }
    else if (machineRunningState==1 && hour<hourStart)
    {
      machineRunningState=0; 
    }
  }
  
  // Actualizacion del tiempo que ha pasado desde la ultima iteracion
  if ((digitalRead(manualModeOnOffPin)!=HIGH))
  {    
    now = millis();
    if ((now + updateEvery*1000) < updateTime) 
    {
      updateTime=now+updateEvery;
    }
  }

  // Si la maquina esta pendiente de reseteo, lo realizamos
  if (machinePendingReset==1)
  {
    LCDClear();
    LCDWrite("Reseteando...");
    resetPositionOfMachine();
    machinePendingReset=0;
  }

  // ¿Es tiempo ya de movernos?
  if (now>=updateTime || updateLCD==1)
  { 
    char lineLCD[40];
    char line1LCD[40];
    char line2LCD[40];
    updateLCD=0;
    
    // Actualizamos Posicion del Helio
    if (now>=updateTime)
    {
      updateTime=updateEvery*1000 + millis();
      SunPositionAlgo_LowAc::CalculateSunsPositionLowAc(month, day, hour, minute, second, timezone, latitude, longitude, SunsAltitude, SunsAzimuth, delta, h);
      SunsAltitude = SunsAltitude + (1.02/tan((SunsAltitude + 10.3/(SunsAltitude + 5.11)) * pi/180.0))/60.0;//Refraction Compensation: Meeus Pg. 105
    }
    
    // Actualizamos LCD
    if (machineRunningState==0) 
    {
      LCDClear();
      LCDWrite("Esperando amanecer...");
    }
    else if (machineRunningState==1)
    {
      moveMachine(0, 0, machineTargetAlt, machineTargetAz);
    
      LCDClear();
      if (lcdOption==0)
      {
        dtostrf(SunsAzimuth, 1, 1, line1LCD);
        dtostrf(SunsAltitude, 1, 1, line2LCD);
        sprintf(lineLCD,"%s%sA %sE",(suntrackerOrHeliostat==1)?"SEGUIDOR. SOL   ":"HELIO. SOL      ", line1LCD, line2LCD);
      }
      else if (lcdOption==1)
      {
        dtostrf(machineCurrentAz, 1, 1, line1LCD);
        dtostrf(machineCurrentAlt, 1, 1, line2LCD);
        sprintf(lineLCD,"%s%sA %sE",(suntrackerOrHeliostat==1)?"SEGUIDOR. ESPEJO":"HELIO. ESPEJO   ", line1LCD, line2LCD);
      }
      else if (lcdOption==2)
      {
        sprintf(lineLCD,"%s%02d:%02d:%02d %02d/%02d",(suntrackerOrHeliostat==1)?"SEGUIDOR. HORA  ":"HELIO. HORA     ",(int)hour,(int)minute,(int)second,(int)day, (int)month);
      }

      LCDWrite(lineLCD);
    }
    else
    {
      LCDClear();
      LCDWrite("Durmiendo...");
    }
  }  
  
  // Control de la maquina por el puerto serie si hemos activado el modo manual
  if ((digitalRead(manualModeOnOffPin)==HIGH))
  {  
    ManualControlThroughLCD();
    updateTime = millis();
  }
  
  if (digitalRead(10)!=HIGH || digitalRead(11)!=HIGH || digitalRead(12)!=HIGH)
  {
    waitForLCDAllKeyRelease(); 
    delay(200);
    if (++lcdOption>2) lcdOption=0;
    updateLCD=1;
  }
}

// Bucle principal de pruebas
/*
int ok=0;
void loop1()
{
  if (ok==0)
  {
    float azsteps;
    Serial.println("Moviendo 45 grados altitud");
    MachineOn(1);
  
    azsteps = linearActuatorMoveMotor(1, 0, 45, azGearRatio, azMotorDirection, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);
    Serial.print("Pasos: ");
    Serial.println(azsteps);
    Serial.println("Moviendo...");
    moveToPosition(azsteps, 0);
    ok=1;
    MachineOff(1);
  }
}
*/
