void moveMachine(float targetalt, float targetaz, bool manual_mode)
{   
  float machineNewAltitude, machineNewAzimuth, altsteps, azsteps; 
  char lineLCD[40];
  char lineLCD1[40];
  char lineLCD2[40];
  
  if (SunsAltitude>=0 || digitalRead(manualModeOnOffPin)==HIGH)
  { // Empezamos a movernos cuando la altitud del Sol sea positiva. UGO -> ¿Podiamos mejor ponerle una hora de comienzo del funcionamiento no?
    // Tambien nos movemos si estamos en modo manual                                                                                                       
    if (manual_mode)
    {
      // Estamos en modo manual, así que usamos las coordenadas pasadas como paráemtros                                                                                                     
      machineNewAltitude = targetalt;
      machineNewAzimuth = targetaz;
    }
    else
    {
      // Modo automático. Si estamos en modo seguidor solar tomamos como target el Sol, en caso Helio calculamos la posición
      if (suntrackerOrHeliostat == 1)
      {
        machineNewAltitude = SunsAltitude;
        machineNewAzimuth = SunsAzimuth;
      }
      else
      {
        FindHeliostatAltAndAz(SunsAltitude, SunsAzimuth, machineTargetAlt, machineTargetAz, machineNewAltitude, machineNewAzimuth); 
      }
    }
     
    if ((machineNewAzimuth > minAz) && (machineNewAltitude > minAlt) && (machineNewAzimuth < maxAz) && (machineNewAltitude < maxAlt))
    {
      azsteps = linearActuatorMoveMotor(2, machineCurrentAz, machineNewAzimuth, azGearRatio, azMotorDirection, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);
      altsteps = linearActuatorMoveMotor(1, machineCurrentAlt, machineNewAltitude, altGearRatio, altMotorDirection, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);
   
      if (ABS(altsteps)>0 || ABS(azsteps)>0)
      {
        MachineOn();  
        myLCD.clear();
        dtostrf(azsteps/steps, 1, 0, lineLCD1);
        dtostrf(altsteps/steps, 1, 0, lineLCD2);
        sprintf(lineLCD,"%sAz %sEl", lineLCD1, lineLCD2);
        myLCD.print("Moviendo vueltas");
        myLCD.selectLine(2);
        myLCD.print(lineLCD);
        moveToPosition(altsteps, azsteps);
      } 
  
      machineCurrentAlt = machineNewAltitude;
      machineCurrentAz = machineNewAzimuth;       
    } 
    else 
    {
      myLCD.clear();
      myLCD.print("Movimiento FUERA");
      myLCD.selectLine(2);
      myLCD.print("de limites");
      delay(2000);  
    }
  }
  
  MachineOff();  
}

void resetPositionOfMachine(){
    myLCD.clear();
    myLCD.print("Reseteando...");
    myLCD.selectLine(2);
    myLCD.print("Azimuth");
    linearActuatorReset(2, azMotorDirection, azLimitAngle, azGearRatio, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);    
    delay(250);
    myLCD.clear();
    myLCD.print("Reseteando...");
    myLCD.selectLine(2);
    myLCD.print("Elevacion");
    linearActuatorReset(1, altMotorDirection, altLimitAngle, altGearRatio, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);        
    delay(250);
    myLCD.clear();
    myLCD.print("Reset finalizado");
    machineCurrentAlt = positionAfterReset(altLimitAngle); 
    machineCurrentAz = positionAfterReset(azLimitAngle);
    delay(250);
}
