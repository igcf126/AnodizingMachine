#include <math.h>
#include <Servo.h>
#include <dht11.h>
#include <TimerOne.h>

int tiempo_ant = 0;
int Estado = 0;
int Estado_e = 0;
int Estado_ult = 0;
bool homed_y = false;
bool homed_z = false;
bool downed_z = false;
int tiempo_step = 2;
bool x = false;
long y = 0;
long z = 0;
bool cmovio = false;

// Variables de tiempo
unsigned long currentTime = millis();
unsigned long tiempo_rest = 0;

// Boton de emergencia 
#define EM_Button 19
#define EM_Puerta 21
int EM_Button_State = HIGH;
int EM_Puerta_State = HIGH;
bool e_stop = false;

// funciones mover brazo
int pasos = 0;
int pasos_z = 0;
int i = 0;
int test = 0;

// Steppers y sus limit switches
#define lms_y 33
#define lms_z 32
#define lms_z_down 30
#define Step_y 2
#define Dir_y 5
#define Step_z 3
#define Dir_z 6
#define servos 8
bool h = false;

// Stepper conveyor
#define Step_c 4
#define Dir_c 7

// limit switches de etapas
#define lms_c 31

// variables de sistema cartesiano
int lms_yState = LOW;
int lms_zState = LOW;
int lms_zDownState = LOW;
int lms_cState = LOW;
int StepYState = LOW;
int StepZState = LOW;
unsigned long pos_y = 0;
unsigned long pos_z = 0;
unsigned long k = 0;
bool bajo = false;

// variables de relay
#define k1 37
#define k2 35

// CONTROL DE TEMPERATURA
#define RELAY_PIN 53//COMO FUE chequear pin de relay
#define DHT11PIN 1
#define TARGET_TEMP 42.0
#define TARGET_RANGE 2.0

dht11 DHT11;
double temperature = 0.0;

// PID parameters
double Kp = 1.0;
double Ki = 0.0;
double Kd = 0.0;
double integral = 0.0;
double prevError = 0.0;
bool targetReached = false;



// Variable PWM
#define PWM 44
int Amp = 0;

// posiciones de envases
int P1 = 70; //estacion de carga //hab[ia un 70, voy a probar un 0]
int P2 = 2460; //estacion de anodizado
int P3 = 4550; //estacion de limpieza
int P4 = 6680; //estacion de sellado
int P5 = 7600; //estacion de entrega
int Cbase = 1450; //soportes de canastas
int P5Z = 300; //altura entrega final

// calculos pieza
String area;
float area_mm = 0.0;
float area_cm = 0.0;
float grosor = 0.1;
int densidad_corriente = 10;

// calculos anodizados
float tiempo_anod = 0.0;
float tiempo_sealed = 0.0;
float Amp_anod = 0.0;
float Volt_Anod = 0.0;

// definir servo llamado Grippers
Servo Grippers;

String msg;
float voltCalc(){ //Calculo voltage
  float d = densidad_corriente * 2.5;
  return d;
}
float sealTimeCalc(){ //Calculo tiempo sellado
  float c = grosor * 30;
  return c;
}
float timeCalcAnod(){ //Calculo tiempo anodizado
  float b = (720/densidad_corriente)*grosor;
  return b;
}
float ampCalc(int x){ //Calculo corriente anodizado
  float a = ((area_cm/6.45)/144) * densidad_corriente;
  return a;
  
}

String test_string;


