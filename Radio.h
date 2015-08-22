#include "application.h"

class Radio {
public:
  Radio();

  bool begin();

  uint8_t setWBFrequency(long freq);
  void tuneWBFrequency(signed char increment);
  unsigned int getWBFrequency();
  uint8_t getRSSI();
  uint8_t getSNR();
  signed char getFreqOffset();
  void setMuteVolume(boolean mute);
  void setVolume(int vol);

private:
  uint8_t rsp[15];
  uint8_t cmd[8];

  void powerUp(void);
  uint8_t command_Tune_Freq(unsigned int frequency);
  unsigned int command_Tune_Status(uint8_t intAck, uint8_t returnIndex);
  uint8_t command_Get_Rev(uint8_t returnIndex);
  uint8_t command_SAME_Status(uint8_t returnIndex);
  uint8_t command_RSQ_Status(uint8_t returnIndex);
  uint8_t command_Get_Int_Status(void);

  void waitForCTS(void);
  void writeCommand(uint8_t cmdSize, uint8_t * command, uint8_t replySize, uint8_t * reply);
  unsigned int getProperty(unsigned int property);
  void setProperty(unsigned int propNumber, unsigned int propValue);
  void i2cReadBytes(uint8_t number_bytes, uint8_t * data_in);
  void i2cWriteBytes(uint8_t number_bytes, uint8_t *data_out);

};
