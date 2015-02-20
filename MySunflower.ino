// MySunflower 
// Ugo Garcia. www.remolinomental.com. ugo.garcia@gmail.com
// Programa para control de Heliostato (sol artificial) basado en la version V097_e del Software de Gabriel Miller (www.cerebralmeltdown.com)
// Es una version reducida de este programa en la que se han quitado muchas de las configuraciones para mayor simpleza del codigo
// TODO
// Desactivar los motores independientemente para que solo este activo el que esta en movimiento (de lo contrario, el disipador del motor parado se calienta muchisimo)
// Poner el pin del interruptor de control manual con una tira de pines mas larga, para que no se suelte
// Cuando entremos en control manual, desactivar los motores para que no se calienten las controladoras
// Hay que ver a que hora empieza a funcionar el heliostato; tiene hora de fin pero no hora de inicio
// El algoritmo de como se actualiza el tiempo para activar cada iteracion no me queda claro



// QUITADO EL MOVIMIENTO EN MOVEMACHINE



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
//SoftwareSerial myLCD(0,9); // pin 3 = RX (unused), pin 9 = TX

// Configuracion inicial
void setup() 
{
  //myLCD.begin(9600); // set up serial port for 9600 baud
  //delay(500); // wait for display to boot up
  //LCDClear();
  //myLCD.write("MySunflower     CARGANDO...");
  //delay(2000);
  
  Wire.begin();
  Serial.begin(9600);
  Serial.println("MySunflower CARGANDO...");
  
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
  
  MachineOff(0);
   
  // Leemos de la memoria la posicion del Target donde hay que reflejar
  machineTargetAlt = eepromReadFloat(2*16*1-1);
  machineTargetAz  = eepromReadFloat(4+2*16*1-1);
  // ... y el modo Heliostato - Seguidor Solar
  suntrackerOrHeliostat = eepromReadByte(4+4+2*16*1-1);
  if (suntrackerOrHeliostat!=1 && suntrackerOrHeliostat!=2) suntrackerOrHeliostat=2;
  
  machinePendingReset=1;
  machineRunningState=1;
}
int iterations=0;

void loop1()
{
  /*
  if (digitalRead(altLimitPin)==HIGH)
    Serial.println("PULSADO");
  else
    Serial.println("OFF");
   delay(1000);*/
   MachineOn(0);
   
   //moveMotorWithAccel(33000, azimuthStepPin, azimuthDirPin, azSpeed, azAccel);
   moveMotorWithAccel(-34000, altitudeStepPin, altitudeDirPin, altSpeed, altAccel); 
   while(1);
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
      //Serial.println("Alcanzada hora de inicio. Arrancamos heliostato"); 
    }
    else if (machineRunningState==1 && hour>=hourReset)
    { // Ha anochecido, reseteamos maquina y esperamos a que sea el dia siguiente
      machineRunningState=2;
      machinePendingReset=1;
      //Serial.println("Alcanzada hora de reset. Aparcamos heliostato y esperamos al dia siguiente");
    }
    else if (machineRunningState==2 && hour<=hourStart)
    { // Hemos cambiado de dia, esperamos entonces a que amanezca
      machineRunningState=0;
      //Serial.println("Empieza un nuevo dia. Esperamos a que sea la hora de inicio para arrancar el heliostato");
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
    //Serial.println("Reseteando maquina...");
    LCDClear();
    //myLCD.write("Reseteando...");
    Serial.println("Reseteando...");
    MachineOn(0);  
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
    //LCDClear();
    //sprintf(lineLCD,"  %02d:%02d:%02d %02d/%02dSun: %d.%dA %d.%dE",(int)hour,(int)minute,(int)second,(int)day, (int)month, (int)SunsAzimuth, (int)((SunsAzimuth - int(SunsAzimuth)) * 10), (int)SunsAltitude, (int)((SunsAltitude - int(SunsAltitude)) * 10));
    //myLCD.write(lineLCD);
    
    Serial.println(""); Serial.print(hour,0); Serial.print(":"); Serial.print(minute, 0); Serial.print(":"); Serial.print(second, 0);
    Serial.print("  "); Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print(". Sun Pos ("); Serial.print(SunsAzimuth,3);
    Serial.print(" AZ"); Serial.print(", "); Serial.print(SunsAltitude,3); Serial.println(" AL)");
    }
    
    // Actualizamos LCD
    if (machineRunningState==0) 
    {
      LCDClear();
      //myLCD.write("Esperando amanecer...");
      Serial.println("Esperando amanecer...");
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
        dtostrf(machineTargetAz, 1, 1, line1LCD);
        dtostrf(machineTargetAlt, 1, 1, line2LCD);
        sprintf(lineLCD,"%s%sA %sE",(suntrackerOrHeliostat==1)?"SEGUIDOR. ESPEJO":"HELIO. ESPEJO   ", line1LCD, line2LCD);
      }
      else if (lcdOption==2)
      {
        sprintf(lineLCD,"%s%02d:%02d:%02d %02d/%02d",(suntrackerOrHeliostat==1)?"SEGUIDOR. HORA  ":"HELIO. HORA     ",(int)hour,(int)minute,(int)second,(int)day, (int)month);
      }

      //myLCD.write(lineLCD);
      Serial.println(lineLCD);
    }
    else
    {
      LCDClear();
      //myLCD.write("Durmiendo...");
      Serial.println("Durmiendo...");
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
