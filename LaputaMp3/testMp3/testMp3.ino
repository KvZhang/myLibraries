#include <SoftwareSerial.h>

#include <LaputaMp3.h>


LaputaMp3 myMp3;
void setup()
{
  delay(1000);
}
void loop()
{
  myMp3.write(SINGLE_PLAY,BEEP_SONG);
  delay(1000);
  myMp3.write(SINGLE_PLAY,FINAL_SONG);
  delay(10000);
}

