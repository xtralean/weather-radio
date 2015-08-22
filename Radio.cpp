#include "Radio.h"

static const int POWER_UP_TIME_MS = 1000;

// Command Bytes
static const byte COMMAND_POWER_UP        = 0x01;  // Powerup
static const byte COMMAND_GET_REV         = 0x10;  // Revision info
static const byte COMMAND_POWER_DOWN      = 0x11;  // Powerdown
static const byte COMMAND_SET_PROPERTY    = 0x12;  // Sets property value
static const byte COMMAND_GET_PROPERTY    = 0x13;  // Gets property value
static const byte COMMAND_GET_INT_STATUS  = 0x14;  // Read interrupt status bits
static const byte COMMAND_WB_TUNE_FREQ    = 0x50;  // Selects WB tuning frequency
static const byte COMMAND_WB_TUNE_STATUS  = 0x52;  // Gets status of previous WB_TUNE_FREQ
static const byte COMMAND_WB_RSQ_STATUS   = 0x53;  // RSQ of current channel
static const byte COMMAND_WB_SAME_STATUS  = 0x54;  // SAME info for current channel
static const byte COMMAND_WB_ASQ_STATUS   = 0x55;  // Gets status of 1050 Hz alert tone
static const byte COMMAND_WB_AGC_STATUS   = 0x57;  // Gets status of AGC
static const byte COMMAND_WB_AGC_OVERRIDE = 0x58;  // Enable or disable WB AGC
static const byte COMMAND_GPO_CTL         = 0x80;  // Configures GPO3 as output or hi-z
static const byte COMMAND_GPO_SET         = 0x81;  // Sets GPO3 output level

// Properties
static const unsigned int PROPERTY_GPO_IEN                    = 0x0001;
static const unsigned int PROPERTY_REFCLK_FREQ                = 0x0201;
static const unsigned int PROPERTY_REFCLK_PRESCALE            = 0x0202;
static const unsigned int PROPERTY_RX_VOLUME                  = 0x4000;
static const unsigned int PROPERTY_RX_HARD_MUTE               = 0x4001;
static const unsigned int PROPERTY_WB_MAX_TUNE_ERROR          = 0x5108;
static const unsigned int PROPERTY_RSQ_INTERRUPT_SOURCE       = 0x5200;
static const unsigned int PROPERTY_WB_RSQ_SNR_HIGH_THRESHOLD  = 0x5201;
static const unsigned int PROPERTY_WB_RSQ_SNR_LOW_THRESHOLD   = 0x5202;
static const unsigned int PROPERTY_WB_RSQ_RSSI_HIGH_THRESHOLD = 0x5203;
static const unsigned int PROPERTY_WB_RSQ_RSSI_LOW_THRESHOLD  = 0x5204;
static const unsigned int PROPERTY_WB_ASQ_INTERRUPT_SOURCE    = 0x5600;
static const unsigned int PROPERTY_WB_SAME_INTERRUPT_SOURCE   = 0x5500;

static int gpio1 = D4;
static int gpio2 = D5;
static int senPin = D3;
static int rstPin = D2;

// In 2-wire mode, connecting the SEN pin to the Arduino is optional.
//  If it's not connected, make sure SEN_ADDRESS is defined as 1
//  assuming SEN is pulled high as on the breakout board.
#define SEN_ADDRESS 1
// In two-wire mode: if SEN is pulled high
#if SEN_ADDRESS
  #define SI4707_ADDRESS 0b1100011
#else  // Else if SEN is pulled low
  #define SI4707_ADDRESS 0b0010001
#endif

Radio::Radio()
{

}

bool Radio::begin()
{
  pinMode(gpio1, OUTPUT);
  pinMode(gpio2, OUTPUT);
  pinMode(senPin, OUTPUT);
  pinMode(rstPin, OUTPUT);

  digitalWrite(rstPin, LOW);  // Keep the SI4707 in reset

  digitalWrite(gpio1, HIGH);  // set interface to I2C
  digitalWrite(gpio2, LOW);

  if (SEN_ADDRESS)
    digitalWrite(senPin, HIGH);
  else
    digitalWrite(senPin, LOW);

  Wire.begin();
  delay(1);

  digitalWrite(rstPin, HIGH);

  powerUp();

  if (command_Get_Rev(1) == 7)
    return true;

  return false;
}

byte Radio::setWBFrequency(long freq)
{
  long freqKhz = constrain(freq, 162400, 162550);;
  return command_Tune_Freq((freqKhz * 10)/25);
}

void Radio::tuneWBFrequency(signed char increment)
{
  unsigned int freq = getWBFrequency();
  freq += increment;
  freq = constrain(freq, 64960, 65020);

  command_Tune_Freq(freq);
}

unsigned int Radio::getWBFrequency()
{
  return command_Tune_Status(0, 2);
}

byte Radio::getRSSI()
{
  return command_RSQ_Status(4);
}