//////////////////////////////////////// SETUP /////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  Grippers.attach(servos);

  pinMode(EM_Button, INPUT);
  pinMode(EM_Puerta, INPUT_PULLUP);
   
   
  pinMode(lms_y, INPUT);
  pinMode(lms_z, INPUT);
  pinMode(lms_z_down, INPUT_PULLUP);

  pinMode(Step_y, OUTPUT);
  pinMode(Dir_y, OUTPUT);
  pinMode(Step_z, OUTPUT);
  pinMode(Dir_z, OUTPUT);
  pinMode(Step_c, OUTPUT);
  pinMode(Dir_c, OUTPUT);

  pinMode(k1, OUTPUT);
  pinMode(k2, OUTPUT);
  
  digitalWrite(k1, HIGH);
  digitalWrite(k2,HIGH);
  pinMode(PWM, OUTPUT);

  Grippers.write(32);

  // inicialiación de control de temp
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  
  delay(1000);

  digitalWrite(RELAY_PIN, LOW);
  // Configure Timer1 to trigger the temperature measurement every second
  // Timer1.initialize(1000000); // 1 second = 1,000,000 microseconds
  // Timer1.attachInterrupt(measureTemperature);

}
//////////////////////////////////////// LOOP /////////////////////////////////////////////////////
void loop() {
  //////////////////////////////////////// TEST /////////////////////////////////////////////////////
  // if (Serial.available() > 0) {
  //   test_string = Serial.readString();
  //   test = test_string.toInt();
  //   Mover_brazo(test);
  //   return;
  // }

  ////////////////////////////////////// FUNCIONAMIENTO /////////////////////////////////////////////////////
  if(e_stop == true){
    Serial.println("aqui");
    if (Estado != 14){
      e_stop = false;
      Serial.println(Estado);      
      Estado_ult = Estado; 
      Estado = 14;
      Serial.println(Estado); 
      Serial.println(Estado_ult);      
      return; 
    }
    else if (Estado == 14){
      e_stop = false;
      Serial.println(Estado); 
      Serial.println(Estado_ult);       
      return;
    }
    return;
  }
  
  else {
    switch (Estado) {
      case 0: /////////////////////////////////////////////// CASE 0 ///////////////////////////////////////////////////////////////// P1 BUSCAR CERO Y 
        // Serial.println(Estado);
        homed_z = false;
        homing_z();
        Estado = 1;

      break; 

      case 1: /////////////////////////////////////////////// CASE 1 ///////////////////////////////////////////////////////////////// P1 BUSCAR CERO Z 
        // Serial.println(Estado);
        homed_y = false;
        homing_y();
        Estado = 2;
        msg = String("1,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",10");
        Serial.println(msg);

      break; 

      case 2: /////////////////////////////////////////////// CASE 2 ///////////////////////////////////////////////////////////////// P1 RECIBIR DATOS Y CALCULOS
        EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);
        if(EM_Button_State == LOW || EM_Puerta_State == LOW ){Emergencia();return;}
        msg = String("1,0,70,0,0,10");
        Serial.println(msg);
      
        if (Serial.available()) 
        {  
          area = Serial.readString();
          area_mm = area.toInt();
          area_cm = area_mm * 0.01;
          tiempo_sealed = sealTimeCalc(); 
          tiempo_anod = timeCalcAnod();
          Amp_anod = ampCalc(area_cm);
          
          Serial.println(area_cm);          
          Serial.println(tiempo_sealed);          
          Serial.println(tiempo_anod);          
          Serial.println(Amp_anod);    

          tiempo_anod *= 60000;
          tiempo_sealed *= 60000;
          Serial.println(tiempo_anod);

          // while(!targetReached){ POR SI SE RESUELVE EL CONTROL DE TEMEPRATURA
          //   // como un mojón esperando
          // }
          Estado = 3;            
       
         }          
      break;

      case 3:  /////////////////////////////////////////////// CASE 3 ///////////////////////////////////////////////////////////////// P1 Activar conveyor para depositar pieza en canasta
        
        // Serial.println(Estado);
        check();
    
        digitalWrite(Dir_c, HIGH);                                                       
        // Serial.println("2, %d, 70, %d, %d, 100", tiempo_anod, Amp_anod, Volt_Anod);
        for (int i = 0; i < 4000; i++)
        {
          digitalWrite(Step_c, HIGH);
          delay(tiempo_step);
          digitalWrite(Step_c, LOW);
          delay(tiempo_step);
        }
        Grippers.write(32); // abriendo gripper 
        Mover_brazo(P1);

        // Mover_brazo_z(Cbase);
        downed_z = false;
        baja_Z();

        Estado = 4;
        msg = String("2,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",30");
        Serial.println(msg);

      break; 

      case 4: /////////////////////////////////////////////// CASE 4 ///////////////////////////////////////////////////////////////// BRAZO Recoger canasta de P1 a P2
        // Serial.println(Estado);
        Grippers.write(1);   
        delay(200);
        
        //Mover_brazo_z(0);
        homed_z = false;
        homing_z();

        Mover_brazo(P2);

        // Mover_brazo_z(Cbase);
        downed_z = false;
        baja_Z();

        Estado = 5;
      break; 
      
      case 5: /////////////////////////////////////////////// CASE 5 ///////////////////////////////////////////////////////////////// P2 Colocar pieza en P2 y dejar anodizar     
        // Serial.println(Estado);
        Grippers.write(32);      
        
        //Mover_brazo_z(0);
        homed_z = false;
        homing_z();

        Mover_brazo(P1);
        currentTime = millis();
        tiempo_anod += currentTime;
        tiempo_anod = 5000;
        Corriente_out(Amp_anod);
        digitalWrite(k1, LOW);
        Estado = 6;
        // Serial.println(Estado);
      break; 

      case 6: /////////////////////////////////////////////// CASE 6 ///////////////////////////////////////////////////////////////// P2 Dejar anodiza
        analogWrite(PWM, 255);
        EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
          return;
        }
        
        tiempo_rest = (tiempo_anod - currentTime)/60000;
        msg = String("3,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",50");
        Serial.println(msg);
        currentTime = millis();
        if (currentTime >= tiempo_anod) {
          Estado = 7; 
          tiempo_anod = 0;
          analogWrite(PWM, 0);   
        }

      break; 

      case 7: /////////////////////////////////////////////// CASE 7 ///////////////////////////////////////////////////////////////// P2 Recoger pieza de P2 y mover a P3 
        // Serial.println(Estado);
        digitalWrite(k1, HIGH);
        Mover_brazo(P2);

        // Mover_brazo_z(Cbase);
        downed_z = false;
        baja_Z();
        
        Estado = 8;
        msg = String("4,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",60");
        Serial.println(msg);

      break;      
      
      case 8: /////////////////////////////////////////////// CASE 8 ///////////////////////////////////////////////////////////////// BRAZO Bajar a agua y mover pieza de P3 y mover a P4
        // Serial.println(Estado);
        Grippers.write(1);
        
        delay(200);
        
        //Mover_brazo_z(0);
        homed_z = false;
        homing_z();

        Mover_brazo(P3);
        // Serial.println("4, %d, 70, %d, %d, 100", tiempo_anod, Amp_anod, Volt_Anod);
        Estado = 9;
        

      break; 
      
      case 9: /////////////////////////////////////////////// CASE 9 ///////////////////////////////////////////////////////////////// BRAZO
        // Serial.println(Estado);
        // Mover_brazo_z(Cbase);
        downed_z = false;
        baja_Z();

        digitalWrite(k2, LOW);
        delay(10000);
        digitalWrite(k2, HIGH);
        
        //Mover_brazo_z(0);
        homed_z = false;
        homing_z();

        Mover_brazo(P4);
        // Serial.println("4, %d, 70, %d, %d, 100", tiempo_anod, Amp_anod, Volt_Anod);
        // Mover_brazo_z(Cbase);
        downed_z = false;
        baja_Z();

        Estado = 10;       
      break; 

      case 10: /////////////////////////////////////////////// CASE 10 ///////////////////////////////////////////////////////////////// P4
        // Serial.println(Estado); 
        Grippers.write(32);
        
        //Mover_brazo_z(0);
        homed_z = false;
        homing_z();

        Mover_brazo(P3);
        currentTime = millis();
        tiempo_sealed += currentTime;
        tiempo_sealed = 5000;
        Estado = 11;
      break; 

      case 11: /////////////////////////////////////////////// CASE 11 ///////////////////////////////////////////////////////////////// P4
        EM_Button_State = digitalRead(EM_Button);

        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
          return;
          }

        currentTime = millis();
        if (currentTime >= tiempo_sealed) {
        Estado = 0;
        }
      break; 

      // case 12: /////////////////////////////////////////////// CASE 12 ///////////////////////////////////////////////////////////////// P4
      //   // Serial.println(Estado);
      //   msg = String("5,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",85");
      //   Serial.println(msg);
      //   Mover_brazo(P4);

      //   // Mover_brazo_z(Cbase);
      //   downed_z = false;
      //   baja_Z();

      //   Estado = 14;
        
        // break;

      // case 13: /////////////////////////////////////////////// CASE 13 ///////////////////////////////////////////////////////////////// BRAZO
      //   // Serial.println(Estado);
      //   Grippers.write(1);
        
      //   //Mover_brazo_z(0);
      //   homed_z = false;
      //   homing_z();

      //   delay(200);
      //   Mover_brazo(P5);
      //   Mover_brazo_z(P5Z);
      //   Estado = 14;
      //   // Serial.println("5, %d, 70, %d, %d, 100", tiempo_anod, Amp_anod, Volt_Anod);
      // break;

      // case 14: /////////////////////////////////////////////// CASE 14 ///////////////////////////////////////////////////////////////// P5
      //   // Serial.println(Estado);
      //   Grippers.write(32);
        
      //   //Mover_brazo_z(0);
      //   homed_z = false;
      //   homing_z();

      //   Estado = 0;
      //   msg = String("1,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",100");
      //   Serial.println(msg);

      // break;

      case 15: //////////////////////////////////////// EMERGENCIA /////////////////////////////////////////////////////
        msg = String("0,")+String(tiempo_anod)+ String(",77,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",0");
        Serial.println(msg);
        
        if (Estado_ult == 4 || Estado_ult == 8 || Estado_ult == 9 || Estado_ult == 13){ // EN BRAZO
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Mover_brazo(P5);
          Mover_brazo_z(P5Z);
          Grippers.write(32);
          Estado = 0;
          return;
        }
        else if (Estado_ult == 6 || Estado_ult == 7 || Estado_ult == 5){ // EN P2
          Grippers.write(32);
          Mover_brazo(P2);

          // Mover_brazo_z(Cbase);
          downed_z = false;
          baja_Z();

          Grippers.write(0);
          Estado_ult = 16;
          Mover_brazo(P3);

          // Mover_brazo_z(Cbase);
          downed_z = false;
          baja_Z();

          delay(10000);
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Estado_ult = 4;
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Mover_brazo(P5);
          Mover_brazo_z(P5Z);
          Grippers.write(32);
          Estado = 0;
          return;
        }
        else if (Estado_ult == 10 || Estado_ult == 11 || Estado_ult == 12){ // EN P4
          Grippers.write(32);
          Mover_brazo(P4);

          // Mover_brazo_z(Cbase);
          downed_z = false;
          baja_Z();

          Grippers.write(0);
          Estado = 5;
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Mover_brazo(P5);
          Mover_brazo_z(P5Z);
          Grippers.write(32);
          Estado = 0;
          return;
        }
        else if (Estado_ult == 0 || Estado_ult == 1 || Estado_ult == 2){
          Estado = 0;
        } 
        else if (Estado_ult == 14){ //EN P1 O P5
          Estado = Estado_ult;
          return;
        }
        else if (Estado_ult == 3)
        {
          Estado = 0;
          return;
        }
        
        else if (Estado_ult == 16){
          Mover_brazo(P3);

          // Mover_brazo_z(Cbase);
          downed_z = false;
          baja_Z();

          delay(10000);
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Estado_ult = 4;
        
          //Mover_brazo_z(0);
          homed_z = false;
          homing_z();

          Mover_brazo(P5);
          Mover_brazo_z(P5Z);
          Grippers.write(32);
          Estado = 0;
        }
      break;
    }
  }
}

//////////////////////////////////////// FUNCIONES DEL BRAZO /////////////////////////////////////////////////////
/*Estas vendrian siendo todas las funciones generales de la maquina para hacer que el codigo en la parte de maquina
de estado sea mas facil de escribir yu entender, estos movimientos son los mas basicos y generales.*/
int Mover_brazo(int k){  
  if (pos_y > k){
    
    EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
    
    pasos = pos_y - k; // Calcular la cantidad de pasos a dar
    digitalWrite(Dir_y, HIGH); // Moverse a la derecha
    for ( i = 0; i < pasos; i++){
      
      EM_Button_State = digitalRead(EM_Button);
      EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
        Emergencia();
        return;}
      
      digitalWrite(Step_y, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_y, LOW);
      delay(tiempo_step);
      pos_y--;
    }
    cmovio = true;
  // Serial.println(pos_y);    
  return;
  }
  else if (pos_y < k){
    
    EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
        Emergencia();
        return;}
    
    pasos = k - pos_y; // Calcular la cantidad de pasos a dar
    digitalWrite(Dir_y, LOW);  // Moverse a la izquierda
    for ( i = 0; i < pasos; i++){
      
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
      // Serial.println("aqui");   
      digitalWrite(Step_y, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_y, LOW);
      delay(tiempo_step);
      pos_y++;
    }
    cmovio = true;
    return;
  }
  return;
}

