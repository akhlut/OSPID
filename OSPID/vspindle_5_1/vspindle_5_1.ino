// TRIAC
int triac_control = 5; // Triac control - pin

//RPM control    
int analogPin = 0; // potentiometer or MACH3 - pin
int analogValue = 0;// value to store reading from pot.
//Power
int powerOn = 4; // Power switch - pin 
int powerIndicator = 1; // indicator GREEN LED -pin
// external On/Off from cnc controller
//Zero Detect    
//int zero_detect = 2; //Zero detect - pin
// when using values in the main routine and IRQ routine must be volatile value
volatile byte zero_bit = LOW; // declare IRQ flag
// HIGH = 1, LOW = 0

// HALL SENSOR 
//int  hallsensor = 3; //- pin
unsigned int rpmcount;
unsigned int rpm;
unsigned long timeold;
//unsigned long timenow;
//unsigned long timenow2;
//unsigned long timeohoho;



//LCD
//LiquidCrystal::LiquidCrystal(rs, enable, d0, d1, d2, d3)
#include <LiquidCrystal.h>
LiquidCrystal lcd(13, 11, 7, 8, 9, 10);
//#include <LiquidCrystalFast.h>
//LiquidCrystalFast lcd(13, 12, 11, 7, 8, 9, 10);


// PID
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Define the aggressive and conservative Tuning Parameters
double consKp=0.18, consKi=0.5, consKd=0;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
int kDev = 34; // rpm to analog - exp: max rpm = 22000 , analogmax = 1023 : rpm/analogmax = kDev = 21.5




void setup()  
{
//  Serial.begin(9600);
  //Triac control setup  
  pinMode(triac_control, OUTPUT);  
  digitalWrite(triac_control, 0); // LED off
  //Power switch  
  pinMode(powerIndicator, OUTPUT);
  digitalWrite(powerIndicator, 0); // LED off
  pinMode(powerOn, INPUT);
  digitalWrite(powerOn, 1); // pull up
  //Zero detect  
//  pinMode(zero_detect, INPUT);
  attachInterrupt(0, zero_fun, FALLING);  // interrupt 0 digital pin 2 connected ZeroCross circuit
  // Hall sensor  
//  pinMode(hallsensor, INPUT);
  attachInterrupt(1, rpm_fun, FALLING);  // interrupt 1 digital pin 3 connected hall sensor
  rpmcount = 0;
  rpm = 0;
  timeold = 0; 

  // LCD detect
  lcd.begin(16,2);               // initialize the lcd 
  lcd.home ();                   // go home
  lcd.print("Hello, ARDUINO ");  
  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print (" vSpindle-2.0   ");    
  delay(3000);
  lcd.clear();  

  //PID

  Input = rpm;
  Setpoint = analogValue;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(1500, 9999);  // limit are set to be delay in microseconds for triac fiering ( 8333 for 60hz, 10000 for 50hz line )9980 to limit max power. 
  myPID.SetSampleTime (1);                                 //  we realy dont need full speed





}

void loop() 
{
  //Power switch and indicator = ON/OFF  
  if (!digitalRead(powerOn)) 
  {
    digitalWrite(powerIndicator, 1);        
  }
  else 
  {    
    digitalWrite(powerIndicator, 0);
  }
  //RPM counter and show on LCD
  if (rpmcount >= 15)//Update RPM every 20 counts, increase this for better RPM resolution,
    //decrease for faster update
    
  {
    detachInterrupt (1);
    rpm = 60000*1000/(micros() - timeold)*rpmcount;

    attachInterrupt (1, rpm_fun , FALLING);
    Input = rpm;
    rpmcount = 0; //reset
    timeold  = micros(); //set time
//    timenow = micros();
     if (rpm/2 < 10000) 
    { 
      lcd.setCursor ( 0, 0 );
      lcd.print (" ");
      lcd.setCursor ( 1, 0 );
      lcd.print (rpm/2);
    }
    else
    {     
      lcd.setCursor ( 0, 0 );
      lcd.print (rpm/2);  
    }
//        timenow2 = micros();
//    timeohoho = timenow2 - timenow;
//    Serial.print (timeohoho); 
//    Serial.print ("a");  

    }
    
  //PID 
//  timenow = micros();
 int analogValue = analogRead(analogPin); // we are reading pot value
//    timenow2 = micros();
//    timeohoho = timenow2 - timenow;
//    Serial.print (timeohoho); 
//    Serial.print ("a");
//    Serial.print(analogValue);
//    Serial.print("A");
  
  
  if (analogValue*kDev <1200)
  {
    Setpoint = 0;
  }
  else
  {
    Setpoint    = analogValue*kDev;  // setpoint converted to rpm
  }
  myPID.Compute();
//    timenow2 = micros();
//    timeohoho = timenow2 - timenow;
//    Serial.print (timeohoho); 
//    Serial.print ("a");
//    Serial.print(analogValue);
//    Serial.print("A");



  //TRIAC delay control      
  if ((zero_bit == 1) && (digitalRead(powerOn)== 0)) // checking zero bit and power on/off from switch or cnc controller
  {
//    myPID.Compute();
    if (Setpoint >0) // ignore the Outputs lower then 1000, to low for start the motor
    {
      delayMicroseconds(10000-Output); // delay for triac fiering in reverse, we need smaler delay for more power
      digitalWrite(triac_control, 1); //triac on 
      delayMicroseconds(10);          // triac On propogation delay
      digitalWrite(triac_control, 0);  //triac off 
      zero_bit = 0; // clear flag           
    }   
  }
}

void zero_fun() // set bit
{
  zero_bit = 1;
  // if zero detect set bit == 1
} 

void rpm_fun() // rpm sensor interrupt
{
  rpmcount++;
  //Each rotation, this interrupt function is run twice
}

