int LCDOK=0;

void printToLCD(String line)
{
  if (line.length()<=16)
  {
    myLCD.print(line);
  }
  else
  {
    myLCD.print(line.substring(0,16));
    myLCD.selectLine(2);
    myLCD.print(line.substring(16));
  }
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
  int max_options=10;
  char *options[]={(char*)"Ver target",(char*)"Modificar targetAZimuth",(char*)"Modificar targetELevation",(char*)"Ver fecha y hora",(char*)"Modificar hora",(char*)"Modificar minutos",(char*)"Modificar dia",(char*)"Modificar mes",(char*)"Modo Heliostato",(char*)"Movimiento manual AZ",(char*)"Movimiento manual EL"};
  char lineLCD[40];
  char lineLCD1[40];
  char lineLCD2[40];
  byte secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC;
   
  
  MachineOff();
  myLCD.clear();
  myLCD.print("Configuracion");
  myLCD.selectLine(2);
  myLCD.print("Usa Flechas");
  
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
      myLCD.clear();
      dtostrf(machineTargetAz, 1, 2, lineLCD1);
      dtostrf(machineTargetAlt, 1, 2, lineLCD2);
      myLCD.print("Target");
      myLCD.selectLine(2);
      sprintf(lineLCD,"%sAz %sEl", lineLCD1, lineLCD2);
      myLCD.print(lineLCD);
      
      waitForLCDKey();
      delay(200);
      waitForLCDAllKeyRelease();
    }
    // Modificar target AZ
    else if (option==1)
    {
      new_value=changeLCDValue(machineTargetAz, 0.05, 0.5, (char*)"Target AZimuth");
      if (LCDOK!=0)
      {
        machineTargetAz = new_value;
        eepromWriteFloat(4+2*16*1-1, machineTargetAz);
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar target EL
    else if (option==2)
    {
      new_value=changeLCDValue(machineTargetAlt, 0.05, 0.5, (char*)"Target ELevation");
      if (LCDOK!=0)
      {
        machineTargetAlt = new_value;
        eepromWriteFloat(2*16*1-1, machineTargetAlt);
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Ver fecha / hora
    else if (option==3)
    {
      myLCD.clear();
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC);

      #ifdef NO_CLOCK
      secondRTC=dummy_second;
      minuteRTC=dummy_minute;
      hourRTC=dummy_hour;
      dayOfMonthRTC=dummy_day;
      monthRTC=dummy_month;
      yearRTC=16;
      #endif

      sprintf(lineLCD,"  %02d:%02d:%02d %02d/%02d",(int)hourRTC,(int)minuteRTC,(int)secondRTC,(int)dayOfMonthRTC, (int)monthRTC);
      myLCD.print(lineLCD);
      
      waitForLCDKey();
      delay(200);
      waitForLCDAllKeyRelease();      
    }
    // Modificar hora
    else if (option==4)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      #ifdef NO_CLOCK
      hourRTC=dummy_hour;
      #endif
      new_value=changeLCDValue(hourRTC, 1, 1, (char*)"Hora");
      #ifdef NO_CLOCK
      dummy_hour=new_value;
      #endif
     
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, (byte)new_value, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC); 
      
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar minutos
    else if (option==5)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      #ifdef NO_CLOCK
      minuteRTC=dummy_minute;
      #endif
      new_value=changeLCDValue(minuteRTC, 1, 1, (char*)"Minutos");
      #ifdef NO_CLOCK
      dummy_minute=new_value;
      #endif
      
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(0, (byte)new_value, hourRTC, dayOfWeekRTC, dayOfMonthRTC, monthRTC, yearRTC); 
      
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar dia
    else if (option==6)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      #ifdef NO_CLOCK
      dayOfMonthRTC=dummy_day;
      #endif
      new_value=changeLCDValue(dayOfMonthRTC, 1, 1, (char*)"Dia");
      #ifdef NO_CLOCK
      dummy_day=new_value;
      #endif
      
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, (byte)new_value, monthRTC, yearRTC); 
      
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modificar mes
    else if (option==7)
    {
      RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
      #ifdef NO_CLOCK
      dayOfMonthRTC=dummy_month;
      #endif
      
      new_value=changeLCDValue(dayOfMonthRTC, 1, 1, (char*)"Mes");
      #ifdef NO_CLOCK
      dummy_month=new_value;
      #endif
      
      if (LCDOK!=0)
      {
        RTC_Code::getDateDs1307(&secondRTC, &minuteRTC, &hourRTC, &dayOfWeekRTC, &dayOfMonthRTC, &monthRTC, &yearRTC); 
        RTC_Code::setDateDs1307(secondRTC, minuteRTC, hourRTC, dayOfWeekRTC, dayOfMonthRTC, (byte)new_value, yearRTC); 
      
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Modo heliostato
    else if (option==8)
    {
      new_value=changeLCDValue(suntrackerOrHeliostat, 1, 1, (char*)"1-Seguir 2-Helio");
      if (new_value==1 || new_value==2)
      {
        eepromWriteByte(4+4+2*16*1-1, (byte)new_value);
        suntrackerOrHeliostat=new_value;
        myLCD.clear();
        myLCD.print("Almacenado");
        delay(1500);  
    
        waitForLCDAllKeyRelease();
      }
    }
    // Movimiento manual AZ
    else if (option==9)
    {
      new_value=changeLCDValue(machineCurrentAz, 0.05, 0.5, (char*)"Angulo AZ");
      if (LCDOK!=0)
      {
        myLCD.clear();
        myLCD.print("Moviendo AZ...");
        delay(1000);  
        
        moveMachine(machineCurrentAlt, new_value, true);
        delay(500);
        waitForLCDAllKeyRelease();
      }
    }
    // Movimiento manual EL
    else if (option==10)
    {
      new_value=changeLCDValue(machineCurrentAlt, 0.05, 0.5, (char*)"Angulo EL");
      if (LCDOK!=0)
      {
        myLCD.clear();
        myLCD.print("Moviendo EL...");
        delay(1000);  
        
        moveMachine(new_value, machineCurrentAz, true);
        delay(500);
        waitForLCDAllKeyRelease();
      }
    }
  }
  
  if (option!=-1)
  {
    // Imprimimos opcion actual
    myLCD.clear();
    printToLCD(options[option]);
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
  myLCD.clear();
  myLCD.print(title);
  myLCD.selectLine(2);
  dtostrf(newValue, 1, 2, lineLCD);
  myLCD.print(lineLCD);
  
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
        myLCD.selectLine(2);
        dtostrf(newValue, 1, 2, lineLCD);
        myLCD.print(lineLCD);
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
        myLCD.selectLine(2);
        dtostrf(newValue, 1, 2, lineLCD);
        myLCD.print(lineLCD);
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
      myLCD.clear();
      myLCD.print("CANCEL");
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