int Mover_brazo_z(int k){  
  if (pos_z > k){
    
      EM_Button_State = digitalRead(EM_Button);
      EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
        Emergencia();
        return;}
      
    // Serial.println(k);
    pasos = pos_z - k; // Calcular la cantidad de pasos a dar
    // Serial.println(pasos);
    // Serial.println("para arriba");
    digitalWrite(Dir_z, LOW); //Moviendo brazo hacia arriba
    for ( i = 0; i < pasos; i++){
      
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){Emergencia();
        return;}
      
      digitalWrite(Step_z, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_z, LOW);
      delay(tiempo_step);
      pos_z --;
    }  
  }
  else if (pos_z < k){
    
      EM_Button_State = digitalRead(EM_Button);
              EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
    pasos = k - pos_z; // Calcular la cantidad de pasos a dar
    digitalWrite(Dir_z, HIGH); //Moviendo brazo hacia abajo
    // Serial.println("para abajo");
    // Serial.println(pasos);
    for ( i = 0; i < pasos; i++){
      
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
      digitalWrite(Step_z, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_z, LOW);
      delay(tiempo_step);
      pos_z ++;
    }  
  }
}

//////////////////////////////////////// HOMING Y /////////////////////////////////////////////////////
void homing_y(){
    while(homed_y == false){
    lms_yState = digitalRead(lms_y);
    // Serial.print(lms_yState);
    // Serial.println();
    
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
    if (lms_yState == HIGH && (homed_y == false)) {

      digitalWrite(Dir_y, LOW); // Go left 
      // Serial.print("moving left");
      // Serial.println();

      for (int i = 0; i < 100; i++) { //make a full rotation to the left (100 steps)
      
        EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
        digitalWrite(Step_y, HIGH);
        delay(tiempo_step);
        digitalWrite(Step_y, LOW);
        delay(tiempo_step);
      } 
      lms_yState = digitalRead(lms_y);
    } 
  
    else if (lms_yState == LOW && (homed_y == false)) {
      digitalWrite(Dir_y, HIGH); //Go right
    
      // Serial.print("moving right"); //To confirm direction
      // Serial.println();
      
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
      digitalWrite(Step_y, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_y, LOW);
      delay(tiempo_step);

      y++; //agregando los pasos tomados 
    
      lms_yState = digitalRead(lms_y);  
       
      if (lms_yState == HIGH) { 
        homed_y = true;
        pos_y = 0;
      // Serial.print("Dio "); Serial.print(y); Serial.print(" pasos en y"); // Para saber los pasos para una translasion completa (comentar para codigo final)
      }
    }
  }
}

