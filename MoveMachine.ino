void moveMachine(float preTargetAlt, float preTargetAz, float targetalt, float targetaz)
{   
  float machineNewAltitude, machineNewAzimuth, altsteps, azsteps; 
  char lineLCD[40];
    
  if (SunsAltitude>=0 || digitalRead(manualModeOnOffPin)==HIGH)
  { // Empezamos a movrnos cuando la altitud del Sol sea positiva. UGO -> ¿Podiamos mejor ponerle una hora de comienzo del funcionamiento no?
    // Tambien nos movemos si estamos en modo manual                                                                                                       
                                                                                                              
    if (suntrackerOrHeliostat == 1)
    { // La maquina esta funcionando como un Seguidor Solar
      machineNewAltitude = SunsAltitude;
      machineNewAzimuth = SunsAzimuth;
    }

    if (suntrackerOrHeliostat == 2)
    { // La maquina esta funcionando como un Heliostato
      // Calculamos la posicion que debe tomar el heliostato para reflejar la luz en el Target configurado
      FindHeliostatAltAndAz(SunsAltitude, SunsAzimuth, targetalt, targetaz, machineNewAltitude, machineNewAzimuth); 
    }
       
    if ((machineNewAzimuth > minAz) && (machineNewAltitude > minAlt) && (machineNewAzimuth < maxAz) && (machineNewAltitude < maxAlt))
    {
      azsteps = linearActuatorMoveMotor(2, machineCurrentAz, machineNewAzimuth, azGearRatio, azMotorDirection, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);
      altsteps = linearActuatorMoveMotor(1, machineCurrentAlt, machineNewAltitude, altGearRatio, altMotorDirection, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);
   
      if (ABS(altsteps)>0 || ABS(azsteps)>0)
      {
        MachineOn();  
        LCDClear();
        dtostrf(azsteps, 1, 0, lineLCD);
        LCDWrite("Moviendo...     ");
        LCDWrite(lineLCD);
        LCDWrite("AZ ");
        dtostrf(altsteps, 1, 0, lineLCD);
        LCDWrite(lineLCD);
        LCDWrite("EL");
        moveToPosition(altsteps, azsteps);
      }
        
      machineCurrentAlt = machineNewAltitude;
      machineCurrentAz = machineNewAzimuth;       
    } 
    else 
    {
      LCDClear();
      LCDWrite("Movimiento FUERAde limites");
      delay(2000);  
    }
  }
  MachineOff();  
}

void resetPositionOfMachine(){
    LCDClear();
    LCDWrite("Reseteando...   Azimuth");
    linearActuatorReset(2, azMotorDirection, azLimitAngle, azGearRatio, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);    
    delay(1000);
    LCDClear();
    LCDWrite("Reseteando...   Elevacion");
    linearActuatorReset(1, altMotorDirection, altLimitAngle, altGearRatio, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);        
    delay(1000);
    LCDClear();
    LCDWrite("Reseteando...   OK!");
    machineCurrentAlt = positionAfterReset(altLimitAngle); 
    machineCurrentAz = positionAfterReset(azLimitAngle);
    delay(1000);
}
