#include<SoftwareSerial.h>

SoftwareSerial Serial1(2,3); //make RX arduino line is pin 2, make TX arduino line is pin 3.

SoftwareSerial gps(10,11);


#include<Wire.h>
#define Dig_pin 7
int Ana_out = 0;
 
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
 
#define minValt1 300
#define maxValt1 425
#define minValt2 20
#define maxValt2 375
#define minValt3 100
#define maxValt3 260


 
double x;
double y;
double z;



int i=0,k=0;

int  gps_status=0;

float latitude=0; 

float logitude=0;                       

String Speed="";

String gpsString="";

char *test=(char*)"$GPRMC";


void initModule(String cmd, char *res, int t)

{

  while(1)

  {

    Serial.println(cmd);

    Serial1.println(cmd);

    delay(100);

    while(Serial1.available()>0)

    {

       if(Serial1.find(res))

       {

        Serial.println(res);

        delay(t);

        return;

       }


       else

       {

        Serial.println("Error");

       }

    }

    delay(t);

  }

}



void setup() 

{

  Wire.begin();
Wire.beginTransmission(MPU_addr);
Wire.write(0x6B);
Wire.write(0);
Wire.endTransmission(true);
Serial.begin(9600);

  Serial1.begin(9600);

  Serial.begin(9600);

  

  

  Serial.println("Initializing....");

  initModule("AT",(char*)"OK",1000);

  initModule("ATE1",(char*)"OK",1000);

  initModule("AT+CPIN?",(char*)"READY",1000);  

  initModule("AT+CMGF=1",(char*)"OK",1000);     

  initModule("AT+CNMI=2,2,0,0,0",(char*)"OK",1000);  

  Serial.println("Initialized Successfully");

 
  gps.begin(9600);

  get_gps();

  show_coordinate();

  delay(2000);



  Serial.println("System Ready..");

}


void loop() 

{
  Wire.beginTransmission(MPU_addr);
Wire.write(0x3B);
Wire.endTransmission(false);
Wire.requestFrom(MPU_addr,14,true);
AcX=Wire.read()<<8|Wire.read();
AcY=Wire.read()<<8|Wire.read();
AcZ=Wire.read()<<8|Wire.read();
int xAng = map(AcX,minValt1,maxValt1,-90,90);
int yAng = map(AcY,minValt2,maxValt2,-90,90);
int zAng = map(AcZ,minValt3,maxValt3,-90,90);
 
x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
 
Serial.print("AngleX= ");
Serial.println(x);
 
Serial.print("AngleY= ");
Serial.println(y);
 
Serial.print("AngleZ= ");
Serial.println(z);
Serial.println("-----------------------------------------");
delay(10000);
   Ana_out = analogRead(A0);
   Serial.println("Anaolog : ");
   Serial.println(Ana_out);
   delay(2000);
   if(Ana_out<150 && x < minValt1 || x > maxValt1  || y < minValt2 || y > maxValt2  || z < minValt3 || z > maxValt3)

   {
    
     
   Ana_out = analogRead(A0);
   Serial.println("Anaolog : ");
   Serial.println(Ana_out);
   delay(2000);
      get_gps();

      show_coordinate();

      

      Serial.println("Sending SMS");

      Send();

      Serial.println("SMS Sent");

      delay(2000);

    

          
   }
   

    if(x < minValt1 || x > maxValt1  || y < minValt2 || y > maxValt2  || z < minValt3 || z > maxValt3)

    {
     
   
      get_gps();

      show_coordinate();

    

      Serial.println("Sending SMS");

      Send();

      Serial.println("SMS Sent");

      delay(2000);

      

    }       

}


void gpsEvent()

{

  gpsString="";

  while(1)

  {

   while (gps.available()>0)            //Serial incoming data from GPS

   {

    char inChar = (char)gps.read();

     gpsString+= inChar;                    //store incoming data from GPS to temparary string str[]

     i++;

    // Serial.print(inChar);

     if (i < 7)                      

     {

      if(gpsString[i-1] != test[i-1])         //check for right string

      {

        i=0;

        gpsString="";

      }

     }

    if(inChar=='\r')

    {

     if(i>60)

     {

       gps_status=1;

       break;

     }

     else

     {

       i=0;

     }

    }

  }

   if(gps_status)

    break;

  }

}


void get_gps()

{

  

   gps_status=0;

   int x=0;

   while(gps_status==0)

   {

    gpsEvent();

    int str_lenth=i;

    coordinate2dec();

    i=0;x=0;

    str_lenth=0;

   }

}


void show_coordinate()

{
  Serial.print("ACCIDENT DETECTED\n");
  

    Serial.print("Latitude:");

    Serial.println(latitude);

    Serial.print("Longitude:");

    Serial.println(logitude);

    Serial.print("Speed(in knots)=");

    Serial.println(Speed);

    delay(2000);

   

}


void coordinate2dec()

{

  String lat_degree="";

    for(i=20;i<=21;i++)         

      lat_degree+=gpsString[i];

      

  String lat_minut="";

     for(i=22;i<=28;i++)         

      lat_minut+=gpsString[i];


  String log_degree="";

    for(i=32;i<=34;i++)

      log_degree+=gpsString[i];


  String log_minut="";

    for(i=35;i<=41;i++)

      log_minut+=gpsString[i];

    

    Speed="";

    for(i=45;i<48;i++)          //extract longitude from string

      Speed+=gpsString[i];

      

     float minut= lat_minut.toFloat();

     minut=minut/60;

     float degree=lat_degree.toFloat();

     latitude=degree+minut;

     

     minut= log_minut.toFloat();

     minut=minut/60;

     degree=log_degree.toFloat();

     logitude=degree+minut;

}


void Send()

{ 

   Serial1.println("AT");

   delay(500);

   serialPrint();

   Serial1.println("AT+CMGF=1");

   delay(500);

   serialPrint();

   Serial1.print("AT+CMGS=");

   Serial1.print('"');

   Serial1.print("9359243907");    //mobile no. for SMS alert

   Serial1.println('"');

   delay(500);

   serialPrint();
   Serial1.print("ACCIDENT DETECTED\n");
   Serial1.print("Latitude:");

   Serial1.println(latitude);

   delay(500);

   serialPrint();

   Serial1.print(" longitude:");

   Serial1.println(logitude);

   delay(500);

   serialPrint();

   Serial1.print(" Speed:");

   Serial1.print(Speed);

   Serial1.println("Knots");

   delay(500);

   serialPrint();

   Serial1.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");

   Serial1.print(latitude,6);

   Serial1.print("+");              //28.612953, 77.231545   //28.612953,77.2293563

   Serial1.print(logitude,6);

   Serial1.write(26);

   delay(2000);

   serialPrint();

}


void serialPrint()

{

  while(Serial1.available()>0)

  {

    Serial.print(Serial1.read());

  }

}
