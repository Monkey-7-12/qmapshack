/**********************************************************************************************
    Copyright (C) 2015-2016 Christian Eichler code@christian-eichler.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#include "gis/trk/CKnownExtension.h"
#include "units/IUnit.h"
#include <QStringBuilder>

const QString CKnownExtension::internalSlope    = "::ql:slope";
const QString CKnownExtension::internalSpeed    = "::ql:speed";
const QString CKnownExtension::internalEle      = "::ql:ele";
const QString CKnownExtension::internalProgress = "::ql:progress";

QHash<QString, CKnownExtension> CKnownExtension::knownExtensions;

static fTrkPtGetVal getExtensionValueFunc(const QString ext)
{
    return [ext](const CGisItemTrk::trkpt_t &p)
           {
               bool ok;
               qreal val = p.extensions.value(ext).toReal(&ok);
               return ok ? val : NOFLOAT;
           };
}

void CKnownExtension::initGarminTPXv1(IUnit &units, const QString &ns)
{
    // support for the Garmin TrackPointExtension v1
    //  https://www8.garmin.com/xmlschemas/TrackPointExtensionv1.xsd
    knownExtensions.insert(ns % ":TrackPointExtension|" % ns % ":atemp",
                           { tr("Air Temperature"), 0, -100., 100., 1., "°C", "://icons/32x32/CSrcATemp.png", true, false,
                             getExtensionValueFunc(ns % ":TrackPointExtension|" % ns % ":atemp")});

    knownExtensions.insert(ns % ":TrackPointExtension|" % ns % ":wtemp",
                           { tr("Water Temperature"), 1, -100., 100., 1., "°C", "://icons/32x32/CSrcWTemp.png", true, false,
                             getExtensionValueFunc(ns % ":TrackPointExtension|" % ns % ":wtemp")});

    knownExtensions.insert(ns % ":TrackPointExtension|" % ns % ":depth",
                           { tr("Depth"), 2, 0., 12000., units.basefactor, units.baseunit, "://icons/32x32/CSrcDepth.png", true, false,
                             getExtensionValueFunc(ns % ":TrackPointExtension|" % ns % ":depth")});

    knownExtensions.insert(ns % ":TrackPointExtension|" % ns % ":hr",
                           { tr("Heart Rate"), 3, 0., 300., 1., "bpm", "://icons/32x32/CSrcHR.png", true, false,
                             getExtensionValueFunc(ns % ":TrackPointExtension|" % ns % ":hr")});

    knownExtensions.insert(ns % ":TrackPointExtension|" % ns % ":cad",
                           { tr("Cadence"), 4, 0., 500., 1., "rpm", "://icons/32x32/CSrcCAD.png", true, false,
                             getExtensionValueFunc(ns % ":TrackPointExtension|" % ns % ":cad")});
}

void CKnownExtension::initMioTPX(IUnit &units)
{
    // support for extensions used by MIO Cyclo ver. 4.2 (who needs xml namespaces?!)
    knownExtensions.insert("heartrate",
                           { tr("Heart Rate"), std::numeric_limits<int>::max(), 0., 300., 1., "bpm", "://icons/32x32/CSrcHR.png", true, false,
                             getExtensionValueFunc("heartrate")});

    knownExtensions.insert("cadence",
                           { tr("Cadence"), std::numeric_limits<int>::max(), 0., 500., 1., "rpm", "://icons/32x32/CSrcCAD.png", true, false,
                             getExtensionValueFunc("cadence")});

    knownExtensions.insert("speed",
                           { tr("Speed"), std::numeric_limits<int>::max(), 0., 600., units.speedfactor, units.speedunit, "://icons/32x32/CSrcSpeed.png", true, false,
                             getExtensionValueFunc("speed")});

    knownExtensions.insert("acceleration",
                           { tr("Acceleration"), std::numeric_limits<int>::max(), std::numeric_limits<qreal>::lowest(), std::numeric_limits<qreal>::max(), units.basefactor, units.baseunit + "/s²", "://icons/32x32/CSrcAccel.png", true, false,
                             getExtensionValueFunc("acceleration")});

    knownExtensions.insert("course",
                           { tr("Course"), std::numeric_limits<int>::max(), -3.2, 3.2, 1., "rad", "://icons/32x32/CSrcCourse.png", true, false,
                             getExtensionValueFunc("course")});
}

void CKnownExtension::init(IUnit &units)
{
    knownExtensions =
    {
        {internalSlope,
         { tr("Slope*"), -1, -90., 90., 1., "°", "://icons/32x32/CSrcSlope.png", true, true,
           [](const CGisItemTrk::trkpt_t &p) { return p.slope1; }}
        },

        {internalSpeed,
         { tr("Speed*"), -1, 0., 600., units.speedfactor, units.speedunit, "://icons/32x32/CSrcSpeed.png", true, true,
           [](const CGisItemTrk::trkpt_t &p) { return p.speed; }}
        },

        {internalEle,
         { tr("Elevation"), -1, 0., 100000., units.basefactor, units.baseunit, "://icons/32x32/CSrcElevation.png", true, true,
           [](const CGisItemTrk::trkpt_t &p) { return (NOINT == p.ele) ? NOFLOAT : p.ele; }}
        },

        {internalProgress,
         { tr("Progress"), -1, 0., NOFLOAT, units.basefactor, units.baseunit, "://icons/32x32/Progress.png", true, true,
           [](const CGisItemTrk::trkpt_t &p) { return p.distance; }}
        }
    };

    initGarminTPXv1(units, "gpxtpx");
    initGarminTPXv1(units, "tp1");

    initMioTPX(units);
}

const CKnownExtension CKnownExtension::get(const QString &name)
{
    CKnownExtension def("", std::numeric_limits<int>::max(), -100000., 100000., 1., "", "://icons/32x32/CSrcUnknown.png", false, true,
                        getExtensionValueFunc(name)
                        );
    return knownExtensions.value(name, def);
}

bool CKnownExtension::isKnown(const QString &name)
{
    return knownExtensions.contains(name);
}