//////////////////////////////////////// HOMING Z /////////////////////////////////////////////////////
void homing_z(){ 
  while(homed_z == false){
    lms_zState = digitalRead(lms_z);
    
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
    if (lms_zState == HIGH && (homed_z == false)) {
      digitalWrite(Dir_z, HIGH); // Go counterclockwise 

      // Serial.print("moving down");
      // Serial.println();

      for (i = 0; i < 100; i++) { //make a full rotation to the left (200 steps)
        
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
        
        digitalWrite(Step_z, HIGH);
        delay(tiempo_step);
        digitalWrite(Step_z, LOW);
        delay(tiempo_step);
      } 
      lms_zState = digitalRead(lms_z);
    } 
  
    else if (lms_zState == LOW && (homed_z == false)) {
      digitalWrite(Dir_z, LOW); //Go clockwise
      
      EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if(EM_Button_State == LOW || EM_Puerta_State == LOW){
          Emergencia();
        return;}
      
      digitalWrite(Step_z, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_z, LOW);
      delay(tiempo_step);
      
      lms_zState = digitalRead(lms_z);  
      
      if (lms_zState == HIGH) {
      homed_z = true;
      pos_z = 0;
      }
    }
  }
}

void baja_Z() {
  while (!downed_z) {
    lms_zDownState = digitalRead(lms_z_down);

    EM_Button_State = digitalRead(EM_Button);
    EM_Puerta_State = digitalRead(EM_Puerta);

    if (EM_Button_State == LOW || EM_Puerta_State == LOW) {
      Emergencia();
      return;
    }

    if (lms_zDownState == HIGH && !downed_z) {
      digitalWrite(Dir_z, LOW); // Go clockwise

      for (i = 0; i < 100; i++) { // Make a full rotation to the right (200 steps)
        EM_Button_State = digitalRead(EM_Button);
        EM_Puerta_State = digitalRead(EM_Puerta);

        if (EM_Button_State == LOW || EM_Puerta_State == LOW) {
          Emergencia();
          return;
        }

        digitalWrite(Step_z, HIGH);
        delay(tiempo_step);
        digitalWrite(Step_z, LOW);
        delay(tiempo_step);
      }
      lms_zDownState = digitalRead(lms_z_down);
    } else if (lms_zDownState == LOW && !downed_z) {
      digitalWrite(Dir_z, HIGH); // Go counterclockwise

      EM_Button_State = digitalRead(EM_Button);
      EM_Puerta_State = digitalRead(EM_Puerta);

      if (EM_Button_State == LOW || EM_Puerta_State == LOW) {
        Emergencia();
        return;
      }

      digitalWrite(Step_z, HIGH);
      delay(tiempo_step);
      digitalWrite(Step_z, LOW);
      delay(tiempo_step);

      lms_zDownState = digitalRead(lms_z_down);

      if (lms_zDownState == HIGH) {
        downed_z = true;
        pos_z = Cbase;
      }
    }
  }
}


