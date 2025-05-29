#include "nmea.hpp"
#include "stmepic.hpp"

using namespace stmepic::gps;
using namespace stmepic;


NmeaParser::NmeaParser() {
  nmea_parsers["GPGGA"] = std::bind(&NmeaParser::parse_gga, this, std::placeholders::_1);
  nmea_parsers["GPGLL"] = std::bind(&NmeaParser::parse_gll, this, std::placeholders::_1);
  nmea_parsers["GPGSA"] = std::bind(&NmeaParser::parse_gsa, this, std::placeholders::_1);
  nmea_parsers["GPGSV"] = std::bind(&NmeaParser::parse_gsv, this, std::placeholders::_1);
  nmea_parsers["GPRMC"] = std::bind(&NmeaParser::parse_rmc, this, std::placeholders::_1);
  nmea_parsers["GPVTG"] = std::bind(&NmeaParser::parse_vtg, this, std::placeholders::_1);
  nmea_parsers["GPGBS"] = std::bind(&NmeaParser::parse_gbs, this, std::placeholders::_1);
}

int NmeaParser::hex_to_int(char c) {
  if(c >= '0' && c <= '9')
    return c - '0';
  if(c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if(c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

Status NmeaParser::parse(const std::string &nmeaSentence) {
  if(!is_valid_sentence(nmeaSentence)) {
    return Status::Invalid("Invalid NMEA sequence.");
  }
  // Find the type of NMEA sentence (e.g., GGA, GLL, etc.)
  size_t commaPos = nmeaSentence.find(',');
  if(commaPos == std::string::npos) {
    return Status::Invalid("NMEA sentence does not contain a command type.");
  }
  std::string sentenceType = nmeaSentence.substr(1, commaPos - 1);
  // Check if the sentence type is in the parsers map
  auto it = nmea_parsers.find(sentenceType);
  if(it == nmea_parsers.end())
    return Status::KeyError("NMEA sentence type not supported");
  return it->second(nmeaSentence);
}

bool NmeaParser::is_valid_sentence(const std::string &sentence) {
  if(sentence.empty()) {
    return false;
  }

  // Check if the sentence starts with '$' and ends with '*'
  if(sentence.front() != '$') {
    return false;
  }
  // Check if the sentence has a valid checksum
  size_t checksumPos = sentence.find('*');
  if(checksumPos == std::string::npos || checksumPos < 3) {
    return false;
  }

  if(checksumPos + 3 != sentence.length()) {
    return false;
  }

  // Calculate the checksum
  unsigned char checksum = 0;
  for(size_t i = 1; i < checksumPos; ++i) {
    char c = sentence[i];
    checksum ^= static_cast<unsigned char>(c);
    if(c < ' ' || c > '~' || c == '*' || c == '$') {
      return false;
    }
  }
  // Convert the checksum to a hexadecimal string
  unsigned int sentenceChecksum = hex_to_int(sentence[checksumPos + 1]) << 4 | hex_to_int(sentence[checksumPos + 2]);
  if(checksum != sentenceChecksum) {
    return false;
  }
  // If all checks passed, the sentence is valid
  return true;
}

Status NmeaParser::parse_by_character(char c) {
  // Append the character to the current sentence
  currentSentence += c;

  // Check if we have reached the end of a sentence
  if(c == '\n' || c == '\r') {
    // Process the complete sentence
    auto result = parse(currentSentence);
    currentSentence.clear(); // Clear the current sentence after processing
    return result;
  }
  return Status::OK(); // Continue accumulating characters
}


void NmeaParser::reset() {
  currentSentence.clear();
  gga_data = {};
  gll_data = {};
  gsa_data = {};
  gsv_data = {};
  rmc_data = {};
  vtg_data = {};
  gbs_data = {};
}


utc_time_t NmeaParser::parse_utc_time(int time) {
  utc_time_t utc_time;
  // tim eis tored in from hh:min:sec as int combined value
  utc_time.hours   = time / 10000;
  utc_time.minutes = (time / 100) % 100;
  utc_time.seconds = time % 100;
  return utc_time;
}

utc_date_t NmeaParser::parse_utc_date(int date) {
  utc_date_t utc_date;
  // date is stored as ddmmyy
  utc_date.day   = date / 10000;
  utc_date.month = (date / 100) % 100;
  utc_date.year  = date % 100 + 2000;
  return utc_date;
}

std::vector<std::string> NmeaParser::split(const std::string &str) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end   = str.find(',');

  while(end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + 1;
    end   = str.find(',', start);
  }
  result.push_back(str.substr(start)); // Add the last segment
  return result;
}


gga_data_t NmeaParser::get_gga_data() const {
  return gga_data;
}

gll_data_t NmeaParser::get_gll_data() const {
  return gll_data;
}

gsa_data_t NmeaParser::get_gsa_data() const {
  return gsa_data;
}

gsv_data_t NmeaParser::get_gsv_data() const {
  return gsv_data;
}

rmc_data_t NmeaParser::get_rmc_data() const {
  return rmc_data;
}

vtg_data_t NmeaParser::get_vtg_data() const {
  return vtg_data;
}


Status NmeaParser::parse_gga(const std::string &sentence) {
  // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 15)
    return Status::Invalid("Not enough fields in GGA sentence");

  gga_data_t gga_temp;

  // UTC Time
  gga_temp.time = parse_utc_time(fields[1].empty() ? 0 : std::stoi(fields[1]));
  // Latitude
  gga_temp.latitude       = fields[2].empty() ? 0.0 : std::stod(fields[2]);
  char latitude_direction = fields[3].empty() ? 'N' : fields[3][0];
  // Longitude
  gga_temp.longitude       = fields[4].empty() ? 0.0 : std::stod(fields[4]);
  char longitude_direction = fields[5].empty() ? 'E' : fields[5][0];
  // Fix Quality
  gga_temp.fix_quality = fields[6].empty() ? 0 : std::stoi(fields[6]);
  // Number of Satellites
  gga_temp.num_satellites = fields[7].empty() ? 0 : std::stoi(fields[7]);
  // HDOP
  gga_temp.hdop = fields[8].empty() ? 0.0 : std::stod(fields[8]);
  // Altitude
  gga_temp.altitude       = fields[9].empty() ? 0.0 : std::stod(fields[9]);
  gga_temp.altitude_units = fields[10].empty() ? 'M' : fields[10][0];
  // Height of geoid
  gga_temp.height       = fields[11].empty() ? 0.0 : std::stod(fields[11]);
  gga_temp.height_units = fields[12].empty() ? 'M' : fields[12][0];
  // DGPS Age
  gga_temp.dgps_age = fields[13].empty() ? 0.0 : std::stod(fields[13]);
  // DGPS Station ID (optional, not used here)

  // Convert latitude and longitude to degrees
  gga_temp.latitude *= (latitude_direction == 'N' ? 1 : -1);
  gga_temp.longitude *= (longitude_direction == 'E' ? 1 : -1);

  gga_data = gga_temp;
  return Status::OK();
}

Status NmeaParser::parse_gll(const std::string &sentence) {
  // $GPGLL,4916.45,N,12311.12,W,225444,A,D*48
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 7)
    return Status::Invalid("Not enough fields in GLL sentence");

  gll_data_t gll_temp;

  // Latitude
  gll_temp.latitude       = fields[1].empty() ? 0.0 : std::stod(fields[1]);
  char latitude_direction = fields[2].empty() ? 'N' : fields[2][0];

  // Longitude
  gll_temp.longitude       = fields[3].empty() ? 0.0 : std::stod(fields[3]);
  char longitude_direction = fields[4].empty() ? 'E' : fields[4][0];

  // UTC Time
  gll_temp.time = parse_utc_time(fields[5].empty() ? 0 : std::stoi(fields[5]));

  // Status
  gll_temp.status = fields[6].empty() ? 'V' : fields[6][0];

  // Mode (optional, may include checksum)
  if(fields.size() > 7 && !fields[7].empty()) {
    gll_temp.mode = fields[7][0];
  } else {
    gll_temp.mode = 'N';
  }

  // Convert latitude and longitude to degrees
  gll_temp.latitude *= (latitude_direction == 'N' ? 1 : -1);
  gll_temp.longitude *= (longitude_direction == 'E' ? 1 : -1);

  gll_data = gll_temp;
  return Status::OK();
}

Status NmeaParser::parse_gsa(const std::string &sentence) {
  // $GPGSA,A,3,04,05,07,08,09,10,11,12,13,14,15,16,17,18,19*3A
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 17)
    return Status::Invalid("Not enough fields in GSA sentence");

  gsa_data_t gsa_temp = {};
  gsa_temp.mode       = fields[1].empty() ? 'M' : fields[1][0];
  gsa_temp.fix_type   = fields[2].empty() ? 1 : std::stoi(fields[2]);

  // Satellite PRNs (fields 3-14)
  for(int i = 0; i < 12; ++i) {
    if(fields[3 + i].empty()) {
      gsa_temp.sats[i] = 0;
    } else {
      gsa_temp.sats[i] = std::stoi(fields[3 + i]);
    }
  }

  // PDOP, HDOP, VDOP (fields 15-17)
  gsa_temp.pdop = fields[15].empty() ? 0.0 : std::stod(fields[15]);
  gsa_temp.hdop = fields[16].empty() ? 0.0 : std::stod(fields[16]);
  // gsa_temp.vdop = (fields.size() > 17 && !fields[17].empty()) ? std::stod(fields[17]) : 0.0;

  gsa_data = gsa_temp;
  return Status::OK();
}

