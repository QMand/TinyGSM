/**
 * @file     TinyGsmClientSIM808.h
 * @author   Volodymyr Shymanskyy
 * @license  LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date     Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM808_H_
#define SRC_TINYGSMCLIENTSIM808_H_
// #pragma message("TinyGSM:  TinyGsmClientSIM808")

#include "TinyGsmClientSIM800.h"
#include "TinyGsmGPS.tpp"

class TinyGsmSim808 : public TinyGsmSim800, public TinyGsmGPS<TinyGsmSim808> {
  friend class TinyGsmGPS<TinyGsmSim808>;

 public:
  explicit TinyGsmSim808(Stream& stream) : TinyGsmSim800(stream) {}


  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGNSPWR=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGNSPWR=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  // works only with ans SIM808 V2
  String getGPSrawImpl() {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  // works only with ans SIM808 V2
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, int* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) { return false; }

    streamSkipUntil(',');          // GNSS run status
    if (streamGetInt(',') == 1) {  // fix status
      // init variables
      float ilat         = 0;
      float ilon         = 0;
      float ispeed       = 0;
      int   ialt         = 0;
      int   ivsat        = 0;
      int   iusat        = 0;
      float iaccuracy    = 0;
      int   iyear        = 0;
      int   imonth       = 0;
      int   iday         = 0;
      int   ihour        = 0;
      int   imin         = 0;
      float secondWithSS = 0;

      // UTC date & Time
      iyear        = streamGetInt(static_cast<int8_t>(4));  // Four digit year
      imonth       = streamGetInt(static_cast<int8_t>(2));  // Two digit month
      iday         = streamGetInt(static_cast<int8_t>(2));  // Two digit day
      ihour        = streamGetInt(static_cast<int8_t>(2));  // Two digit hour
      imin         = streamGetInt(static_cast<int8_t>(2));  // Two digit minute
      secondWithSS = streamGetFloat(',');  // 6 digit second with subseconds

      ilat   = streamGetFloat(',');     // Latitude
      ilon   = streamGetFloat(',');     // Longitude
      ialt   = streamGetFloat(',');     // MSL Altitude. Unit is meters
      ispeed = streamGetFloat(',');     // Speed Over Ground. Unit is knots.
      streamSkipUntil(',');             // Course Over Ground. Degrees.
      streamSkipUntil(',');             // Fix Mode
      streamSkipUntil(',');             // Reserved1
      streamSkipUntil(',');             // Horizontal Dilution Of Precision
      iaccuracy = streamGetFloat(',');  // Position Dilution Of Precision
      streamSkipUntil(',');             // Vertical Dilution Of Precision
      streamSkipUntil(',');             // Reserved2
      ivsat = streamGetInt(',');        // GNSS Satellites in View
      iusat = streamGetInt(',');        // GNSS Satellites Used
      streamSkipUntil(',');             // GLONASS Satellites Used
      streamSkipUntil(',');             // Reserved3
      streamSkipUntil(',');             // C/N0 max
      streamSkipUntil(',');             // HPA
      streamSkipUntil('\n');            // VPA

      // Set pointers
      if (lat != NULL) *lat = ilat;
      if (lon != NULL) *lon = ilon;
      if (speed != NULL) *speed = ispeed;
      if (alt != NULL) *alt = ialt;
      if (vsat != NULL) *vsat = ivsat;
      if (usat != NULL) *usat = iusat;
      if (accuracy != NULL) *accuracy = iaccuracy;
      if (iyear < 2000) iyear += 2000;
      if (year != NULL) *year = iyear;
      if (month != NULL) *month = imonth;
      if (day != NULL) *day = iday;
      if (hour != NULL) *hour = ihour;
      if (minute != NULL) *minute = imin;
      if (second != NULL) *second = static_cast<int>(secondWithSS);

      waitResponse();
      return true;
    }

    streamSkipUntil('\n');  // toss the row of commas
    waitResponse();
    return false;
  }
};

#endif  // SRC_TINYGSMCLIENTSIM808_H_
