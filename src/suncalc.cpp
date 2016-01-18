/*
    LinKNX KNX home automation platform

    Sun rise/set times computation:
    Copyright (C) 1989-1992 Paul Schlyter
    C++ wrapper:
    Copyright (C) 2008 Jean-François Meessen <linknx@ouaye.net>

 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "suncalc.h"
#include "services.h"

namespace suncalc
{

/*
SUNRISET.C - computes Sun rise/set times, start/end of twilight, and
             the length of the day at any date and latitude

Written as DAYLEN.C, 1989-08-16

Modified to SUNRISET.C, 1992-12-01

(c) Paul Schlyter, 1989, 1992

Released to the public domain by Paul Schlyter, December 1992

*/


#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

/* A macro to compute the number of days elapsed since 2000 Jan 0.0 */
/* (which is equal to 1999 Dec 31, 0h UT)                           */

#define days_since_2000_Jan_0(y,m,d) \
    (367L*(y)-((7*((y)+(((m)+9)/12)))/4)+((275*(m))/9)+(d)-730530L)

/* Some conversion factors between radians and degrees */

#ifndef PI
 #define PI        3.1415926535897932384
#endif

#define RADEG     ( 180.0 / PI )
#define DEGRAD    ( PI / 180.0 )

/* The trigonometric functions in degrees */

#define sind(x)  sin((x)*DEGRAD)
#define cosd(x)  cos((x)*DEGRAD)
#define tand(x)  tan((x)*DEGRAD)

#define atand(x)    (RADEG*atan(x))
#define asind(x)    (RADEG*asin(x))
#define acosd(x)    (RADEG*acos(x))
#define atan2d(y,x) (RADEG*atan2(y,x))


/* Following are some macros around the "workhorse" function __daylen__ */
/* They mainly fill in the desired values for the reference altitude    */
/* below the horizon, and also selects whether this altitude should     */
/* refer to the Sun's center or its upper limb.                         */


/* This macro computes the length of the day, from sunrise to sunset. */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define day_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -35.0/60.0, 1 )

/* This macro computes the length of the day, including civil twilight. */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define day_civil_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -6.0, 0 )

/* This macro computes the length of the day, incl. nautical twilight.  */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define day_nautical_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -12.0, 0 )

/* This macro computes the length of the day, incl. astronomical twilight. */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define day_astronomical_twilight_length(year,month,day,lon,lat)  \
        __daylen__( year, month, day, lon, lat, -18.0, 0 )


/* This macro computes times for sunrise/sunset.                      */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define sun_rise_set(year,month,day,lon,lat,rise,set)  \
        __sunriset__( year, month, day, lon, lat, -35.0/60.0, 1, rise, set )

/* This macro computes the start and end times of civil twilight.       */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define civil_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -6.0, 0, start, end )

/* This macro computes the start and end times of nautical twilight.    */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define nautical_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -12.0, 0, start, end )

/* This macro computes the start and end times of astronomical twilight.   */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define astronomical_twilight(year,month,day,lon,lat,start,end)  \
        __sunriset__( year, month, day, lon, lat, -18.0, 0, start, end )


#define hours(x)    (((int)(x))%24)
#define minutes(x)  (((int)(((x)-hours(x))*60))%60)

/* Function prototypes */

double __daylen__( int year, int month, int day, double lon, double lat,
                   double altit, int upper_limb );

int __sunriset__( int year, int month, int day, double lon, double lat,
                  double altit, int upper_limb, double *rise, double *set );

void sunpos( double d, double *lon, double *r );

void sun_RA_dec( double d, double *RA, double *dec, double *r );

double revolution( double x );

double rev180( double x );

double GMST0( double d );

/* The "workhorse" function for sun rise/set times */

int __sunriset__( int year, int month, int day, double lon, double lat,
                  double altit, int upper_limb, double *trise, double *tset )
