#pragma once

#include <stdint.h>

namespace AP {

class NMEAInterpreter
{
public:
  virtual ~NMEAInterpreter() = default;

  /**
   * @brief This is called when the parser encounters the dollar sign, which is the start of a message.
   * */
  virtual void onMessageBegin() = 0;

  /**
   * @brief This is called when the parser receives the last checksum byte and computes whether or not the checksum
   * passes.
   *
   * @param checksumPassed True if the checksum was valid, false otherwise.
   * */
  virtual void onMessageEnd(bool checksumPassed) = 0;

  /**
   * @brief This is called when the talker has been parsed.
   *
   * @param talker The string containining the talker ID.
   *
   * @param len The length of the talker.
   *
   * @note The string is null terminated.
   * */
  virtual void onTalker(const char* talker, uint8_t len) = 0;

  /**
   * @brief This is called when the sentence type has been parsed.
   *
   * @param type The string containing the sentence type.
   *
   * @param len The length of the type string.
   *
   * @note The type string is null terminated.
   * */
  virtual void onType(const char* type, uint8_t len) = 0;

  /**
   * @brief This is called when a message field is parsed.
   *
   * @param field The field that was parsed.
   *
   * @param len The length of the field.
   *
   * @param index The field index within the message.
   *
   * @note The field string is null terminated.
   * */
  virtual void onField(const char* field, uint8_t len, uint8_t index) = 0;
};

class NMEAParser final
{
public:
  NMEAParser(NMEAInterpreter* interpreter);

  void write(char value);

protected:
  enum class State
  {
    kNone,
    kTalker,
    kType,
    kFields,
    kChecksum
  };

  void reset();

  void addToChecksum(char value);

  void completeMessage();

private:
  NMEAInterpreter* interpreter_{};

  uint8_t checksum_{};

  char readBuffer_[82 + 1 /* 82 is max nmea message size, +1 for null terminator */];

  uint8_t readSize_{};

  State state_{ State::kNone };

  uint8_t fieldIndex_{};
};

} // namespace AP
