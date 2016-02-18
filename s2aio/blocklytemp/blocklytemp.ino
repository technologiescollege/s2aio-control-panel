#include <U8glib.h>;
 U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);

#include <Adafruit_NeoPixel.h>
 Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 6, NEO_GRB + NEO_KHZ800);

void setup()
{
  u8g.firstPage();
do {u8g.setFont(u8g_font_unifont);
u8g.drawStr( 0, 22, "Bonjour !");
} while( u8g.nextPage());
delay(1000);

  pinMode(6, OUTPUT);

  pixels.begin();

}


void loop()
{
  u8g.firstPage();
  do {
  u8g.drawStr(0, 12, "Exemple");
  }
   while( u8g.nextPage() );

  pixels.setPixelColor(0, pixels.Color(255,255,255));
  pixels.show();

}