/***************************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value IS critical in this function!            */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing rise/set     */
/*               times, and to zero when computing start/end of       */
/*               twilight.                                            */
/*        *rise = where to store the rise time                        */
/*        *set  = where to store the set  time                        */
/*                Both times are relative to the specified altitude,  */
/*                and thus this function can be used to comupte       */
/*                various twilight times, as well as rise/set times   */
/* Return value:  0 = sun rises/sets this day, times stored at        */
/*                    *trise and *tset.                               */
/*               +1 = sun above the specified "horizon" 24 hours.     */
/*                    *trise set to time when the sun is at south,    */
/*                    minus 12 hours while *tset is set to the south  */
/*                    time plus 12 hours. "Day" length = 24 hours     */
/*               -1 = sun is below the specified "horizon" 24 hours   */
/*                    "Day" length = 0 hours, *trise and *tset are    */
/*                    both set to the time when the sun is at south.  */
/*                                                                    */
/**********************************************************************/
{
      double  d,  /* Days since 2000 Jan 0.0 (negative before) */
      sr,         /* Solar distance, astronomical units */
      sRA,        /* Sun's Right Ascension */
      sdec,       /* Sun's declination */
      sradius,    /* Sun's apparent radius */
      t,          /* Diurnal arc */
      tsouth,     /* Time when Sun is at south */
      sidtime;    /* Local sidereal time */

      int rc = 0; /* Return cde from function - usually 0 */

      /* Compute d of 12h local mean solar time */
      d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

      /* Compute local sideral time of this moment */
      sidtime = revolution( GMST0(d) + 180.0 + lon );

      /* Compute Sun's RA + Decl at this moment */
      sun_RA_dec( d, &sRA, &sdec, &sr );

      /* Compute time when Sun is at south - in hours UT */
      tsouth = 12.0 - rev180(sidtime - sRA)/15.0;

      /* Compute the Sun's apparent radius, degrees */
      sradius = 0.2666 / sr;

      /* Do correction to upper limb, if necessary */
      if ( upper_limb )
            altit -= sradius;

      /* Compute the diurnal arc that the Sun traverses to reach */
      /* the specified altitide altit: */
      {
            double cost;
            cost = ( sind(altit) - sind(lat) * sind(sdec) ) /
                  ( cosd(lat) * cosd(sdec) );
            if ( cost >= 1.0 )
                  rc = -1, t = 0.0;       /* Sun always below altit */
            else if ( cost <= -1.0 )
                  rc = +1, t = 12.0;      /* Sun always above altit */
            else
                  t = acosd(cost)/15.0;   /* The diurnal arc, hours */
      }

      /* Store rise and set times - in hours UT */
      *trise = tsouth - t;
      *tset  = tsouth + t;

      return rc;
}  /* __sunriset__ */



/* The "workhorse" function */


double __daylen__( int year, int month, int day, double lon, double lat,
                   double altit, int upper_limb )
/**********************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value is not critical. Set it to the correct   */
/*       longitude if you're picky, otherwise set to to, say, 0.0     */
/*       The latitude however IS critical - be sure to get it correct */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing day length   */
/*               and to zero when computing day+twilight length.      */
/**********************************************************************/
{
      double  d,  /* Days since 2000 Jan 0.0 (negative before) */
      obl_ecl,    /* Obliquity (inclination) of Earth's axis */
      sr,         /* Solar distance, astronomical units */
      slon,       /* True solar longitude */
      sin_sdecl,  /* Sine of Sun's declination */
      cos_sdecl,  /* Cosine of Sun's declination */
      sradius,    /* Sun's apparent radius */
      t;          /* Diurnal arc */

      /* Compute d of 12h local mean solar time */
      d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

      /* Compute obliquity of ecliptic (inclination of Earth's axis) */
      obl_ecl = 23.4393 - 3.563E-7 * d;

      /* Compute Sun's position */
      sunpos( d, &slon, &sr );

      /* Compute sine and cosine of Sun's declination */
      sin_sdecl = sind(obl_ecl) * sind(slon);
      cos_sdecl = sqrt( 1.0 - sin_sdecl * sin_sdecl );

      /* Compute the Sun's apparent radius, degrees */
      sradius = 0.2666 / sr;

      /* Do correction to upper limb, if necessary */
      if ( upper_limb )
            altit -= sradius;

      /* Compute the diurnal arc that the Sun traverses to reach */
      /* the specified altitide altit: */
      {
            double cost;
            cost = ( sind(altit) - sind(lat) * sin_sdecl ) /
                  ( cosd(lat) * cos_sdecl );
            if ( cost >= 1.0 )
                  t = 0.0;                      /* Sun always below altit */
            else if ( cost <= -1.0 )
                  t = 24.0;                     /* Sun always above altit */
            else  t = (2.0/15.0) * acosd(cost); /* The diurnal arc, hours */
      }
      return t;
}  /* __daylen__ */