//////////////////////////////////////// HOMING Z /////////////////////////////////////////////////////
int Corriente_out(int b){
  Amp = map(b, 0, 1.51, 0, 255);
  analogWrite(PWM, Amp);
}

/////////////////////////////////////// EMERGENCIA /////////////////////////////////////////////////////
void Emergencia() {
  msg = String("0,")+String(tiempo_anod)+ String(",70,") + String(Amp_anod) + String(",") + String(Volt_Anod), String(",0");
  Serial.println(msg);
  Corriente_out(0);
  digitalWrite(k1, HIGH);
  digitalWrite(k1, HIGH);
  // e_stop = true;
  // Serial.println("bobop");
  // Serial.print("Wee");
  Estado = 15;
  while(EM_Button_State == LOW || EM_Puerta_State == LOW){
    EM_Button_State = digitalRead(EM_Button);
    EM_Puerta_State = digitalRead(EM_Puerta);
  // Serial.print("e");
  }   
  return;
}

/////////////////////////////////////// CHECK CANASTA /////////////////////////////////////////////////////
void check(){
  lms_cState = digitalRead(lms_c);
  if(lms_cState == LOW){
    // Serial.println("0, %d, 70, %d, %d, 1", tiempo_anod, Amp_anod, Volt_Anod);
    Emergencia();
    return;
  }
}

