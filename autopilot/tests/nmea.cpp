#include <AP_NMEA.h>

#include <gtest/gtest.h>

#include <string>

namespace {

struct Message final
{
  std::string talker;

  std::string type;

  std::vector<std::string> fields;

  bool checksumPassed{ false };
};

class FakeInterpreter final : public AP::NMEAInterpreter
{
public:
  FakeInterpreter(Message* message)
    : message_(message)
  {
  }

  void onMessageBegin() override
  {
    //
  }

  void onMessageEnd(const bool checksumPassed) override { message_->checksumPassed = checksumPassed; }

  void onTalker(const char* talkerId, uint8_t) override { message_->talker = talkerId; }

  void onType(const char* type, uint8_t) override { message_->type = type; }

  void onField(const char* field, uint8_t, uint8_t) override { message_->fields.emplace_back(field); }

private:
  Message* message_{};
};

} // namespace

TEST(NMEA, ParseGGA)
{
  Message message;
  FakeInterpreter interpreter(&message);
  AP::NMEAParser parser(&interpreter);
  const char sentence[] = "$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F\r\n";
  for (size_t i = 0; i < (sizeof(sentence) - 1); i++) {
    parser.write(sentence[i]);
  }
  EXPECT_TRUE(message.checksumPassed);
  EXPECT_EQ(message.talker, "GP");
  EXPECT_EQ(message.type, "GGA");
  ASSERT_EQ(message.fields.size(), 14);
  EXPECT_EQ(message.fields.at(0), "172814.0");
  EXPECT_EQ(message.fields.at(1), "3723.46587704");
}
