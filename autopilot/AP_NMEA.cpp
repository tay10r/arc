#include "AP_NMEA.h"

namespace AP {

NMEAParser::NMEAParser(NMEAInterpreter* interpreter)
  : interpreter_(interpreter)
{
}

auto
NMEAParser::write(const char value) -> bool
{
  if (readSize_ >= (sizeof(readBuffer_) - 1)) {
    // TODO : failure
    return false;
  }

  if ((state_ == State::kNone) && (value != '$')) {
    return false;
  }

  auto complete{ false };

  switch (state_) {
    case State::kNone:
      if (value == '$') {
        interpreter_->onMessageBegin();
        state_ = State::kTalker;
        break;
      }
      break;
    case State::kTalker:
      readBuffer_[readSize_] = value;
      addToChecksum(value);
      readSize_++;
      if (readSize_ == 2) {
        readBuffer_[readSize_] = 0;
        interpreter_->onTalker(readBuffer_, readSize_);
        state_ = State::kType;
        readSize_ = 0;
        break;
      }
      break;
    case State::kType:
      addToChecksum(value);
      if (value == ',') {
        readBuffer_[readSize_] = 0;
        interpreter_->onType(readBuffer_, readSize_);
        readSize_ = 0;
        state_ = State::kFields;
      } else {
        readBuffer_[readSize_++] = value;
      }
      break;
    case State::kFields:
      if ((value == '*') || (value == ',')) {
        readBuffer_[readSize_] = 0;
        interpreter_->onField(readBuffer_, readSize_, fieldIndex_);
        readSize_ = 0;
        fieldIndex_++;
      }
      if (value == '*') {
        state_ = State::kChecksum;
        fieldIndex_ = 0;
      } else if (value == ',') {
        addToChecksum(value);
      } else {
        addToChecksum(value);
        readBuffer_[readSize_++] = value;
      }
      break;
    case State::kChecksum:
      if (value == '\n') {
        completeMessage();
        checksum_ = 0;
        state_ = State::kNone;
        readSize_ = 0;
        complete = true;
      } else if (value != '\r') {
        readBuffer_[readSize_] = value;
        readSize_++;
      }
      break;
  }

  return complete;
}

namespace {

[[nodiscard]] auto
isHexDigit(char c) -> bool
{
  if ((c >= '0') && (c <= '9')) {
    return true;
  }

  if ((c >= 'a') && (c <= 'f')) {
    return true;
  }

  if ((c >= 'A') && (c <= 'F')) {
    return true;
  }

  return false;
}

[[nodiscard]] auto
hexToByte(const char c) -> uint8_t
{
  if ((c >= 'a') && (c <= 'f')) {
    return static_cast<uint8_t>(c - 'a') + 10;
  }

  if ((c >= 'A') && (c <= 'F')) {
    return static_cast<uint8_t>(c - 'A') + 10;
  }

  return static_cast<uint8_t>(c - '0');
}

} // namespace

void
NMEAParser::completeMessage()
{
  if (readSize_ < 2) {
    // incomplete checksum
    interpreter_->onMessageEnd(false);
    return;
  }

  if (!isHexDigit(readBuffer_[0]) || !isHexDigit(readBuffer_[1])) {
    // invalid checksum characters
    interpreter_->onMessageEnd(false);
    return;
  }

  const auto expectedChecksum = (hexToByte(readBuffer_[0]) << 4) | hexToByte(readBuffer_[1]);

  interpreter_->onMessageEnd(checksum_ == expectedChecksum);
}

void
NMEAParser::addToChecksum(const char value)
{
  checksum_ ^= static_cast<uint8_t>(value);
}

void
NMEAParser::reset()
{
  readSize_ = 0;
  state_ = State::kNone;
  fieldIndex_ = 0;
}

} // namespace AP