// /////////////////////////////////////// CHECK TEMPERATURE /////////////////////////////////////////////////////

// void measureTemperature()
// {
//   int chk = DHT11.read(DHT11PIN);

//   if (chk == 0)
//   {
//     temperature = (double)DHT11.temperature;

//     Serial.print("Temperature (C): ");
//     Serial.println(temperature, 2);

//     Serial.print("Humidity (%): ");
//     Serial.println((double)DHT11.humidity, 2);

//     double error = TARGET_TEMP - temperature;
//     integral += error;
//     double derivative = error - prevError;
//     prevError = error;

//     // PID control output calculation
//     double output = (Kp * error) + (Ki * integral) + (Kd * derivative);

//     if (output >= 0.0)
//     {
//       digitalWrite(RELAY_PIN, HIGH);
//       Serial.println("Relay ON");
//     }
//     else
//     {
//       digitalWrite(RELAY_PIN, LOW);
//       Serial.println("Relay OFF");
//     }
//   }
//   else
//   {
//     Serial.println("Error reading from DHT11 sensor");
//   }

//   if (temperature >= (TARGET_TEMP - TARGET_RANGE) && temperature <= (TARGET_TEMP + TARGET_RANGE)) {
//    // Target temperature is within the range
//      targetReached = true;
//   //   Serial.println("Relay ON");
//   }