/* This function computes the Sun's position at any instant */

void sunpos( double d, double *lon, double *r )
/******************************************************/
/* Computes the Sun's ecliptic longitude and distance */
/* at an instant given in d, number of days since     */
/* 2000 Jan 0.0.  The Sun's ecliptic latitude is not  */
/* computed, since it's always very near 0.           */
/******************************************************/
{
      double M,         /* Mean anomaly of the Sun */
             w,         /* Mean longitude of perihelion */
                        /* Note: Sun's mean longitude = M + w */
             e,         /* Eccentricity of Earth's orbit */
             E,         /* Eccentric anomaly */
             x, y,      /* x, y coordinates in orbit */
             v;         /* True anomaly */

      /* Compute mean elements */
      M = revolution( 356.0470 + 0.9856002585 * d );
      w = 282.9404 + 4.70935E-5 * d;
      e = 0.016709 - 1.151E-9 * d;

      /* Compute true longitude and radius vector */
      E = M + e * RADEG * sind(M) * ( 1.0 + e * cosd(M) );
            x = cosd(E) - e;
      y = sqrt( 1.0 - e*e ) * sind(E);
      *r = sqrt( x*x + y*y );              /* Solar distance */
      v = atan2d( y, x );                  /* True anomaly */
      *lon = v + w;                        /* True solar longitude */
      if ( *lon >= 360.0 )
            *lon -= 360.0;                   /* Make it 0..360 degrees */
}

void sun_RA_dec( double d, double *RA, double *dec, double *r )
{
      double lon, obl_ecl, x, y, z;

      /* Compute Sun's ecliptical coordinates */
      sunpos( d, &lon, r );

      /* Compute ecliptic rectangular coordinates (z=0) */
      x = *r * cosd(lon);
      y = *r * sind(lon);

      /* Compute obliquity of ecliptic (inclination of Earth's axis) */
      obl_ecl = 23.4393 - 3.563E-7 * d;

      /* Convert to equatorial rectangular coordinates - x is uchanged */
      z = y * sind(obl_ecl);
      y = y * cosd(obl_ecl);

      /* Convert to spherical coordinates */
      *RA = atan2d( y, x );
      *dec = atan2d( z, sqrt(x*x + y*y) );

}  /* sun_RA_dec */


/******************************************************************/
/* This function reduces any angle to within the first revolution */
/* by subtracting or adding even multiples of 360.0 until the     */
/* result is >= 0.0 and < 360.0                                   */
/******************************************************************/

#define INV360    ( 1.0 / 360.0 )

double revolution( double x )
/*****************************************/
/* Reduce angle to within 0..360 degrees */
/*****************************************/
{
      return( x - 360.0 * floor( x * INV360 ) );
}  /* revolution */

double rev180( double x )
/*********************************************/
/* Reduce angle to within +180..+180 degrees */
/*********************************************/
{
      return( x - 360.0 * floor( x * INV360 + 0.5 ) );
}  /* revolution */


/*******************************************************************/
/* This function computes GMST0, the Greenwhich Mean Sidereal Time */
/* at 0h UT (i.e. the sidereal time at the Greenwhich meridian at  */
/* 0h UT).  GMST is then the sidereal time at Greenwich at any     */
/* time of the day.  I've generelized GMST0 as well, and define it */
/* as:  GMST0 = GMST - UT  --  this allows GMST0 to be computed at */
/* other times than 0h UT as well.  While this sounds somewhat     */
/* contradictory, it is very practical:  instead of computing      */
/* GMST like:                                                      */
/*                                                                 */
/*  GMST = (GMST0) + UT * (366.2422/365.2422)                      */
/*                                                                 */
/* where (GMST0) is the GMST last time UT was 0 hours, one simply  */
/* computes:                                                       */
/*                                                                 */
/*  GMST = GMST0 + UT                                              */
/*                                                                 */
/* where GMST0 is the GMST "at 0h UT" but at the current moment!   */
/* Defined in this way, GMST0 will increase with about 4 min a     */
/* day.  It also happens that GMST0 (in degrees, 1 hr = 15 degr)   */
/* is equal to the Sun's mean longitude plus/minus 180 degrees!    */
/* (if we neglect aberration, which amounts to 20 seconds of arc   */
/* or 1.33 seconds of time)                                        */
/*                                                                 */
/*******************************************************************/

