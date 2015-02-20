int LCDOK=0;

void LCDClear()
{
  myLCD.write(254); // move cursor to beginning of first line
  myLCD.write(128);

  myLCD.write("                "); // clear display
  myLCD.write("                ");

  myLCD.write(254); // move cursor to beginning of first line
  myLCD.write(128);
}

void LCDWrite (char *string_to_write)
{
  myLCD.write(string_to_write);  
  if (printLCDToSerial==1) Serial.println(string_to_write);
}

void LCDWriteChar (char c)
{
  myLCD.write(c);  
}

void waitForLCDKey()
{
  while (digitalRead(10)==HIGH && digitalRead(11)==HIGH && digitalRead(12)==HIGH && digitalRead(manualModeOnOffPin)==HIGH);
}

void waitForLCDAllKeyRelease()
{
  while (digitalRead(10)!=HIGH || digitalRead(11)!=HIGH || digitalRead(12)!=HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ManualControlThroughLCD()
{
  float new_value; 
  int option=-1;
  int max_options=8;
  char *options[]={"Ver target","Modificar targetAZimuth","Modificar targetELevation", "Ver fecha y hora", "Modificar hora", "Modificar minutos", "Modificar dia", "Modificar mes","Modo Heliostato"};
  char lineLCD[40];
  byte secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC;
   
  
  MachineOff();
  LCDClear();
  LCDWrite("Configuracion   Usa Flechas");
  
  while (1)
  {
  // Esperamos a que se pulse un boton o se anule el modo manual
  waitForLCDKey();
  
  // Modo manual desactivado, salimos
  if (digitalRead(manualModeOnOffPin)!=HIGH) return;
  
  // Boton retroceso
  if (digitalRead(10)!=HIGH)
  {
    if (option==-1) option=1;
    delay(200);
    // Esperamos a que se suelte el boton
    while (digitalRead(10)!=HIGH);
    if (--option<0) option=max_options;
  }
  // Boton avance
  else if (digitalRead(11)!=HIGH)
  {
    delay(200);
    // Esperamos a que se suelte el boton
    while (digitalRead(11)!=HIGH);
    if (++option>max_options) option=0;
  }
  // Boton seleccionar
  else if (digitalRead(12)!=HIGH && option!=-1)
  {
    delay(200);
    // Esperamos a que se suelte el boton
    while (digitalRead(12)!=HIGH);
    
    // Actuamos en funcion de la opcion seleccionada
    // Ver target actual
    if (option==0)
    {
      LCDClear();
      dtostrf(machineTargetAz, 1, 2, lineLCD);
      LCDWrite(lineLCD);
      LCDWrite("AZ ");
      dtostrf(machineTargetAlt, 1, 2, lineLCD);
      LCDWrite(lineLCD);
      LCDWrite("EL");
      waitForLCDKey();
      delay(200);
      waitForLCDAllKeyRelease();
    }
    // Modificar target AZ
    else if (option==1)
    {
      new_value=changeLCDValue(machineTargetAz, 0.05, 0.5, "Target AZimuth");
      if (LCDOK!=0)
      {
        machineTargetAz = new_value;
        eepromWriteFloat(4+2*16*1-1, machineTargetAz);
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar target EL
    else if (option==2)
    {
      new_value=changeLCDValue(machineTargetAlt, 0.05, 0.5, "Target ELevation");
      if (LCDOK!=0)
      {
        machineTargetAlt = new_value;
        eepromWriteFloat(2*16*1-1, machineTargetAlt);
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Ver fecha / hora
    else if (option==3)
    {
      LCDClear();
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC);

      sprintf(lineLCD,"  %02d:%02d:%02d %02d/%02d",(int)hourRTC,(int)minuteRTC,(int)secondRTC,(int)dayOfMonthRTC, (int)monthRTC);
      LCDWrite(lineLCD);
      
      waitForLCDKey();
      delay(200);
      waitForLCDAllKeyRelease();      
    }
    // Modificar hora
    else if (option==4)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      new_value=changeLCDValue(hourRTC, 1, 1, "Hora");
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, (byte)new_value, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC); 
      
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar minutos
    else if (option==5)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      new_value=changeLCDValue(minuteRTC, 1, 1, "Minutos");
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(0, (byte)new_value, hourRTC, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC); 
      
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar dia
    else if (option==6)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      new_value=changeLCDValue(dayOfMonthRTC, 1, 1, "Dia");
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, (byte)new_value, monthRTC, yearRTC); 
      
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar mes
    else if (option==7)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      new_value=changeLCDValue(dayOfMonthRTC, 1, 1, "Mes");
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, dayOfMonthRTC, (byte)new_value, yearRTC); 
      
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modo heliostato
    else if (option==8)
    {
      new_value=changeLCDValue(suntrackerOrHeliostat, 1, 1, "1-Seguir 2-Helio");
      if (new_value==1 || new_value==2)
      {
        eepromWriteByte(4+4+2*16*1-1, (byte)new_value);
        suntrackerOrHeliostat=new_value;
        LCDClear();
        LCDWrite("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
  }
  
  if (option!=-1)
  {
    // Imprimimos opcion actual
    LCDClear();
    LCDWrite(options[option]);
  }
  }
  
  //MachineOn(0);
}

float changeLCDValue(float current_value, double min_step, double max_step, char *title)
{
  float newValue=current_value;
  char lineLCD[40];
  unsigned long now;
  LCDOK=0;
  
  while (1)
  {
  LCDClear();
  LCDWrite(title);
  LCDWriteChar(254);
  LCDWriteChar(192);
  dtostrf(newValue, 1, 2, lineLCD);
  LCDWrite(lineLCD);
  
  // Esperamos a que se pulse un boton o se anule el modo manual
  waitForLCDKey();
  
  // Modo manual desactivado, salimos
  if (digitalRead(manualModeOnOffPin)!=HIGH) return 0;
  
  // Boton retroceso
  if (digitalRead(10)!=HIGH)
  {
    now=millis();
    delay(200);
    // Esperamos a que se suelte el boton o a que pase el tiempo minimo para empezar a modificar el valor
    while (digitalRead(10)!=HIGH && millis()<now+1000);
    
    // El boton esta pulsado durante un tiempo; empezamos a modificar el valor rapidamente
    if (millis()>=now+1000)
    {
      while (digitalRead(10)!=HIGH)
      {
        newValue-=max_step;
        LCDWriteChar(254);
        LCDWriteChar(192);
        dtostrf(newValue, 1, 2, lineLCD);
        LCDWrite(lineLCD);
        delay(50);
      }
    }
    // Si se ha soltado el boton, simplemente incrementamos el valor y esperamos a una nueva tecla
    else
    {
      newValue-=min_step;
    }
  }
  // Boton avance
  else if (digitalRead(11)!=HIGH)
  {
    now=millis();
    delay(200);
    // Esperamos a que se suelte el boton o a que pase el tiempo minimo para empezar a modificar el valor
    while (digitalRead(11)!=HIGH && millis()<now+1000);
    
    // El boton esta pulsado durante un tiempo; empezamos a modificar el valor rapidamente
    if (millis()>=now+1000)
    {
      while (digitalRead(11)!=HIGH)
      {
        newValue+=max_step;
        LCDWriteChar(254);
        LCDWriteChar(192);
        dtostrf(newValue, 1, 2, lineLCD);
        LCDWrite(lineLCD);
        delay(50);
      }
    }
    // Si se ha soltado el boton, simplemente incrementamos el valor y esperamos a una nueva tecla
    else
    {
      newValue+=min_step;
    }
  }
  // Boton Seleccionar
  else if (digitalRead(12)!=HIGH)
  {
    now=millis();
    delay(200);
    
    // Esperamos a que se suelte el boton y contamos el tiempo
    while (digitalRead(12)!=HIGH && millis()<now+3000);
    
    // Si se ha mantenido pulsado mucho tiempo -> CANCEL
    if (millis()>=now+3000)
    {
      LCDClear();
      LCDWrite("CANCEL");
      delay(1500);
      return 0;
    }
    // Si se ha hecho una pulsacion corta -> OK
    else
    {
      LCDOK=1;
      return newValue; 
    }
  }
  
  }
  
  return newValue;
}
