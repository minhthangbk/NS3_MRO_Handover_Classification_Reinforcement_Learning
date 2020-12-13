/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 CTTC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */


#include <ns3/log.h>
#include <ns3/double.h>
#include <cmath>

#include "antenna-model.h"
#include "three-sectors-parabolic-antenna-model.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ThreeSectorsParabolicAntennaModel");

NS_OBJECT_ENSURE_REGISTERED (ThreeSectorsParabolicAntennaModel);


TypeId 
ThreeSectorsParabolicAntennaModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ThreeSectorsParabolicAntennaModel")
    .SetParent<AntennaModel> ()
    .SetGroupName("Antenna")
    .AddConstructor<ThreeSectorsParabolicAntennaModel> ()
    .AddAttribute ("Beamwidth",
                   "The 3dB beamwidth (degrees)",
                   DoubleValue (60),
                   MakeDoubleAccessor (&ThreeSectorsParabolicAntennaModel::SetBeamwidth,
                                       &ThreeSectorsParabolicAntennaModel::GetBeamwidth),
                   MakeDoubleChecker<double> (0, 180))
    .AddAttribute ("Orientation",
                   "The angle (degrees) that expresses the orientation of the antenna on the x-y plane relative to the x axis",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&ThreeSectorsParabolicAntennaModel::SetOrientation,
                                       &ThreeSectorsParabolicAntennaModel::GetOrientation),
                   MakeDoubleChecker<double> (-360, 360))
    .AddAttribute ("MaxAttenuation",
                   "The maximum attenuation (dB) of the antenna radiation pattern.",
                   DoubleValue (20.0),
                   MakeDoubleAccessor (&ThreeSectorsParabolicAntennaModel::m_maxAttenuation),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void 
ThreeSectorsParabolicAntennaModel::SetBeamwidth (double beamwidthDegrees)
{ 
  NS_LOG_FUNCTION (this << beamwidthDegrees);
  m_beamwidthRadians = DegreesToRadians (beamwidthDegrees);
}

double
ThreeSectorsParabolicAntennaModel::GetBeamwidth () const
{
  return RadiansToDegrees (m_beamwidthRadians);
}

void 
ThreeSectorsParabolicAntennaModel::SetOrientation (double orientationDegrees)
{
  NS_LOG_FUNCTION (this << orientationDegrees);
  m_orientationRadians = DegreesToRadians (orientationDegrees);
}

double
ThreeSectorsParabolicAntennaModel::GetOrientation () const
{
  return RadiansToDegrees (m_orientationRadians);
}

double 
ThreeSectorsParabolicAntennaModel::GetGainDb (Angles a)
{
  NS_LOG_FUNCTION (this << a);
  // azimuth angle w.r.t. the reference system of the antenna
//  double phi = a.phi - m_orientationRadians;

  double phi1 = a.phi;
  double phi2 = a.phi-DegreesToRadians(120);
  double phi3 = a.phi-DegreesToRadians(-120);
  // make sure phi is in (-pi, pi]
  while (phi1 <= -M_PI)
    {
      phi1 += M_PI+M_PI;
    }
  while (phi1 > M_PI)
    {
      phi1 -= M_PI+M_PI;
    }

  while (phi2 <= -M_PI)
    {
       phi2 += M_PI+M_PI;
    }
  while (phi2 > M_PI){
      phi2 -= M_PI+M_PI;
  }

  while (phi3 <= -M_PI){
     phi3 += M_PI+M_PI;
  }
  while (phi3 > M_PI){
     phi3 -= M_PI+M_PI;
  }

  NS_LOG_LOGIC ("phi1 = " << phi1 );

  double gainDb1 = -std::min (12 * pow (phi1 / m_beamwidthRadians, 2), m_maxAttenuation);
  double gainDb2 = -std::min (12 * pow (phi2 / m_beamwidthRadians, 2), gainDb1);
  double gainDb3 = -std::min (12 * pow (phi3 / m_beamwidthRadians, 2), gainDb2);

  NS_LOG_LOGIC ("gain = " << gainDb3);
  return gainDb3;
}


}

