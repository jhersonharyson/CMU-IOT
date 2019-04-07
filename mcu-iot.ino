#define BLYNK_PRINT Serial

#include <SPI.h>
//#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#define DHTPIN 23
#define DHTTYPE DHT22


/////////////////////////////////////////////////////////////////////

/************************Hardware Related Macros************************************/
#define         MQ_PIN                       (34)     //define which analog input channel you are going to use
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)  //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (60)    //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (5)     //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
/**********************Application Related Macros**********************************/
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)
/*****************************Globals***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms

/////////////////////////////////////////////////////////////////////





DHT_Unified dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WidgetRTC rtc;
uint32_t delayMS;





// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "1a2960a5e3a04f57a6473b15a4dd7f9b";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "INTELBRAS";
char pass[] = "32335096";


const char* server = "big-city-server.herokuapp.com/api/v1/ws";

BLYNK_WRITE(V3)
{
  Serial.println("WebHook data:");
  Serial.println(param.asStr());
}

BLYNK_WRITE(V1) {


  
//  metragemGas();
    String lpg   = String(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG));
    String co    = String(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO));
    String smoke = String(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE));
    Serial.print("gas ");
   Serial.println(analogRead(MQ_PIN));
   Serial.print("LPG:"); 
   Serial.print(lpg);
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CO (monoxide):"); 
   Serial.print(co);
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE:"); 
   Serial.print(smoke);
   Serial.print( "ppm" );
   Serial.print("\n\n");
  
  GpsParam gps(param);

  // Print 6 decimal places for Lat, Lon
  Serial.print("Lat: ");
  Serial.println(gps.getLat(), 7);

  Serial.print("Lon: ");
  Serial.println(gps.getLon(), 7);

  // Print 2 decimal places for Alt, Speed
  Serial.print("Altitute: ");
  Serial.println(gps.getAltitude(), 2);

  Serial.print("Speed: ");
  Serial.println(gps.getSpeed(), 2);

  // Delay between measurements.
  // Get temperature event and print its value.
  sensors_event_t eventTemp;
  sensors_event_t eventHumi;
  dht.temperature().getEvent(&eventTemp);
  if (isnan(eventTemp.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(eventTemp.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&eventHumi);
  if (isnan(eventHumi.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(eventHumi.relative_humidity);
    Serial.println(F("%"));
  }
  clockDisplay();
  Serial.println("Data sent!");
  Serial.println();
  // \n\t\t\"humidity\": "+String(eventHumi.relative_humidity)+",\n\t\t\"temperature\": "+String(eventTemp.temperature)+"\n\t
//  ",\n\t\t\"toxic_gases\": {
//            \n\t\t\t\"co\": "",
//            \n\t\t\t\"smoke\": "",
//            \n\t\t\t\"lta\": ""
//     \n\t\t}"
  String msg = "{\n\t\"sensor\":{\n\t\t\"humidity\": "+(isnan(eventHumi.relative_humidity)? "\"\"" : String(eventHumi.relative_humidity))+",\n\t\t\"temperature\": "+(isnan(eventTemp.temperature)? "\"\"" : String(eventTemp.temperature))+",\n\t\t\"toxic_gases\": { \n\t\t\t\"co\": "+String(co)+",\n\t\t\t\"smoke\":"+String(smoke)+", \n\t\t\t\"lpg\": "+String(lpg)+" \n\t\t}\n\t},\n\t\"lat\": "+String(param[0].asString())+",\n\t\"log\": "+String(param[1].asString())+"\n}\n\n\n";
  Serial.println(msg);
  Blynk.virtualWrite(V9, msg);
  Blynk.virtualWrite(V10, 1);
  Blynk.virtualWrite(V3, msg);
  
//  Blynk.virtualWrite(V10, "http://big-city-server.herokuapp.com/api/v1/ws/iot");

}

void setup()
{
  Serial.begin(9600);
  Serial.println("Inicializando...");
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));


  Serial.print("Calibrating...\n");                                           //prints "calibrating..."
  Serial.print("This will take approx ");                                     //prints "this will take approx "
  Serial.print(CALIBARAION_SAMPLE_TIMES * CALIBRATION_SAMPLE_INTERVAL / 1000);//prints "# of seconds" NUMSAMPLES X SAMPLEINTERVAL / 1000
  Serial.print(" seconds \n");                                                //prints "seconds"
  Ro = MQCalibration(MQ_PIN);                                                 //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                                              //when you perform the calibration                    
  Serial.print("Calibration is done...\n");                                   //prints "calibration is done"
  Serial.print("\n");                                                         //prints a blank line
  Serial.print("Sensor Resistance (R=");                                     //prints "Sensor Resistance (Ro="
  Serial.print(Ro);                                                           //Prints value of sensor resistance
  Serial.print("kohm)\n");                                                    //prints "Kohm)"                                           
  Serial.print("\n");
  
  pinMode(0, INPUT);
  
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  //  setSyncInterval(900000);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
}

void loop()
{
  Blynk.run();
  timer.run();
}

void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin(); 
}

//void metragemGas(){
// 
//   Serial.print("gas ");
//   Serial.println(analogRead(MQ_PIN)/Ro);
//   Serial.print("LPG:"); 
//   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
//   Serial.print( "ppm" );
//   Serial.print("    ");   
//   Serial.print("CO (monoxide):"); 
//   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
//   Serial.print( "ppm" );
//   Serial.print("    ");   
//   Serial.print("SMOKE:"); 
//   Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
//   Serial.print( "ppm" );
//   Serial.print("\n\n");
//  
//}

//String toString(int tipo, int * arguments){
//  switch(tipo){
//  case 0:{
//     String * formatted =  ",\n\t\t\"toxic_gases\": { \n\t\t\t\"co\": "+String(arguments[0])+",\n\t\t\t\"smoke\":"+String(arguments[1])+", \n\t\t\t\"lta\": "+String(arguments[2])+" \n\t\t}";
//    return formatted;
//    }
//  }
//}


/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  int divisor = raw_adc;
  if(divisor <= 1) divisor = 1;
  return ( ((float)RL_VALUE*(4095-raw_adc)/divisor));
}
/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  double val=0;
 
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    
    val += MQResistanceCalculation(analogRead(mq_pin));
    Serial.print("--val> ");                                                      //according to the chart in the datasheet 
  Serial.println(val);
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  Serial.print("--antes> ");                                                      //according to the chart in the datasheet 
  Serial.println(val);
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
 Serial.print("--clean> ");                                                      //according to the chart in the datasheet 
  Serial.println(val);
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
  Serial.print("--> ");                                                      //according to the chart in the datasheet 
  Serial.println(val);
  return val; 
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which 
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/ 

int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
  return 0;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 

int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

