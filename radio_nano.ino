// Eldario v2 adapted by Emanuel Setio Dewo
// More info at http://dewo.wordpress.com
// Please install these required libraries first
 
#include <RTClib.h>
#include <SparkFunSi4703.h>
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_PCF8574.h>
 
LiquidCrystal_PCF8574 lcd(0x3F);
 
byte kursor[8] = {
  B00000,
  B10000,
  B11000,
  B11100,
  B11000,
  B10000,
  B00000
};
byte volchar0[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte volchar1[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
 
byte silentchar[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
};
 
int SDIO = A4;
int SCLK = A5;
int resetPin = 2;
 
Si4703_Breakout radio(resetPin, SDIO, SCLK);
int channel;
int _channel;
byte volume;
byte _volume;
int rdsBuffer[10];
 
int LED = 13;
int pinA = 16; // A2
int pinB = 15; // A1
int pinMD = 14; // A0
 
int minFreq = 760;
int maxFreq = 1080;
int maxVol = 14;
int minVol = 0;
 
int pinALast;
int aVal;
int aMD = 0;
boolean bCW;
boolean md;
 
int maxNyala = 15000;
long Nyala;
boolean isNyala;
boolean berubah;
 
boolean setup_jam;
int setup_mode;
int __tahun;
int __bulan;
int __tanggal;
int __jam;
int __menit;
 
 
int interval_jam = 30000;
long _jam;
 
RTC_DS3231 rtc;
 
char hari[7][4] = {"Mgg","Sen","Sel","Rab","Kam","Jum","Sab"};
 
void setup() {
  Serial.begin(57600);
  Serial.println("Radio Si4703");
   
  lcd.begin(16,2);
  lcd.createChar(0, silentchar);
  lcd.createChar(1, volchar0);
  lcd.createChar(2, volchar1);
  lcd.createChar(3, kursor);
  lcd.home();
  lcd.setBacklight(true);
    lcd.print("Eldario by Dewo");
  //lcd.print(" FM      Volume");
   
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinMD, INPUT);
  pinALast = digitalRead(pinA);
  berubah = false;
  md = false;
  // Baca volume
  _volume = EEPROM.read(0);
  if (_volume > maxVol) volume = 1;
  else volume = _volume;
  Serial.print("Volume: ");
  Serial.println(_volume);
   
  // Baca frekuensi
  _channel = EEPROM.read(1);
  _channel = _channel * 10;
  _channel = _channel + EEPROM.read(2);
  if (_channel < minFreq || _channel > maxFreq) channel = minFreq;
  else channel = _channel;
  Serial.print("Frekuensi: ");
  Serial.println(_channel);
  setup_mode = 0;
  // Apakah perubahan jam ditekan?
  if (digitalRead(pinMD) == LOW) {
    setup_jam = true;
    lcd.clear();
    lcd.print("Setup Waktu");
    lcd.setCursor(0,1);
    lcd.print("Lepaskan Tombol");
    Serial.print("Setup waktu");
     
    DateTime now = rtc.now();
    __tahun = now.year();
    __bulan = now.month();
    __tanggal = now.day();
 
    __jam = now.hour();
    __menit = now.minute();
    delay(5000);
    Tampilan_setup();
  }
  else {
    setup_jam = false;
    delay(3000);
    Nyalakan_radio();
    isNyala = false;
    Tampilan();
    ambil_jam();
  }
}
 
void loop() {
  aVal = digitalRead(pinA);
  if (aVal != pinALast) { // knop diputar
    if (digitalRead(pinB) != aVal) { // berputar ke kanan
      if (setup_jam) {
        lcd.setCursor(2,1);
        lcd.print("          ");
        lcd.setCursor(2,1);
        switch(setup_mode) {
          case 0 :
            __tahun++;
            lcd.print(__tahun, DEC);
            break;
          case 1 :
            __bulan++;
            if (__bulan > 12) __bulan = 1;
            lcd.print(__bulan, DEC);
            break;
          case 2 :
            __tanggal++;
            if (__tanggal > 31) __tanggal = 1;
            lcd.print(__tanggal, DEC);
            break;
          case 3 :
            __jam++;
            if (__jam > 23) __jam = 0;
            lcd.print(__jam, DEC);
            break;
          case 4 :
            __menit++;
            if (__menit > 59) __menit = 0;
            lcd.print(__menit, DEC);
            break;
        }
      }
      else {
        if (md) {
          channel = radio.seekUp();
        }
        else {
          volume++;
          if (volume > maxVol) volume = maxVol;
          radio.setVolume(volume);
        }
        Tampilan();
      }
      bCW = true;
    }
 
    else { // berputar ke kiri
      if (setup_jam) {
        lcd.setCursor(2,1);
        lcd.print("          ");
        lcd.setCursor(2,1);
        switch(setup_mode) {
          case 0 :
            __tahun--;
            lcd.print(__tahun, DEC);
            break;
          case 1 :
            __bulan--;
            if (__bulan < 1) __bulan = 12;
            lcd.print(__bulan, DEC);
            break;
          case 2 :
            __tanggal--;
            if (__tanggal < 1) __tanggal = 31;
            lcd.print(__tanggal, DEC);
            break;
          case 3 :
            __jam--;
            if (__jam < 0) __jam = 23;
            lcd.print(__jam, DEC);
            break;
          case 4 :
            __menit--;
            if (__menit < 0) __menit = 59;
            lcd.print(__menit, DEC);
            break;
        }
      }
      else {
        if (md) {
          channel = radio.seekDown();
        }
        else {
          if (volume > 0) {
            volume--;
            radio.setVolume(volume);
          }
        }
        Tampilan();
      }
    }
     
  }
  pinALast = aVal;
 
  aMD = digitalRead(pinMD);
  if (aMD == LOW) {
    delay(250);
    if (setup_jam) {
      setup_mode++;
      Tampilan_setup();
    }
    else {
      md = !md;
      digitalWrite(LED, md);
      Tampilan();
    }
  }
 
  if (millis() - Nyala > maxNyala && isNyala) {
    if (_volume != volume) {
      berubah = true;
      _volume = volume;
      EEPROM.write(0, _volume);
      Serial.print("Simpan Volume: ");
      Serial.println(_volume);
    }
    if (_channel != channel) {
      berubah = true;
      _channel = channel;
      EEPROM.write(1, _channel / 10);
      EEPROM.write(2, _channel % 10);
      Serial.print("Simpan Frekuensi: ");
      Serial.println(_channel);
    }
 
    if (berubah == true) {
      berubah = false;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print(" Menyimpan...");
      delay(1000);
    }
 
    md = false;
    digitalWrite(LED, md);
    Tampilan();
    isNyala = false;
    lcd.setBacklight(false);
  }
 
  if (millis() > _jam + interval_jam) {
    if (!setup_jam) {
      _jam = millis();
      ambil_jam();
    }
  }
}
 
void Tampilan_setup() {
  lcd.clear();
  switch(setup_mode) {
    case 0 :
      lcd.print("Ubah Tahun : ");
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(__tahun, DEC);
      break;
   
    case 1 :
      lcd.print("Ubah Bulan : ");
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(__bulan, DEC);
      break;
   
    case 2 :
      lcd.print("Ubah Tanggal :");
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(__tanggal, DEC);
      break;
   
    case 3 :
      lcd.print("Ubah Jam :   ");
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(__jam, DEC);
      break;
   
    case 4 :
      lcd.print("Ubah Menit : ");
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(__menit, DEC);
      break;
 
    default :
      setup_jam = false;
      setup_mode = 0;
      rtc.adjust(DateTime(__tahun, __bulan, __tanggal, __jam, __menit, 0));
      lcd.print("Tgl telah diubah");
      lcd.setCursor(0,1);
      for (int i=0; i<16; i++) {
        lcd.print(">");
        delay(100);
      }
      delay(1000);
      Nyalakan_radio();                                                                                                                                                                            
      Tampilan();
      ambil_jam();
      break;
  }
}
 
void Tampilan() {
  if (!isNyala) {
    isNyala = true;
    lcd.setBacklight(true);
  }
  Nyala = millis();
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(1,1);
  // Frekuensi
  lcd.print(channel/10);
  lcd.print(".");
  lcd.print(channel %10);
 
  // Baris volume
  lcd.setCursor(9,1);
  for (int i=0; i < maxVol/2; i++) {
    lcd.write(byte(0));
  }
  lcd.setCursor(9,1);
  for (int i=0; i < volume/2; i++) {
    lcd.write(byte(2));
  }
  if (volume % 2 > 0) {
    lcd.write(byte(1));
  }
 
  // Tampilkan mode
  if (md) {
    lcd.setCursor(0,1);
  }
  else {
    lcd.setCursor(8,1);
  }
  //lcd.print("~");
  lcd.write(byte(3));
}
 
void ambil_jam() {
  lcd.setCursor(0,0);
  lcd.print(" ");
   
  DateTime now = rtc.now();
  lcd.print(hari[now.dayOfTheWeek()]);
  //lcd.print(now.dayOfTheWeek());
  lcd.print(" ");
   
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour(), DEC);
  lcd.print(":");
 
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute(), DEC);
  lcd.print(" ");
 
  if (now.day() < 10) lcd.print("0");
  lcd.print(now.day());
  lcd.print("/");
 
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
}
 
void Nyalakan_radio() {
  radio.powerOn();
  radio.setChannel(channel);
  radio.setVolume(volume);
}
