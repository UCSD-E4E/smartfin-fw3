/*
TinyGPS++ - a small GPS library for Arduino providing universal NMEA parsing
Based on work by and "distanceBetween" and "courseTo" courtesy of Maarten Lamers.
Suggestion to add satellites, courseTo(), and cardinal() by Matt Monson.
Location precision improvements suggested by Wayne Holder.
Copyright (C) 2008-2013 Mikal Hart
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "TinyGPSMod.h"
#include <math.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define _GPRMCterm   "GNRMC"
#define _GPGGAterm   "GNGGA"

// Converts degrees to radians.
#define radians(angleDegrees) (angleDegrees * M_PI / 180.0)

// Converts radians to degrees.
#define degrees(angleRadians) (angleRadians * 180.0 / M_PI)

#define TWO_PI 6.283185307179586476925286766559
#define sq(x) ((x)*(x))

TinyGPSPlus::TinyGPSPlus()
  :  parity(0)
  ,  isChecksumTerm(false)
  ,  term{0}
  ,  curSentenceType(GPS_SENTENCE_OTHER)
  ,  curTermNumber(0)
  ,  curTermOffset(0)
  ,  sentenceHasFix(false)
  ,  isGPSModuleEnabled(false)
  ,  customElts(0)
  ,  customCandidates(0)
  ,  encodedCharCount(0)
  ,  sentencesWithFixCount(0)
  ,  failedChecksumCount(0)
  ,  passedChecksumCount(0)
{
}

//
// public methods
//

bool TinyGPSPlus::encode(char c)
{
  ++encodedCharCount;

  switch(c)
  {
  case ',': // term terminators
    parity ^= (uint8_t)c;
  case '\r':
  case '\n':
  case '*':
    {
      bool isValidSentence = false;
      if (curTermOffset < sizeof(term))
      {
        term[curTermOffset] = 0;
        isValidSentence = endOfTermHandler();
      }
      ++curTermNumber;
      curTermOffset = 0;
      isChecksumTerm = c == '*';
      return isValidSentence;
    }
    break;

  case '$': // sentence begin
    curTermNumber = curTermOffset = 0;
    parity = 0;
    curSentenceType = GPS_SENTENCE_OTHER;
    isChecksumTerm = false;
    sentenceHasFix = false;
    return false;

  default: // ordinary characters
    if (curTermOffset < sizeof(term) - 1)
      term[curTermOffset++] = c;
    if (!isChecksumTerm)
      parity ^= c;
    return false;
  }

  return false;
}

//
// internal utilities
//
int TinyGPSPlus::fromHex(char a)
{
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}

// static
// Parse a (potentially negative) number with up to 2 decimal digits -xxxx.yy
int32_t TinyGPSPlus::parseDecimal(const char *term)
{
  bool negative = *term == '-';
  if (negative) ++term;
  int32_t ret = 100 * (int32_t)atol(term);
  while (isdigit(*term)) ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 10 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += term[2] - '0';
  }
  return negative ? -ret : ret;
}

// static
// Parse degrees in that funny NMEA format DDMM.MMMM
void TinyGPSPlus::parseDegrees(const char *term, RawDegrees &deg)
{
  uint32_t leftOfDecimal = (uint32_t)atol(term);
  uint16_t minutes = (uint16_t)(leftOfDecimal % 100);
  uint32_t multiplier = 10000000UL;
  uint32_t tenMillionthsOfMinutes = minutes * multiplier;

  deg.deg = (int16_t)(leftOfDecimal / 100);

  while (isdigit(*term))
    ++term;

  if (*term == '.')
    while (isdigit(*++term))
    {
      multiplier /= 10;
      tenMillionthsOfMinutes += (*term - '0') * multiplier;
    }

  deg.billionths = (5 * tenMillionthsOfMinutes + 1) / 3;
  deg.negative = false;
}

#define COMBINE(sentence_type, term_number) (((unsigned)(sentence_type) << 5) | term_number)

// Processes a just-completed term
// Returns true if new sentence has just passed checksum test and is validated
bool TinyGPSPlus::endOfTermHandler()
{
  // If it's the checksum term, and the checksum checks out, commit
  if (isChecksumTerm)
  {
    byte checksum = 16 * fromHex(term[0]) + fromHex(term[1]);
    if (checksum == parity)
    {
      passedChecksumCount++;
      if (sentenceHasFix)
        ++sentencesWithFixCount;

      switch(curSentenceType)
      {
      case GPS_SENTENCE_GPRMC:
        date.commit();
        time.commit();
        if (sentenceHasFix)
        {
           location.commit();
           speed.commit();
           course.commit();
        }
        break;
      case GPS_SENTENCE_GPGGA:
        time.commit();
        if (sentenceHasFix)
        {
          location.commit();
          altitude.commit();
        }
        satellites.commit();
        hdop.commit();
        break;
      }

      // Commit all custom listeners of this sentence type
      for (TinyGPSCustom *p = customCandidates; p != NULL && strcmp(p->sentenceName, customCandidates->sentenceName) == 0; p = p->next)
         p->commit();
      return true;
    }

    else
    {
      ++failedChecksumCount;
    }

    return false;
  }

  // the first term determines the sentence type
  if (curTermNumber == 0)
  {
    if (!strcmp(term, _GPRMCterm))
      curSentenceType = GPS_SENTENCE_GPRMC;
    else if (!strcmp(term, _GPGGAterm))
      curSentenceType = GPS_SENTENCE_GPGGA;
    else
      curSentenceType = GPS_SENTENCE_OTHER;

    // Any custom candidates of this sentence type?
    for (customCandidates = customElts; customCandidates != NULL && strcmp(customCandidates->sentenceName, term) < 0; customCandidates = customCandidates->next);
    if (customCandidates != NULL && strcmp(customCandidates->sentenceName, term) > 0)
       customCandidates = NULL;

    return false;
  }

  if (curSentenceType != GPS_SENTENCE_OTHER && term[0])
    switch(COMBINE(curSentenceType, curTermNumber))
  {
    case COMBINE(GPS_SENTENCE_GPRMC, 1): // Time in both sentences
    case COMBINE(GPS_SENTENCE_GPGGA, 1):
      time.setTime(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 2): // GPRMC validity
      sentenceHasFix = term[0] == 'A';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 3): // Latitude
    case COMBINE(GPS_SENTENCE_GPGGA, 2):
      location.setLatitude(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 4): // N/S
    case COMBINE(GPS_SENTENCE_GPGGA, 3):
      location.rawNewLatData.negative = term[0] == 'S';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 5): // Longitude
    case COMBINE(GPS_SENTENCE_GPGGA, 4):
      location.setLongitude(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 6): // E/W
    case COMBINE(GPS_SENTENCE_GPGGA, 5):
      location.rawNewLngData.negative = term[0] == 'W';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 7): // Speed (GPRMC)
      speed.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 8): // Course (GPRMC)
      course.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 9): // Date (GPRMC)
      date.setDate(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 6): // Fix data (GPGGA)
      sentenceHasFix = term[0] > '0';
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 7): // Satellites used (GPGGA)
      satellites.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 8): // HDOP
      hdop.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 9): // Altitude (GPGGA)
      altitude.set(term);
      break;
  }

  // Set custom values as needed
  for (TinyGPSCustom *p = customCandidates; p != NULL && strcmp(p->sentenceName, customCandidates->sentenceName) == 0 && p->termNumber <= curTermNumber; p = p->next)
    if (p->termNumber == curTermNumber)
         p->set(term);

  return false;
}

/* static */
double TinyGPSPlus::distanceBetween(double lat1, double long1, double lat2, double long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1-long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}

