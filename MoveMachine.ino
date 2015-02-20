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
     /* Serial.print("Objetivo a reflejar (");
      Serial.print(machineTargetAz,3);
      Serial.print(" AZ");
      Serial.print(", ");
      Serial.print(machineTargetAlt,3);
      Serial.println(" AL)");*/
    }
   
   /*
    Serial.print("Pos Objetivo Maquina (");
    Serial.print(machineNewAzimuth,3);
    Serial.print(" AZ");
    Serial.print(", ");
    Serial.print(machineNewAltitude,3);
    Serial.println(" AL)");
    */
    
    if ((machineNewAzimuth > minAz) && (machineNewAltitude > minAlt) && (machineNewAzimuth < maxAz) && (machineNewAltitude < maxAlt))
    {
      azsteps = linearActuatorMoveMotor(2, machineCurrentAz, machineNewAzimuth, azGearRatio, azMotorDirection, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);
      altsteps = linearActuatorMoveMotor(1, machineCurrentAlt, machineNewAltitude, altGearRatio, altMotorDirection, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);
   
      if (abs(altsteps)>0 || abs(azsteps)>0)
      {
        MachineOn(0);  
        LCDClear();
        dtostrf(azsteps, 1, 0, lineLCD);
        //myLCD.write("Moviendo...     ");
        //myLCD.write(lineLCD);
        //myLCD.write("AZ ");
        Serial.println("Moviendo...");
        Serial.print(lineLCD);
        Serial.print("AZ ");
        dtostrf(altsteps, 1, 0, lineLCD);
        //myLCD.write(lineLCD);
        //myLCD.write("EL");
        Serial.print(lineLCD);
        Serial.println("EL ");
   
        moveToPosition(altsteps, azsteps);
      }
        
      machineCurrentAlt = machineNewAltitude;
      machineCurrentAz = machineNewAzimuth;       
    } 
    else 
    {
      LCDClear();
      //myLCD.write("Movimiento FUERAde limites");
      delay(2000);  
      Serial.println("El movimiento excede los limites de la maquina");  
    }
  }
  MachineOff(0);  
}

void resetPositionOfMachine(){
    LCDClear();
    //myLCD.write("Reseteando...   Azimuth");
    Serial.println("Reseteando...   Azimuth");
    linearActuatorReset(2, azMotorDirection, azLimitAngle, azGearRatio, azLengthA, azLengthB, azAcuteObtuse, azAngleAtZero);    
    delay(1000);
    LCDClear();
    //myLCD.write("Reseteando...   Elevacion");
    Serial.println("Reseteando...   Elevacion");
    linearActuatorReset(1, altMotorDirection, altLimitAngle, altGearRatio, altLengthA, altLengthB, altAcuteObtuse, altAngleAtZero);        
    delay(1000);
    LCDClear();
    //myLCD.write("Reseteando...   OK!");
    Serial.println("Reseteando...   OK!");
    machineCurrentAlt = positionAfterReset(altLimitAngle); 
    machineCurrentAz = positionAfterReset(azLimitAngle);
    Serial.print("Posicion after reset AZ: ");
    Serial.println(machineCurrentAz);
    Serial.print("Posicion after reset ALT: ");
    Serial.println(machineCurrentAlt);
    
    delay(1000);
}
