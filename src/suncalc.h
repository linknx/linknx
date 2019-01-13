/*
    LinKNX KNX home automation platform
    Copyright (C) 2007 Jean-François Meessen <linknx@ouaye.net>
 
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

#ifndef SUNCALC_H
#define SUNCALC_H

#include <list>
#include <string>
#include "config.h"
#include "logger.h"
#include "objectcontroller.h"
#include "ruleserver.h"
#include "ticpp.h"

class SolarTimeSpec : public TimeSpec
{
public:
    SolarTimeSpec() {};
    virtual ~SolarTimeSpec();

protected:
    virtual double computeTime(double rise, double set) const = 0;
    virtual void getDayRaw(const tm &current, int &mday, int &mon, int &year, int &wdays) const;
    virtual void getTimeRaw(int mday, int mon, int year, int &min, int &hour) const;

private:
    static Logger& logger_m;
};

class SunriseTimeSpec : public SolarTimeSpec
{
public:
    virtual ~SunriseTimeSpec();
    virtual void exportXml(ticpp::Element* pConfig);
protected:
    virtual double computeTime(double rise, double set) const;

};

class SunsetTimeSpec : public SolarTimeSpec
{
public:
    virtual ~SunsetTimeSpec();
    virtual void exportXml(ticpp::Element* pConfig);
protected:
    virtual double computeTime(double rise, double set) const;

};

class SolarNoonTimeSpec : public SolarTimeSpec
{
public:
    virtual ~SolarNoonTimeSpec();
    virtual void exportXml(ticpp::Element* pConfig);
protected:
    virtual double computeTime(double rise, double set) const;

};

class SolarInfo
{
public:
    SolarInfo(struct tm * timeinfo);
    virtual ~SolarInfo();

    virtual bool getSunrise(int *min, int *hour);
    virtual bool getSunset(int *min, int *hour);
    virtual bool getNoon(int *min, int *hour);
private:
    bool get(double res, int *min, int *hour);
    double rise_m, set_m;
    int    rs_m;
	int year_m;
	int mon_m;
	int mday_m;
    long tz_offset_m;
    static Logger& logger_m;
};

class LocationInfo
{
public:
    LocationInfo();
    void importXml(ticpp::Element* pConfig);
    void exportXml(ticpp::Element* pConfig);
    void getCoord(double *lon, double *lat) { *lon = lon_m; *lat = lat_m; };
    void setCoord(double lon, double lat) { lon_m = lon; lat_m = lat; };
    long getGmtOffset();
    bool isEmpty() { return lon_m==0 && lat_m==0; };

protected:
    double lon_m, lat_m;
    long gmtOffset_m;
};

#endif
