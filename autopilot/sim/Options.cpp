#include "Options.h"

#include <spdlog/spdlog.h>

#include <cxxopts.hpp>

#include <iostream>

auto
Options::parse(int argc, char** argv) -> bool
{
  cxxopts::Options options(argv[0], "Simulates vehicles and their autopilots.");

  options.add_options()                                                                                            //
    ("debug", "Print debug log messages.", cxxopts::value<bool>()->default_value("false")->implicit_value("true")) //
    ("home",
     "Where to spawn the vehicle, in terms of lat,lon,alt,heading.",
     cxxopts::value<std::string>()->default_value(home)) //
    ("sim-address",
     "The IPv4 address to bind the MAVLink TCP server to.",
     cxxopts::value<std::string>()->default_value(simAddress)) //
    ("base-port",
     "The TCP port to bind the first autopilot to.",
     cxxopts::value<int>()->default_value(std::to_string(basePort)))                                              //
    ("help", "Prints this help content.", cxxopts::value<bool>()->default_value("false")->implicit_value("true")) //
    ;

  try {
    const auto results = options.parse(argc, argv);
    debugLog = results["debug"].as<bool>();
    home = results["home"].as<std::string>();
    simAddress = results["sim-address"].as<std::string>();
    basePort = results["base-port"].as<int>();
    helpRequested = results["help"].as<bool>();
  } catch (const cxxopts::exceptions::exception& e) {
    SPDLOG_ERROR("{}", e.what());
    return false;
  }

  if (helpRequested) {
    std::cout << options.help() << std::endl;
    return false;
  }

  return true;
}
