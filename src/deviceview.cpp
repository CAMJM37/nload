/*
 * nload
 * real time monitor for network traffic
 * Copyright (C) 2001 - 2018 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "deviceview.h"

#include "device.h"
#include "graph.h"
#include "setting.h"
#include "settingstore.h"
#include "stringutils.h"
#include "window.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

DeviceView::DeviceView(Device* device)
    : m_deviceNumber(0), m_totalNumberOfDevices(0), m_device(device)
{
  statsFile.open("statsFile.txt");
}

DeviceView::~DeviceView()
{
  statsFile.close();
}

void DeviceView::update()
{
    if(m_device->exists())
    {
        const Statistics& statistics = m_device->getStatistics();

        m_deviceGraphIn.update(statistics.getDataInPerSecond());
        m_deviceGraphOut.update(statistics.getDataOutPerSecond());
    }
    else
    {
        m_deviceGraphIn.resetTrafficData();
        m_deviceGraphOut.resetTrafficData();
    }
}

// print the device's data
void DeviceView::print(Window& window)
{
  // format statistics
  vector<string> statLinesIn;
  vector<string> statLinesOut;

  generateStatisticsIn(statLinesIn);
  generateStatisticsOut(statLinesOut);
  statsFile << statLinesOut[0] << " " << statLinesIn[0] << "\r" << std::endl;
}

// set the number identifying the device (for display only)
void DeviceView::setDeviceNumber(int deviceNumber)
{
    m_deviceNumber = deviceNumber;
}

// set the total number of shown devices (for display only)
void DeviceView::setTotalNumberOfDevices(int totalNumberOfDevices)
{
    m_totalNumberOfDevices = totalNumberOfDevices;
}

unsigned long long DeviceView::roundUpMaxDeflection(unsigned long long value)
{
    unsigned long long rounded = 2 * 1024; // 2 kByte/s
    while (rounded < value)
    {
        if((rounded << 1) < rounded)
            return value;

        rounded <<= 1;
    }

    return rounded;
}

string DeviceView::formatTrafficValue(unsigned long value, int precision)
{
    Statistics::dataUnit trafficFormat = (Statistics::dataUnit) ((int) SettingStore::get("TrafficFormat"));

    string unitString = Statistics::getUnitString(trafficFormat, value);
    float unitFactor = Statistics::getUnitFactor(trafficFormat, value);

    ostringstream oss;
    oss << fixed << setprecision(precision) << ((float) value / unitFactor) << " " << unitString << "/s";

    return oss.str();
}

string DeviceView::formatDataValue(unsigned long long value, int precision)
{
    Statistics::dataUnit dataFormat = (Statistics::dataUnit) ((int) SettingStore::get("DataFormat"));

    string unitString = Statistics::getUnitString(dataFormat, value);
    float unitFactor = Statistics::getUnitFactor(dataFormat, value);

    ostringstream oss;
    oss << fixed << setprecision(precision) << ((float) value / unitFactor) << " " << unitString << endl;

    return oss.str();
}

void DeviceView::generateStatisticsIn(vector<string>& statisticLines)
{
    const Statistics& statistics = m_device->getStatistics();

    statisticLines.push_back("CurIn: " + formatTrafficValue(statistics.getDataInPerSecond(), 2));
    statisticLines.push_back("Avg: " + formatTrafficValue(statistics.getDataInAverage(), 2));
    statisticLines.push_back("Min: " + formatTrafficValue(statistics.getDataInMin(), 2));
    statisticLines.push_back("Max: " + formatTrafficValue(statistics.getDataInMax(), 2));
    statisticLines.push_back("Ttl: " + formatDataValue(statistics.getDataInTotal(), 2));
}

void DeviceView::generateStatisticsOut(vector<string>& statisticLines)
{
    const Statistics& statistics = m_device->getStatistics();

    statisticLines.push_back("CurOut: " + formatTrafficValue(statistics.getDataOutPerSecond(), 2));
    statisticLines.push_back("Avg: " + formatTrafficValue(statistics.getDataOutAverage(), 2));
    statisticLines.push_back("Min: " + formatTrafficValue(statistics.getDataOutMin(), 2));
    statisticLines.push_back("Max: " + formatTrafficValue(statistics.getDataOutMax(), 2));
    statisticLines.push_back("Ttl: " + formatDataValue(statistics.getDataOutTotal(), 2));
}

void DeviceView::printStatistics(Window& window, const vector<string>& statisticLines, int x, int y)
{
    for(vector<string>::const_iterator itLine = statisticLines.begin(); itLine != statisticLines.end(); ++itLine)
    {
        window.print(x, y++) << *itLine;
    }
}