byte Radio::getSNR()
{
  return command_RSQ_Status(5);
}

signed char Radio::getFreqOffset()
{
  return (signed char) command_RSQ_Status(7);
}

void Radio::setMuteVolume(boolean mute)
{
  setProperty(PROPERTY_RX_HARD_MUTE, (mute<<1) | (mute<<0));
}

void Radio::setVolume(int vol)
{
  vol = constrain(vol, 0, 63); // vol should be between 0 and 63
  setProperty(PROPERTY_RX_VOLUME, vol);
}

void Radio::powerUp(void)
{
  cmd[0] = COMMAND_POWER_UP;  // Command 0x01: Power Up
  cmd[1] = 0x53;  // GP02 output enabled, crystal osc enabled, WB receive mode
  cmd[2] = 0x05;  // Use analog audio outputs
  writeCommand(3, cmd, 0, rsp);

  delay(POWER_UP_TIME_MS);
}

byte Radio::command_Tune_Freq(unsigned int frequency)
{
  cmd[0] = COMMAND_WB_TUNE_FREQ;
  cmd[1] = 0;
  cmd[2] = (uint8_t)(frequency >> 8);
  cmd[3] = (uint8_t)(frequency & 0x00FF);
  writeCommand(4, cmd, 1, rsp);

  delay(500); // Delay required to allow time to tune

  return (command_Tune_Status(1, 1) >> 8);
}

unsigned int Radio::command_Tune_Status(byte intAck, byte returnIndex)
{
  cmd[0] = COMMAND_WB_TUNE_STATUS;
  cmd[1] = (intAck & 0x01);
  writeCommand(2, cmd, 6, rsp);

  return ((rsp[returnIndex] << 8) | rsp[returnIndex + 1]);
}

byte Radio::command_Get_Rev(byte returnIndex)
{
  cmd[0] = COMMAND_GET_REV;
  writeCommand(1, cmd, 9, rsp);

  return rsp[returnIndex];
}

byte Radio::command_SAME_Status(byte returnIndex)
{
  cmd[0] = COMMAND_WB_SAME_STATUS;
  cmd[1] = 0;
  cmd[2] = 0;

  writeCommand(3, cmd, 14, rsp);

  return rsp[returnIndex];
}

byte Radio::command_RSQ_Status(byte returnIndex)
{

  cmd[0] = COMMAND_WB_RSQ_STATUS;
  cmd[1] = 0;

  writeCommand(2, cmd, 8, rsp);

  return rsp[returnIndex];
}

byte Radio::command_Get_Int_Status(void)
{
  cmd[0] = COMMAND_GET_INT_STATUS;
  cmd[1] = 0;
  rsp[1] = 0;

  writeCommand(1, cmd, 1, rsp);

  return rsp[0];
}

void Radio::waitForCTS(void)
{
  byte status = 0;
  int i = 1000;

  while (--i && !(status&0x80))
  {
    i2cReadBytes(1, &status);
    delayMicroseconds(500);
  }
}

void Radio::writeCommand(byte cmdSize, byte * command, byte replySize, byte * reply)
{
  waitForCTS();
  i2cWriteBytes(cmdSize, command);
  waitForCTS();
  if (replySize)
  {
    i2cReadBytes(replySize, reply);
  }

  for (int i=0; i<replySize; i++)
  {
    reply[i] += 1;
    reply[i] -= 1;
  }
}

unsigned int Radio::getProperty(unsigned int property)
{
  cmd[0] = COMMAND_GET_PROPERTY;
  cmd[1] = 0;
  cmd[2] = (byte)((property & 0xFF00) >> 8);
  cmd[3] = (byte)(property & 0x00FF);
  writeCommand(4, cmd, 4, rsp);

  return ((rsp[2] << 8) | rsp[3]);
}

void Radio::setProperty(unsigned int propNumber, unsigned int propValue)
{
  cmd[0] = COMMAND_SET_PROPERTY;
  cmd[1] = 0;
  cmd[2] = (uint8_t)(propNumber >> 8);
  cmd[3] = (uint8_t)(propNumber & 0x00FF);
  cmd[4] = (uint8_t)(propValue >> 8);
  cmd[5] = (uint8_t)(propValue & 0x00FF);

  writeCommand(6, cmd, 1, rsp);
}

void Radio::i2cReadBytes(byte number_bytes, byte * data_in)
{
  int timeout = 1000;

  Wire.requestFrom((byte) SI4707_ADDRESS, number_bytes);
  while ((Wire.available() < number_bytes) && (--timeout))
    ;
  while((number_bytes--) && timeout)
  {
    *data_in++ = Wire.read();
  }
  Wire.endTransmission();
}

void Radio::i2cWriteBytes(uint8_t number_bytes, uint8_t *data_out)
{
  Wire.beginTransmission(SI4707_ADDRESS);
  while (number_bytes--)
  {
    Wire.write(*data_out++);
  }
  Wire.endTransmission();
}
