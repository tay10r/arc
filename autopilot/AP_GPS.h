#pragma once

#include "AP_Mavlink.h"
#include "AP_NMEA.h"
#include "AP_Time.h"

#include <stdint.h>

namespace AP {

class GPSSensor : public NMEAInterpreter
{
public:
  /**
   * @brief Gets a pointer to the null GPS sensor object.
   *
   * @return A pointer to a GPS sensor that never successfully reads anything.
   * */
  static auto null() -> GPSSensor*;

  /**
   * @brief This structure represents
   * */
  struct GGA final
  {
    /**
     * @brief The time of day that the sample was read at, in terms of milliseconds.
     * */
    int32_t timeOfDay{};

    /**
     * @brief Latitude, in terms of degrees in WGS84.
     * */
    float lat{};

    /**
     * @brief Longitude, in terms of degrees in WGS84.
     * */
    float lon{};

    /**
     * @brief Altitude in meters MSL.
     * */
    float alt{};

    /**
     * @brief The geoid separation in terms of meters.
     * */
    float geoidSeparation{};

    /**
     * @brief The number of satellites in use.
     * */
    uint8_t numSatellites{};

    /**
     * @brief Whether or not there is a fix on location.
     * */
    bool hasFix{ false };
  };

  struct VTG final
  {
    /**
     * @brief Heading over ground, in terms of radians from true North.
     * */
    float hdg{};

    /**
     * @brief Speed over ground, in terms of meters per second.
     * */
    float speed{};
  };

  using GGA_Callback = void (*)(void*, const GGA&);

  using VTG_Callback = void (*)(void*, const VTG&);

  virtual ~GPSSensor() = default;

  /**
   * @brief Attempts to read from the GPS stream.
   *
   * @return True if data was successfully parsed, false otherwise.
   * */
  [[nodiscard]] virtual auto read() -> bool = 0;

  /**
   * @brief Sets up the class for handling incoming data.
   *
   * @param userData A pointer to pass to the callback functions.
   *
   * @param ggaFunc The function to call if a GGA message is received.
   *
   * @param vtgFunc The function to call if a VTG message is received.
   * */
  void setup(void* userData, GGA_Callback ggaFunc, VTG_Callback vtgFunc);

protected:
  /**
   * @brief This method is meant to be called by the derived classes in order to read data incoming from the sensor.
   *
   * @param buffer The NMEA data to read.
   *
   * @param size The number of bytes in the buffer.
   *
   * @return True on success, false on failure.
   * */
  [[nodiscard]] auto parseData(const char* buffer, uint8_t size) -> bool;

  void notifyGGA(const GGA& gga);

  void notifyVTG(const VTG& vtg);

private:
  enum class Type : uint8_t
  {
    kOther,
    kGGA,
    kVTG
  };

  void onMessageBegin() override;

  void onMessageEnd(bool checksumPassed) override;

  void onTalker(const char*, uint8_t) override;

  void onType(const char* type, uint8_t typeSize) override;

  void onField(const char* field, const uint8_t fieldSize, const uint8_t fieldIndex) override;

  void onGGAField(const char* field, const uint8_t fieldSize, const uint8_t fieldIndex);

  auto parseTimeOfDay(const char* str, uint8_t len) -> bool;

private:
  NMEAParser parser_{ this };

  Type type_{ Type::kOther };

  void* userData_{};

  GGA_Callback ggaFunc_{};

  VTG_Callback vtgFunc_{};

  GGA gga_{};

  VTG vtg_{};
};

class GPSComponent final : public MAVLinkComponent
{
public:
  void setSensor(GPSSensor* sensor);

  void loop(MAVLinkBus& bus, uint32_t timeDelta) override;

protected:
  [[nodiscard]] auto readFromSensor() -> bool;

  [[nodiscard]] auto publishReport(MAVLinkBus& bus) -> bool;

  static void onGGA(void* selfPtr, const GPSSensor::GGA& gga);

  static void onVTG(void* selfPtr, const GPSSensor::VTG& vtg);

private:
  /**
   * @brief The GPS sensor being read from.
   * */
  GPSSensor* sensor_{ GPSSensor::null() };

  /**
   * @brief Publish GPS data every second.
   * */
  Timer publishTimer_{ 1000000ul };

  /**
   * @brief A flag to indicate that a GPS report is due.
   * */
  bool publishDue_{ false };

  /**
   * @brief How often to attempt to read data from the GPS.
   * */
  Timer readTimer_{ 100000ul };

  /**
   * @brief The last received GGA message.
   * */
  GPSSensor::GGA lastGGA_{};

  /**
   * @brief The initial height above sea level, which is used to compute the height above home.
   * */
  float initialHeight_{};

  /**
   * @brief Whether or not the first MSL level has been received.
   * */
  bool receivedFirstHeight_{};

  /**
   * @brief The time since boot, in terms of milliseconds.
   * */
  uint32_t timeSinceBootMs_{};

  /**
   * @brief The fractional component of the time since boot.
   * */
  uint32_t timeSinceBootFractional_{};
};

} // namespace AP