double TinyGPSPlus::courseTo(double lat1, double long1, double lat2, double long2)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  double dlon = radians(long2-long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double a1 = sin(dlon) * cos(lat2);
  double a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}

const char *TinyGPSPlus::cardinal(double course)
{
  static const char* directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
  int direction = (int)((course + 11.25f) / 22.5f);
  return directions[direction % 16];
}

void TinyGPSLocation::commit()
{
   rawLatData = rawNewLatData;
   rawLngData = rawNewLngData;
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSLocation::setLatitude(const char *term)
{
   TinyGPSPlus::parseDegrees(term, rawNewLatData);
}

void TinyGPSLocation::setLongitude(const char *term)
{
   TinyGPSPlus::parseDegrees(term, rawNewLngData);
}

double TinyGPSLocation::lat()
{
   updated = false;
   double ret = rawLatData.deg + rawLatData.billionths / 1000000000.0;
   return rawLatData.negative ? -ret : ret;
}

double TinyGPSLocation::lng()
{
   updated = false;
   double ret = rawLngData.deg + rawLngData.billionths / 1000000000.0;
   return rawLngData.negative ? -ret : ret;
}

int32_t TinyGPSLocation::lat_int32()
{
  updated = false;
  int32_t ret = rawLatData.deg * 1000000UL  + rawLatData.billionths / 1000;
  return rawLatData.negative ? -ret : ret;
}

int32_t TinyGPSLocation::lng_int32()
{
  updated = false;
  int32_t ret = rawLngData.deg * 1000000UL  + rawLngData.billionths / 1000;
  return rawLngData.negative ? -ret : ret;
}

void TinyGPSDate::commit()
{
   date = newDate;
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSTime::commit()
{
   time = newTime;
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSTime::setTime(const char *term)
{
   newTime = (uint32_t)TinyGPSPlus::parseDecimal(term);
}

void TinyGPSDate::setDate(const char *term)
{
   newDate = atol(term);
}

uint16_t TinyGPSDate::year()
{
   updated = false;
   uint16_t year = date % 100;
   return year + 2000;
}

uint8_t TinyGPSDate::month()
{
   updated = false;
   return (date / 100) % 100;
}

uint8_t TinyGPSDate::day()
{
   updated = false;
   return date / 10000;
}

uint8_t TinyGPSTime::hour()
{
   updated = false;
   return time / 1000000;
}

uint8_t TinyGPSTime::minute()
{
   updated = false;
   return (time / 10000) % 100;
}

uint8_t TinyGPSTime::second()
{
   updated = false;
   return (time / 100) % 100;
}

uint8_t TinyGPSTime::centisecond()
{
   updated = false;
   return time % 100;
}

void TinyGPSDecimal::commit()
{
   val = newval;
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSDecimal::set(const char *term)
{
   newval = TinyGPSPlus::parseDecimal(term);
}

void TinyGPSInteger::commit()
{
   val = newval;
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSInteger::set(const char *term)
{
   newval = atol(term);
}

TinyGPSCustom::TinyGPSCustom(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber)
{
   begin(gps, _sentenceName, _termNumber);
}

void TinyGPSCustom::begin(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber)
{
   lastCommitTime = 0;
   updated = valid = false;
   sentenceName = _sentenceName;
   termNumber = _termNumber;
   memset(stagingBuffer, '\0', sizeof(stagingBuffer));
   memset(buffer, '\0', sizeof(buffer));

   // Insert this item into the GPS tree
   gps.insertCustom(this, _sentenceName, _termNumber);
}

void TinyGPSCustom::commit()
{
   strcpy(this->buffer, this->stagingBuffer);
   lastCommitTime = millis();
   valid = updated = true;
}

void TinyGPSCustom::set(const char *term)
{
   strncpy(this->stagingBuffer, term, sizeof(this->stagingBuffer));
}

void TinyGPSPlus::insertCustom(TinyGPSCustom *pElt, const char *sentenceName, int termNumber)
{
   TinyGPSCustom **ppelt;

   for (ppelt = &this->customElts; *ppelt != NULL; ppelt = &(*ppelt)->next)
   {
      int cmp = strcmp(sentenceName, (*ppelt)->sentenceName);
      if (cmp < 0 || (cmp == 0 && termNumber < (*ppelt)->termNumber))
         break;
   }

   pElt->next = *ppelt;
   *ppelt = pElt;
}

bool TinyGPSPlus::checkComms()
{
  /* Verify the communications interface by querying the version 
    returns TRUE if comms are up, else false
  */
  
  uint8_t gps_get_version[] = {0xB5, 0x62, 0x0A, 0x04, 0, 0, 14, 52};
  uint32_t timeout_milliseconds = 5000;
  uint32_t start_time_millis = millis();
  int index = 0;
  uint8_t chk_a = 0;
  uint8_t chk_b = 0;
  uint8_t read_checksum_a = 0;
  uint8_t read_checksum_b = 0;
  uint16_t length = 0;
  bool done = FALSE;
  bool part_of_checksum = FALSE;
  
  Serial5.flush();
  for (uint32_t i = 0; i<sizeof(gps_get_version); i++) Serial5.write(gps_get_version[i]);
  

  /* Verify that we get a valid packet response; we currently don't care what the version actually is */
  while(((millis() - start_time_millis) < timeout_milliseconds) && (done == FALSE))
  {
    if(Serial5.available() > 0)
    {
      uint8_t byte = Serial5.read();

      switch(index)
      {
        case 0:
          if (byte != 0xB5)
            index = -1;
          break;
        case 1:
          if (byte != 0x62)
            index = -1;
          break;
        case 2:
          if (byte != 0x0A)
            index = -1;
          else
            part_of_checksum = TRUE;
          break;
        case 3:
          if (byte != 0x04)
            index = -1;
          break;
        case 4:
          length = byte;
          break;
        case 5:
          length += (byte << 8);
          break;
        default:
          if (index == (6 + length))
          {
            part_of_checksum = FALSE;
            read_checksum_a = byte;
          }
          else if (index == (7 + length))
          {
            read_checksum_b = byte;
            done = TRUE;
          }
          break;
      }
      if (part_of_checksum)
      {
        chk_a += byte;
        chk_b += chk_a;
      }
      index++;
    }
  }
  
  if ((chk_a == read_checksum_a) && (chk_b == read_checksum_b))
    return TRUE;
  return FALSE;
}

void TinyGPSPlus::gpsModuleInit()
{
  // ublox command to set to 115200 baud
  // byte gps_baud_config[20] =   {B5   62  06 00 14 00 01 00 00 00 D0 08   00      00   00  C2  01  00    07 00 03 00    00 00 00 00 C0 7E
  byte gps_baud_115200_msg[28] = {181, 98, 6, 0, 20, 0, 1, 0, 0, 0, 208, 8, 0, 0, 0, 194, 1, 0, 7, 0, 3, 0, 0, 0, 0, 0, 192, 126};
  Serial5.begin(GPS_INIT_BAUD_RATE);
  Serial5.flush();
  delay(1000);
  // change GPS to output 115200 baud.  Try 3 times to make sure it gets through
  for (int i = 0; i<28; i++) Serial5.write(gps_baud_115200_msg[i]);
  delay(500);
  for (int i = 0; i<28; i++) Serial5.write(gps_baud_115200_msg[i]);
  delay(500);
  for (int i = 0; i<28; i++) Serial5.write(gps_baud_115200_msg[i]);
  Serial5.flush();
  Serial5.end();
  delay(500);

  // change microcontroller to 115200 baud
  Serial5.begin(GPS_BAUD_RATE);
  Serial5.flush();
  delay(1000);

  // disable non GGA NEMA strings for output to reduce latency
  nemaMsgDisable("GSV");
  delay(500);
  nemaMsgDisable("GSA");
  delay(500);
  nemaMsgDisable("VTG");
  delay(500);
  // nemaMsgDisable("RMC");
  // delay(500);
  nemaMsgDisable("GLL");
  delay(500);
  Serial5.flush();

  isGPSModuleEnabled = true;
}

void TinyGPSPlus::gpsModuleStop()
{
  Serial5.end();
  // Serial.println("GPS_END");
  // sprintf(global_char_buffer, "GPS Off\n");
  // logData(SD_LOG_DATA, global_char_buffer, 8);
  isGPSModuleEnabled = false;
}

inline int TinyGPSPlus::calculateChecksum (const char *msg)
{
  int checksum = 0;
  for (int i = 0; msg[i] && i < 32; i++)
    checksum ^= (unsigned char)msg[i];

  return checksum;
}

inline int TinyGPSPlus::nemaMsgSend (const char *msg)
{
  char checksum[8];
  snprintf(checksum, sizeof(checksum) - 1, "*%.2X", calculateChecksum(msg));
  Serial5.printf("$%s%s\n", msg, checksum);
  return 1; 
}

inline int TinyGPSPlus::nemaMsgDisable (const char *nema)
{
  if (strlen(nema) != 3) return 0;

  char tmp[32];
  //snprintf(tmp, sizeof(tmp) - 1, "PUBX,40,%s,0,0,0,0", nema);
  snprintf(tmp, sizeof(tmp)-1, "PUBX,40,%s,0,0,0,0,0,0", nema);
  nemaMsgSend(tmp);

  return 1;
}

inline int TinyGPSPlus::nemaMsgEnable (const char *nema)
{
  if (strlen(nema) != 3) return 0;

  char tmp[32];
  //snprintf(tmp, sizeof(tmp) - 1, "PUBX,40,%s,0,1,0,0", nema);
  snprintf(tmp, sizeof(tmp)-1, "PUBX,40,%s,0,1,0,0,0,0", nema);
  nemaMsgSend(tmp);

  return 1;
}
