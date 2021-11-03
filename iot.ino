//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                          //
//                                     Reloj NTP                                                            //
//                                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Bibliotecas.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include                              <time.h>                              // para calcular el tiempo
#include                              <WiFi.h>                              // para wifi
#include                              <WiFiUdp.h>                           // para leer udp de ntp
#include                              <U8g2lib.h>                           // para imprimir en el oled

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constantes.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define   FONT_ONE_HEIGHT               8                                   // tamaño de letra
#define   FONT_TWO_HEIGHT               20                                  // tamaño de letra (reloj)
#define   NTP_DELAY_COUNT               20                                  // delay para actualizar udp
#define   NTP_PACKET_LENGTH             48                                  // longitud de ntp
#define   TIME_ZONE                     (-6)                                // zona horaria
#define   UDP_PORT                      4000                                // puerto udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Variables.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

char      chBuffer[128];                                                    // buffer
char      chPassword[] =                  "0r0zc0T0rr3z3566";               // contraseña de wifi
char      chSSID[] =                      "Wu-Tang LAN";                    // SSID de wifi
bool      bTimeReceived =                 false;                            // indica si se recibio el udp
U8G2_SSD1306_128X64_NONAME_F_HW_I2C       u8g2(U8G2_R0, 16, 15, 4);         // OLED
int       nWifiStatus =                   WL_IDLE_STATUS;                   // wifi status
WiFiUDP   Udp;                                                              // paquete udp
bool      tick =                          false;                            //tick para intercalar el encendido del LED
                                     
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Setup.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(LED,OUTPUT);
  // Serial.
  
  Serial.begin(115200);
  while(!Serial)
  {
    Serial.print('.');
  }

  // configuracion OLED.
  
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);

  //se imprimen nuestros nombres
      u8g2.clearBuffer();
      u8g2.drawStr(0, FONT_ONE_HEIGHT, "Calderon Acero");
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 2, "Garibaldi Oliva");
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 3, "Orozco Torrez");
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 4, "Padilla Valdez");
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 6, "Seguridad D04");
      u8g2.sendBuffer();
      delay(5000);
 
  // Wifi.

    // comienza la conexion wifi
    
    u8g2.clearBuffer();
    sprintf(chBuffer, "%s", "Conectandose a:");
    u8g2.drawStr(64 - (u8g2.getStrWidth(chBuffer) / 2), 0, chBuffer);
    sprintf(chBuffer, "%s", chSSID);
    u8g2.drawStr(64 - (u8g2.getStrWidth(chBuffer) / 2), 31 - (FONT_ONE_HEIGHT / 2), chBuffer);
    u8g2.sendBuffer();
    delay(1000);

    // Connecta a wifi.

    Serial.print("Buscando el tiempo");
    WiFi.begin(chSSID, chPassword);
    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();
    sprintf(chBuffer, "Reloj conectado a %s.", chSSID);
    Serial.println(chBuffer);
    
    // Muestra el estatus del wifi
      
      u8g2.clearBuffer();
      sprintf(chBuffer, "%s", "WiFi");
      u8g2.drawStr(64 - (u8g2.getStrWidth(chBuffer) / 2), 0, chBuffer);
      // IP del dispositivo      
      char  chIp[81];
      WiFi.localIP().toString().toCharArray(chIp, sizeof(chIp) - 1);
      sprintf(chBuffer, "IP  : %s", chIp);
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 2, chBuffer);
      // SSID de red      
      sprintf(chBuffer, "SSID: %s", chSSID);
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 3, chBuffer);
      // Calidad de conexion.      
      sprintf(chBuffer, "RSSI: %d", WiFi.RSSI());
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 4, chBuffer);      
      u8g2.drawStr(0, FONT_ONE_HEIGHT * 6, "Esperando el tiempo...");  
      u8g2.sendBuffer();
  // Udp.
  
  Udp.begin(UDP_PORT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Loop principal.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  loop()
{

  static  int   nNtpDelay = NTP_DELAY_COUNT;
  static  byte  chNtpPacket[NTP_PACKET_LENGTH];

  if(bTimeReceived == false)
  {
      nNtpDelay += 1;
      if(nNtpDelay >= NTP_DELAY_COUNT)
      {
          // envia la solicitud ntp y reinicia el delay
          nNtpDelay = 0;
            
            // Inicializa ntp packet.              
                memset(chNtpPacket, 0, NTP_PACKET_LENGTH);
                chNtpPacket[0]  = 0b00011011;
            // envia el ntp packet.
            IPAddress ipNtpServer(129, 6, 15, 28);
            Udp.beginPacket(ipNtpServer, 123);
            Udp.write(chNtpPacket, NTP_PACKET_LENGTH);
            Udp.endPacket();
      }
      if(nNtpDelay == (NTP_DELAY_COUNT - 1))
      {         
          if(Udp.parsePacket())
          {
              // Lee el paquete
              
              Udp.read(chNtpPacket, NTP_PACKET_LENGTH);
              struct  timeval tvTimeValue = {0, 0};              
              tvTimeValue.tv_sec = ((unsigned long)chNtpPacket[40] << 24) +       // bits 24 a 31 de ntp time
                                   ((unsigned long)chNtpPacket[41] << 16) +       // bits 16 a 23 de ntp time
                                   ((unsigned long)chNtpPacket[42] <<  8) +       // bits  8 a 15 de ntp time
                                   ((unsigned long)chNtpPacket[43]) -             // bits  0 a  7 de ntp time
                                   (((70UL * 365UL) + 17UL) * 86400UL) +
                                   (TIME_ZONE * 3600UL) +
                                   (5);
              
              settimeofday(& tvTimeValue, NULL);
              
              // paquete recibido
      
              bTimeReceived = true;
              
              // apaga el wifi

              WiFi.mode(WIFI_OFF);
          }
      }
  }
  
  // Actualiza OLED.

  if(bTimeReceived)
  {      
      struct  timeval tvTimeValue;
      gettimeofday(& tvTimeValue, NULL);
  
      u8g2.clearBuffer();    
      struct tm * tmPointer = localtime(& tvTimeValue.tv_sec);
    
      // Muestra la fecha
  
      strftime(chBuffer, sizeof(chBuffer), "Hoy es %d/%b/%Y",  tmPointer);
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(64 - (u8g2.getStrWidth(chBuffer) / 2), 0, chBuffer);
  
      // Muestra la hora
      
      strftime(chBuffer, sizeof(chBuffer), "%I:%M:%S",  tmPointer);
      u8g2.setFont(u8g2_font_fur20_tn);
      u8g2.drawStr(10, 63 - FONT_TWO_HEIGHT, chBuffer);      
      u8g2.sendBuffer();
  }

  //Espera un segundo antes de volver a ejecutar el bucle
  delay(1000);
  //Si el LED esta prendido lo apaga o viceversa
  if(tick){
    digitalWrite(LED,HIGH);
    tick = !tick;
  }
  else{
    digitalWrite(LED,LOW);
    tick = !tick;
  }
}