double GMST0( double d )
{
      double sidtim0;
      /* Sidtime at 0h UT = L (Sun's mean longitude) + 180.0 degr  */
      /* L = M + w, as defined in sunpos().  Since I'm too lazy to */
      /* add these numbers, I'll let the C compiler do it for me.  */
      /* Any decent C compiler will add the constants at compile   */
      /* time, imposing no runtime or code overhead.               */
      sidtim0 = revolution( ( 180.0 + 356.0470 + 282.9404 ) +
                          ( 0.9856002585 + 4.70935E-5 ) * d );
      return sidtim0;
}  /* GMST0 */


}

Logger& SolarTimeSpec::logger_m(Logger::getInstance("SolarTimeSpec"));

SolarTimeSpec::~SolarTimeSpec() {};
SunriseTimeSpec::~SunriseTimeSpec() {};
SunsetTimeSpec::~SunsetTimeSpec() {};
SolarNoonTimeSpec::~SolarNoonTimeSpec() {};

void SolarTimeSpec::importXml(ticpp::Element* pConfig)
{
    TimeSpec::importXml(pConfig);
    offset_m = RuleServer::parseDuration(pConfig->GetAttribute("offset"), true);
}

void SolarTimeSpec::exportXml(ticpp::Element* pConfig)
{
    if (offset_m != 0)
        pConfig->SetAttribute("offset", RuleServer::formatDuration(offset_m));
    TimeSpec::exportXml(pConfig);
}

void SunriseTimeSpec::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "sunrise");
    SolarTimeSpec::exportXml(pConfig);
}

void SunsetTimeSpec::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "sunset");
    SolarTimeSpec::exportXml(pConfig);
}

void SolarNoonTimeSpec::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("type", "noon");
    SolarTimeSpec::exportXml(pConfig);
}

double SunriseTimeSpec::computeTime(double rise, double set)
{
    return rise;
}

double SunsetTimeSpec::computeTime(double rise, double set)
{
    return set;
}

double SolarNoonTimeSpec::computeTime(double rise, double set)
{
    return (rise+set)/2.0;
}

void SolarTimeSpec::getData(int *min, int *hour, int *mday, int *mon, int *year, int *wdays, ExceptionDays *exception, const struct tm * timeinfo)
{
    LocationInfo* params = Services::instance()->getLocationInfo();
    double lon, lat;
    params->getCoord(&lon, &lat);
    long tzOffset = params->getGmtOffset(timeinfo);
    
    double rise, set;
    
    int    rs;
    logger_m.infoStream() << "sun_rise_set date " << timeinfo->tm_year+1900<< "-" << timeinfo->tm_mon+1 << "-" << timeinfo->tm_mday << endlog;
    rs   = suncalc::sun_rise_set( timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, lon, lat, &rise, &set );

    if (rs == 0)
    {
        double res = computeTime(rise, set);
        res += (((double)offset_m)/3600);
        *min = minutes(res + minutes((double)tzOffset/3600));
        *hour = hours(res + (double)tzOffset/3600);
        if (*min < 0 || *hour < 0)
        {
            *min = 0;
            *hour = 0;
        }
        logger_m.infoStream() << "sun_rise_set returned " << *hour<< ":" <<*min << endlog;
    }
    else
    {
        *min = 0;
        *hour = 0;
        logger_m.errorStream() << "sun_rise_set returned error." << endlog;
    }
    *mday = mday_m;
    *mon = mon_m;
    *year = year_m;
    *wdays = wdays_m;
    *exception = exception_m;
}

