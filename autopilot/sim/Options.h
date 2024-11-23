#pragma once

#include <string>

/**
 * @brief Command line options for running the sim.
 * */
struct Options final
{
  /**
   * @brief Log in debug mode.
   * */
  bool debugLog{ false };

  /**
   * @brief Where to spawn the vehicle. The default is Long Point in Cape Cod.
   * */
  std::string home{ "42.034763572014384,-70.16775492706986,0,0" };

  /**
   * @brief The address to bind the MAVLink server to.
   * */
  std::string simAddress{ "127.0.0.1" };

  /**
   * @brief The starting port number to bind the MAVLink interface to.
   * */
  int basePort{ 5760 };

  /**
   * @brief Whether or not the help option was passed.
   * */
  bool helpRequested{ false };

  [[nodiscard]] auto parse(int argc, char** argv) -> bool;
};