Status NmeaParser::parse_vtg(const std::string &sentence) {
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 9)
    return Status::Invalid("Not enough fields in VTG sentence");

  vtg_data_t vtg_temp;
  vtg_temp.true_track_degrees     = fields[1].empty() ? 0.0f : std::stof(fields[1]);
  vtg_temp.magnetic_track_degrees = fields[3].empty() ? 0.0f : std::stof(fields[3]);
  vtg_temp.speed_knots            = fields[5].empty() ? 0.0f : std::stof(fields[5]);
  vtg_temp.speed_kmh              = fields[7].empty() ? 0.0f : std::stof(fields[7]);

  // Mode is usually in field 9, but may include the checksum (e.g., "A*3D")
  if(!fields[9].empty()) {
    vtg_temp.faa_mode = fields[9][0];
  } else {
    vtg_temp.faa_mode = 'N';
  }

  vtg_data = vtg_temp;
  return Status::OK();
}

Status NmeaParser::parse_gsv(const std::string &sentence) {
  // $GPGSV,4,1,16,01,40,000,42,02,30,060,45,03,20,120,50*7A
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 4)
    return Status::Invalid("Not enough fields in GSV sentence");

  gsv_data_t gsv_temp;
  gsv_temp.total_msgs   = fields[1].empty() ? 0 : std::stoi(fields[1]);
  gsv_temp.msg_num      = fields[2].empty() ? 0 : std::stoi(fields[2]);
  gsv_temp.sats_in_view = fields[3].empty() ? 0 : std::stoi(fields[3]);

  // Each satellite: 4 fields (PRN, elevation, azimuth, SNR)
  int sat_count = (fields.size() - 4) / 4;
  gsv_temp.satellites.clear();
  for(int i = 0; i < sat_count; ++i) {
    gsv_data_t::satellite_info sat;
    int base      = 4 + i * 4;
    sat.prn       = (base < fields.size() && !fields[base].empty()) ? std::stoi(fields[base]) : 0;
    sat.elevation = (base + 1 < fields.size() && !fields[base + 1].empty()) ? std::stoi(fields[base + 1]) : 0;
    sat.azimuth   = (base + 2 < fields.size() && !fields[base + 2].empty()) ? std::stoi(fields[base + 2]) : 0;
    gsv_temp.satellites.push_back(sat);
  }

  // If this is the first message, start a new set; otherwise, append
  if(gsv_temp.msg_num == 1) {
    gsv_data = gsv_temp;
    return Status::OK();
  }
  gsv_data.satellites.insert(gsv_data.satellites.end(), gsv_temp.satellites.begin(), gsv_temp.satellites.end());
  gsv_data.total_msgs   = gsv_temp.total_msgs;
  gsv_data.msg_num      = gsv_temp.msg_num;
  gsv_data.sats_in_view = gsv_temp.sats_in_view;
  return Status::OK();
}