// }


//////////////////////////////////////// NOTAS /////////////////////////////////////////////////////
/* 
NOTA 1: Para el programa poder moverse con posiciones el programa debera de ser capaz de guardar la 
posicion actual, y restar esta a la posicion a la que quiero que se mueva para conocer la distacia 
que separa a estas. Por ejemplo si quiero moverme de la posicion 1800 a la 2800 tenemos que 1800 es 
menor que 2800 por ende el sabe que tiene que moverse hacia el frente, en caso de que fuera al reves
y que quisiera moverme de 2800 a 1800 al hacer la relacion el veria que 1800 es menor que 2800 por 
ende se moveria hacia atras, reduciendo la distancia hasta llegar alla.

Nota 2: Posicion de canastas y Estados
Estado	  Descripción	                                                                                             Posición de canasta
0	        Homing de z	                                                                                             P1
1	        Homing de y	                                                                                             P1
2	        Esperar mensaje de computador por área	                                                                 P1
3	        Mover pieza a canasta desde le conveyor	                                                                 P1
4	        Mover canasta de P1 sobre P2 y bajarlo                                                                   brazo
5	        Soltar canasta en P2 y retirar brazo a P1	                                                               P2
6	        Esperar tiempo de anodizado	                                                                             P2
7	        Mover brazo a P2, y bajar brazo	                                                                         P2
8	        Remover canasta de P2 y levantar sobre P3 esperar 10 segundos y mover levantar sobre P4	                 Brazo
9	        Soltar canasta en P4 y mover brazo a z0	                                                                 P4
10	      Esperar tiempo de sellado	                                                                               P4
11	      Bajar brazo a P4	                                                                                       P4
12	      Mover canasta de P4 a P5	                                                                               Brazo
13	      Soltar canasta y volver a Estado 1	                                                                     P5

Nota 3: Emergencia
•	Si la canasta se encuentra en un estado donde la canasta está en un envase entonces el 
  brazo va a la posición de la canasta y la lleva a la posición inicial y cuando la deja hace homing.
•	Si la canasta esta en el brazo esta simplemente se la lleva a la posición inicial y cuando la deja 
  hace homing.
•	Si el estado de la maquina era el estado 1, 2, 3 o 4 este no hace nada y devuelve a la maquina al 
  estado que estaba.	
*/