bool SolarTimeSpec::adjustTime(struct tm * timeinfo)
{
    LocationInfo* params = Services::instance()->getLocationInfo();
    double lon, lat;
    params->getCoord(&lon, &lat);
    long tz_offset = params->getGmtOffset(timeinfo);
    
    double rise, set;
    
    int    rs;
    logger_m.infoStream() << "adjustTime date " << timeinfo->tm_year+1900<< "-" <<timeinfo->tm_mon+1 << "-" << timeinfo->tm_mday << endlog;
    rs   = suncalc::sun_rise_set( timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, lon, lat, &rise, &set );

    if (rs == 0)
    {
        double res = computeTime(rise, set);
        res += (((double)offset_m)/3600);
        timeinfo->tm_min = minutes(res + minutes((double)tz_offset/3600));
        timeinfo->tm_hour = hours(res + (double)tz_offset/3600);
        if (timeinfo->tm_min < 0 || timeinfo->tm_hour < 0)
        {
            timeinfo->tm_min = 0;
            timeinfo->tm_hour = 0;
        }
        logger_m.infoStream() << "adjustTime returned " << timeinfo->tm_hour<< ":" <<timeinfo->tm_min << endlog;
    }
    else
    {
        logger_m.errorStream() << "adjustTime returned error." << endlog;
        return false;
    }
    return true;
}

Logger& SolarInfo::logger_m(Logger::getInstance("SolarInfo"));

SolarInfo::SolarInfo(struct tm * timeinfo) : rs_m(0)
{
    LocationInfo* params = Services::instance()->getLocationInfo();
    double lon, lat;
    params->getCoord(&lon, &lat);
    tz_offset_m = params->getGmtOffset(timeinfo);

    logger_m.infoStream() << "SolarInfo date " << timeinfo->tm_year+1900<< "-" <<timeinfo->tm_mon+1 << "-" << timeinfo->tm_mday << endlog;
    rs_m  = suncalc::sun_rise_set( timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, lon, lat, &rise_m, &set_m );
}

SolarInfo::~SolarInfo() {};

bool SolarInfo::getSunrise(int *min, int *hour)
{
    return get(rise_m, min, hour);
}

bool SolarInfo::getSunset(int *min, int *hour)
{
    return get(set_m, min, hour);
}

bool SolarInfo::getNoon(int *min, int *hour)
{
    return get((rise_m+set_m)/2.0, min, hour);
}

bool SolarInfo::get(double res, int *min, int *hour)
{
    if (rs_m == 0)
    {
        *min = minutes(res + minutes((double)tz_offset_m/3600));
        *hour = hours(res + (double)tz_offset_m/3600);
        if (*min < 0 || *hour < 0)
        {
            *min = 0;
            *hour = 0;
        }
        logger_m.infoStream() << "returned " << *hour<< ":" << *min << endlog;
    }
    else
    {
        logger_m.errorStream() << "returned error." << endlog;
        return false;
    }
    return true;
}

void LocationInfo::importXml(ticpp::Element* pConfig)
{
    pConfig->GetAttributeOrDefault("lon", &lon_m, 0);
    pConfig->GetAttributeOrDefault("lat", &lat_m, 0);
}

void LocationInfo::exportXml(ticpp::Element* pConfig)
{
    pConfig->SetAttribute("lon", lon_m);
    pConfig->SetAttribute("lat", lat_m);
}

/* The GMT offset calculation code below has been borrowed from the APR library
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

LocationInfo::LocationInfo() : lon_m(0), lat_m(0)
{
#if defined(HAVE_TM_GMTOFF) || defined(HAVE___TM_GMTOFF)
    gmtOffset_m = 0;
#else
    struct timeval now;
    time_t t1, t2;
    struct tm t;

    gettimeofday(&now, NULL);
    t1 = now.tv_sec;
    t2 = 0;

    t = *gmtime(&t1);
    t.tm_isdst = 0; /* we know this GMT time isn't daylight-savings */
    t2 = mktime(&t);
    gmtOffset_m = (long)difftime(t1, t2);
#endif
}

long LocationInfo::getGmtOffset(const struct tm* timeinfo)
{
#if defined(HAVE_TM_GMTOFF)
    return timeinfo->tm_gmtoff;
#elif defined(HAVE___TM_GMTOFF)
    return timeinfo->__tm_gmtoff;
#else
    return gmtOffset_m + (timeinfo->tm_isdst ? 3600 : 0);
#endif
}