Status NmeaParser::parse_rmc(const std::string &sentence) {
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 12)
    return Status::Invalid("Not enough fields in RMC sentence");

  rmc_data_t rmc_temp;
  // 1: UTC time
  rmc_temp.utc_time = parse_utc_time(fields[1].empty() ? 0 : std::stoi(fields[1]));
  // 2: Status
  char status = fields[2].empty() ? 'V' : fields[2][0];
  // 3: Latitude
  rmc_temp.latitude = fields[3].empty() ? 0.0 : std::stod(fields[3]);
  // 4: N/S
  char latitude_direction = fields[4].empty() ? 'N' : fields[4][0];
  // 5: Longitude
  rmc_temp.longitude = fields[5].empty() ? 0.0 : std::stod(fields[5]);
  // 6: E/W
  char longitude_direction = fields[6].empty() ? 'E' : fields[6][0];
  // 7: Speed
  rmc_temp.speed = fields[7].empty() ? 0.0 : std::stod(fields[7]);
  // 8: Course
  rmc_temp.course = fields[8].empty() ? 0.0 : std::stod(fields[8]);
  // 9: Date
  int data            = fields[9].empty() ? 0 : std::stoi(fields[9]);
  rmc_temp.date.day   = data / 10000;
  rmc_temp.date.month = (data / 100) % 100;
  rmc_temp.date.year  = data % 100 + 2000;
  // 10: Magnetic variation (optional)
  rmc_temp.variation = fields[10].empty() ? 0.0 : std::stod(fields[10]);
  // 11: Magnetic variation E/W (optional, not used here)
  // 12: Mode
  char mode = fields[12].empty() ? 'N' : fields[12][0];

  // Convert latitude and longitude to degrees
  rmc_temp.latitude *= (latitude_direction == 'N' ? 1 : -1);
  rmc_temp.longitude *= (longitude_direction == 'E' ? 1 : -1);

  rmc_data = rmc_temp;
  return Status::OK();
}

