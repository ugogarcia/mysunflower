// GENERALES
 
// MOVIMIENTO ALTITUD
const float altGearRatio=5;              // Set 2. Numero de vueltas por unidad de longitud
const int altMotorDirection=1;           // Set 3. 0 - Motor detenido. 1 - Movimiento normal. -1 - Movimiento reverso
const float altLengthA=12.19;            // Set 4. Longitud A del actuador lineal (uno de los lados, realmente da igual cual. Usaremos el mas corto)
const float altLengthB=43.6;             // Set 5. Lontigud B del actuador lineal (lado mas largo)
const float altAngleAtZero=90;           // Set 6. Angulo que forman A y B cuando la altitud es 0 (el espejo esta totalmente vertical)
const int altAcuteObtuse=1;              // Set 7. 1 - El angulo que forman A y B se vuelve mas agudo cuando movemos la altitud positivamente (el espejo mira mas hacia el cielo). 2 - Caso contrario
const float altLimitAngle=-33;           // Set 8. Grados de altitud alcanzados cuando la maquina pulsa el microinterruptor limite de altitud (negativo mira hacia el suelo, positivo hacia el cielo)
  
// MOVIMIENTO AZIMUTH
const float azGearRatio=5;               // Set 9. Numero de vueltas por unidad de longitud
const int azMotorDirection=1;            // Set 10. 0 - Motor detenido. 1 - Movimiento normal. -1 - Movimiento reverso
const float azLengthA=12.9;              // Set 11. Longitud A del actuador lineal (uno de los lados, realmente da igual cual. Usaremos el mas corto)
const float azLengthB=59.2;              // Set 12. Lontigud B del actuador lineal (lado mas largo)
const float azAngleAtZero=90;            // Set 13. Angulo que forman A y B cuando el azimuth es 0 (el espejo esta perpendicular al sur, es decir, mirando al frente del heliostato)
const int azAcuteObtuse=2;               // Set 14. 1 - El angulo que forman A y B se vuelve mas agudo cuando movemos el azimuth positivamente (el espejo gira a la derecha visto desde atras). 2 - Caso contrario
const float azLimitAngle=-50;            // Set 15. Grados de azimuth alcanzados cuando la maquina pulsa el microinterruptor limite de azimuth (negativo mira a la izquierda visto desde atras, positivo a la derecha)

  // LIMITES DE LA MAQUINA
const float minAz=-45;                   // Set 16. Azimuth MIN
const float minAlt=-30;                  // Set 17. Altitud MIN
const float maxAz=45;                    // Set 18. Azimuth MAX
const float maxAlt=45;                   // Set 19. Altitud MAX

// CONFIGURACION GENERAL
const float latitude = 37.38715;         // Latitud, longitud y zona horaria donde esta ubicada la maquina
const float longitude = -6.08597;
const float timezone = 1;
const int useNorthAsZero = 0;            // 0 - Hemisferio Norte, 1 - Hemisferio Sur. UGO -> Lo podr´amos quitar, siempre vamos a estar en el hemisferio norte
const unsigned long updateEvery = 30;    // Frecuencia de actualizacion de la posicion en segundos
const float moveAwayFromLimit = 5;       // Numero de grados a alejarse del microinterruptor de fin de recorrido cuando se alcanza
const int hourReset = 17;                // Hora a la que el heliostato volvera a su posicion de inicio hasta el dia siguiente (usar hora sin tener en cuenta el horario de verano)
const int hourStart = 10;                 // Hora a la que empezar´ el heliostato a funcionar

// MOTORES
const float steps = 800;                 // Numero de pasos necesarios para que el motor de una vuelta completa
const float altSpeed=200;                // RPM de los motores en movimiento normal
const float azSpeed=200; 
const float altResetSpeed=200;           // RPM de los motores en movimiento de reset 
const float azResetSpeed=200;
const float altAccel = 2;                // Numero de segundos para que motor alcance su velocidad maxima
const float azAccel = 2;  
const long altitudeMax = 1000000;        // Numero de pasos que da el motor como maximo cuando se esta buscando la posicion de reset
const long azimuthMax  = 1000000;

// ASIGNACION CONEXIONES
const int azimuthStepPin = 2;            // Pin de paso y direccion
const int azimuthDirPin = 3;  
const int altitudeStepPin = 4;
const int altitudeDirPin = 5;
const int EnablePin = 6;                 // Pin de activacion de las controladores de motores
const int altLimitPin = 7;               // Pin de los microinterruptores de limite de recorrido
const int azLimitPin = 7; 
const int manualModeOnOffPin = 8;        // Pin de control manual

// MISCELANEA      
float pi = 3.14159265, SunsAltitude, SunsAzimuth, h, delta;
unsigned long updateTime = 0,  now = 0; 
float altMove, azMove;
float altLeftoverSteps; // UGO. ¿Y esto para que es?
float azLeftoverSteps;
float machineTargetAlt;
float machineTargetAz;
float machineCurrentAlt;
float machineCurrentAz;
char inData[20];

int machineRunningState=0;               // 0 - Esperando amanecer. 1 - Funcionando. 2 - Esperando cambio de dia
int machinePendingReset=0;               // 1 - La maquina esta esperando un reseteo

