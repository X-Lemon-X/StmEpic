#pragma once
#include "stmepic.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>


/**
 * @file nmea.hpp
 *  @brief NMEA Parser for GPS data from various Modules.
 */

/**
 * @defgroup Gps
 * @brief Functions related to different gps devices.
 * @{
 */


/**
 * @defgroup NmeaParser_Barometers_Gps NmeaParser
 * @brief NmeaParser from Nmea sentences.
 * @{
 */

namespace stmepic::gps {

struct utc_time_t {
  int hours;
  int minutes;
  int seconds;
  size_t second_epoch; // seconds since epoch
};

struct utc_date_t {
  int day;
  int month;
  int year;
};

struct gga_data_t {
  utc_time_t time;
  double latitude;
  double longitude;
  int fix_quality;
  int num_satellites;
  double hdop;
  double altitude;
  char altitude_units;
  double height;
  char height_units;
  double dgps_age;
};

struct gbs_data_t {
  utc_time_t time;
  double err_latitude;
  double err_longitude;
  double err_altitude;
  int svid;
  double prob;
  double bias;
  double stddev;
};

struct gll_data_t {
  double latitude;
  double longitude;
  utc_time_t time;
  char status;
  char mode;
};

struct gsa_data_t {
  char mode;
  int fix_type;
  int sats[12];
  double pdop;
  double hdop;
  double vdop;
};

struct gsv_data_t {
  int total_msgs;
  int msg_num;
  int sats_in_view;

  struct satellite_info {
    int prn;
    int elevation;
    int azimuth;
  };

  std::vector<satellite_info> satellites;
};

struct rmc_data_t {
  utc_time_t utc_time;
  char status;
  double latitude;
  double longitude;
  double speed;
  double course;
  utc_date_t date;
  double variation;
};

struct vtg_data_t {
  float true_track_degrees;
  float magnetic_track_degrees;
  float speed_knots;
  float speed_kmh;
  char faa_mode;
};


/**
 * @brief NMEA Parser Class
 * This class is responsible for parsing NMEA sentences from GPS devices.
 * It supports various NMEA sentence types including GGA, GLL, GSA, GSV, RMC, VTG, and GBS.
 * It provides methods to parse sentences character by character or as a whole string.
 * It also provides methods to retrieve parsed data for each sentence type.
 */
class NmeaParser {
public:
  /**
   * @brief Constructor for NmeaParser
   * Initializes the parser and sets up the mapping of NMEA sentence types to their respective parsing functions.
   */
  NmeaParser();

  /**
   * @brief Parse a NMEA sentence by character
   * @param c The character to parse
   * @return An integer indicating the result of the parsing operation
   * This method processes a single character from a NMEA sentence.
   * It accumulates characters until a complete sentence is formed, then parses it.
   */
  Status parse_by_character(char c);

  /**
   * @brief Parse a complete NMEA sentence
   * @param nmeaSentence The complete NMEA sentence to parse
   * @return An integer indicating the result of the parsing operation
   * This method processes a complete NMEA sentence and extracts relevant data.
   */
  Status parse(const std::string &nmeaSentence);

  /**
   * @brief Resets the parser
   *  This method clears the current sentence and resets all parsed data structures.
   */
  void reset();

  /**
   * @brief Get the parsed GGA data
   * @return gga_data_t containing parsed GGA data
   */
  gga_data_t get_gga_data() const;

  /**
   * @brief Get the parsed GLL data
   * @return gll_data_t containing parsed GLL data
   */
  gll_data_t get_gll_data() const;

  /**
   * @brief Get the parsed GSA data
   * @return gsa_data_t containing parsed GSA data
   */
  gsa_data_t get_gsa_data() const;

  /**
   * @brief Get the parsed GSV data
   * @return gsv_data_t containing parsed GSV data
   */
  gsv_data_t get_gsv_data() const;

  /**
   * @brief Get the rmc data object
   * @return rmc_data_t
   */
  rmc_data_t get_rmc_data() const;

  /**
   * @brief Get the parsed VTG data
   * @return vtg_data_t containing parsed VTG data
   */
  vtg_data_t get_vtg_data() const;

  /**
   * @brief Get the parsed GBS data
   * @return gbs_data_t containing parsed GBS data
   */
  gbs_data_t get_gbs_data() const;

private:
  static bool is_valid_sentence(const std::string &sentence);
  static int hex_to_int(char c);
  static utc_time_t parse_utc_time(int time);
  static utc_date_t parse_utc_date(int date);
  static std::vector<std::string> split(const std::string &str);
  float parce_degrees(const std::string &degrees_str);

  float to_float(const std::string &str);
  int to_int(const std::string &str);

  std::unordered_map<std::string, std::function<Status(const std::string &)>> nmea_parsers;
  std::string currentSentence;
  gga_data_t gga_data;
  gll_data_t gll_data;
  gsa_data_t gsa_data;
  gsv_data_t gsv_data;
  rmc_data_t rmc_data;
  vtg_data_t vtg_data;
  gbs_data_t gbs_data;

  Status parse_gga(const std::string &sentence);
  Status parse_gll(const std::string &sentence);
  Status parse_gsa(const std::string &sentence);
  Status parse_gsv(const std::string &sentence);
  Status parse_rmc(const std::string &sentence);
  Status parse_vtg(const std::string &sentence);
  Status parse_gbs(const std::string &sentence);
};


} // namespace stmepic::gps