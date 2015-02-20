///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This code calculates the length of the linear actuator.
float leadscrewLength(float b,float c,float AcuteObtuse,float angle, float AngleAtZero){
  float output;
  float angleCopy=angle; // UGO. Estas copias se hacen porque creo que las variables se pasan por referencia en Arduino y asi no las modificamos en otras funciones. Verificar
  if (AcuteObtuse == 1){angleCopy = AngleAtZero - angleCopy;}
  if (AcuteObtuse == 2){angleCopy = AngleAtZero + angleCopy;}

  output = sqrt(b * b + c * c - 2 * b * c * cos((angleCopy) * pi / 180));
  return output;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long linearActuatorMoveMotor(int altOrAz, float MachinesPreviousAngle, float MachinesNewAngle, float GearRatio, float MotorDirection, float b, float c, int AcuteObtuse, float AngleAtZero){
      float NewLength = leadscrewLength(b, c, AcuteObtuse, MachinesNewAngle, AngleAtZero);//New Leadscrew Length
      float PreviousLength = leadscrewLength(b, c, AcuteObtuse, MachinesPreviousAngle, AngleAtZero);//Previous Leadscrew Length
     float ChangeInLength = NewLength - PreviousLength;
      float NumberOfSteps;
          if (altOrAz==1){
              NumberOfSteps = steps * ChangeInLength * GearRatio * MotorDirection+altLeftoverSteps; 
              if (abs(NumberOfSteps)==NumberOfSteps){altLeftoverSteps=abs(float(NumberOfSteps-float(long(NumberOfSteps))));}else{
              altLeftoverSteps=abs(float(NumberOfSteps-float(long(NumberOfSteps))))*-1;}
          }
          
          if (altOrAz==2){
              NumberOfSteps = steps * ChangeInLength * GearRatio * MotorDirection+azLeftoverSteps; 
              if (abs(NumberOfSteps)==NumberOfSteps){azLeftoverSteps=abs(float(NumberOfSteps-float(long(NumberOfSteps))));}else{
              azLeftoverSteps=abs(float(NumberOfSteps-float(long(NumberOfSteps))))*-1;}
          }    
      return NumberOfSteps;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void linearActuatorReset(int altOrAz, float MotorDirection, float LimitAngle, float GearRatio, float b, float c, float AcuteObtuse, float AngleAtZero){    
    int dirMod = 1;
    if (AcuteObtuse==2){dirMod=-1;}
    Serial.print("Buscando limite ");
    if (altOrAz==2) Serial.println("AZ");
    if (altOrAz==1) Serial.println("AL");
    
    findLimits(altOrAz, MotorDirection*dirMod, LimitAngle);//Seeks out limit switch
    float dif = leadscrewLength(b,c,AcuteObtuse,positionAfterReset(LimitAngle), AngleAtZero) - leadscrewLength(b,c,AcuteObtuse,LimitAngle, AngleAtZero);       
    float NumberOfSteps = steps * dif * GearRatio * MotorDirection;
    Serial.print("Retrocediendo a posicion de reposo ");
    if (altOrAz==2) Serial.println("AZ");
    if (altOrAz==1) Serial.println("AL");
    
    if (altOrAz==2){moveToPosition(0, (NumberOfSteps));}//Moves motor away from limit switch
    if (altOrAz==1){moveToPosition((NumberOfSteps), 0);}//Moves motor away from limit switch
}