Status NmeaParser::parse_gbs(const std::string &sentence) {
  // $GPGBS,0.5,0.5,0.5,1,2,3*7A
  std::vector<std::string> fields = split(sentence);

  if(fields.size() < 7)
    return Status::Invalid("Not enough fields in GBS sentence");

  gbs_data_t gbs_temp;
  // UTC Time (field 1)
  gbs_temp.time = parse_utc_time(fields[1].empty() ? 0 : std::stoi(fields[1]));
  // Estimated error in latitude (meters) (field 2)
  gbs_temp.err_latitude = fields[2].empty() ? 0.0 : std::stod(fields[2]);
  // Estimated error in longitude (meters) (field 3)
  gbs_temp.err_longitude = fields[3].empty() ? 0.0 : std::stod(fields[3]);
  // Estimated error in altitude (meters) (field 4)
  gbs_temp.err_altitude = fields[4].empty() ? 0.0 : std::stod(fields[4]);
  // Satellite ID (field 5)
  gbs_temp.svid = fields[5].empty() ? 0 : std::stoi(fields[5]);
  // Probability (field 6)
  gbs_temp.prob = fields[6].empty() ? 0.0 : std::stod(fields[6]);
  // Bias (meters) (field 7)
  gbs_temp.bias = (fields.size() > 7 && !fields[7].empty()) ? std::stod(fields[7]) : 0.0;
  // Standard deviation (meters) (field 8)
  gbs_temp.stddev = (fields.size() > 8 && !fields[8].empty()) ? std::stod(fields[8]) : 0.0;

  gbs_data = gbs_temp;
  return Status::OK();
}