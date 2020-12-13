/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

#include "ns3/propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/pointer.h"
#include <cmath>
#include "cost231-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Cost231PropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (Cost231PropagationLossModel);
//Thang modifies according to B.1.2.1-1
TypeId
Cost231PropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Cost231PropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<Cost231PropagationLossModel> ()
    .AddAttribute ("Lambda",
                   "The wavelength  (default is 1.8 GHz at 300 000 km/s).",
                   DoubleValue (300000000.0 / 1.8e9), //DoubleValue (300000000.0 / 2.3e9), //THANG modified
                   MakeDoubleAccessor (&Cost231PropagationLossModel::m_lambda),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Frequency",
                   "The Frequency  (default is 1.8 GHz).",
                   DoubleValue (1.8e9), //DoubleValue (2.3e9), //THANG modified
                   MakeDoubleAccessor (&Cost231PropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BSAntennaHeight",
                   "BS Antenna Height (default is 5m).",
                   DoubleValue (5),//DoubleValue (50.0), //THANG modified
                   MakeDoubleAccessor (&Cost231PropagationLossModel::m_BSAntennaHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SSAntennaHeight",
                   "SS Antenna Height (default is 1.5m).",
                   DoubleValue (1.5), //DoubleValue (3), //THANG modified
                   MakeDoubleAccessor (&Cost231PropagationLossModel::m_SSAntennaHeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinDistance",
                   "The distance under which the propagation model refuses to give results (m) ",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&Cost231PropagationLossModel::SetMinDistance, &Cost231PropagationLossModel::GetMinDistance),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("Stream",
					  "Set Stream for RVs ",
					  IntegerValue (1),
					  MakeIntegerAccessor (&Cost231PropagationLossModel::SetStream, &Cost231PropagationLossModel::GetStream),
					  MakeIntegerChecker<int64_t> ())
                   ;

  return tid;
}

Cost231PropagationLossModel::Cost231PropagationLossModel ()
{
  m_shadowing = 10;

  m_randomShadowing = CreateObject<NormalRandomVariable>();
  m_randomShadowing->SetAttribute("Variance",DoubleValue(100)); //sigma == 10 for urban areas
  m_randomShadowing->SetAttribute("Bound", DoubleValue(20));// +- 2*sigma, 95%
//  /*
//   * THANG
//   * change for new path loss
//   */
//  m_shadowing = 3;
}

void
Cost231PropagationLossModel::SetLambda (double frequency, double speed)
{
  m_lambda = speed / frequency;
  m_frequency = frequency;
}

double
Cost231PropagationLossModel::GetShadowing (void)
{
  return m_shadowing;
}
void
Cost231PropagationLossModel::SetShadowing (double shadowing)
{
  m_shadowing = shadowing;
}

void
Cost231PropagationLossModel::SetLambda (double lambda)
{
  m_lambda = lambda;
  m_frequency = 300000000 / lambda;
}

double
Cost231PropagationLossModel::GetLambda (void) const
{
  return m_lambda;
}

void
Cost231PropagationLossModel::SetMinDistance (double minDistance)
{
  m_minDistance = minDistance;
}
double
Cost231PropagationLossModel::GetMinDistance (void) const
{
  return m_minDistance;
}

void
Cost231PropagationLossModel::SetBSAntennaHeight (double height)
{
  m_BSAntennaHeight = height;
}

double
Cost231PropagationLossModel::GetBSAntennaHeight (void) const
{
  return m_BSAntennaHeight;
}

void
Cost231PropagationLossModel::SetSSAntennaHeight (double height)
{
  m_SSAntennaHeight = height;
}

double
Cost231PropagationLossModel::GetSSAntennaHeight (void) const
{
  return m_SSAntennaHeight;
}

double
Cost231PropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

  double distance = a->GetDistanceFrom (b); //meter
  if (distance <= m_minDistance)
    {
      return 0.0;
    }

  double frequency_MHz = m_frequency * 1e-6;//1800Mhz*10^-9 = 1800 (Mhz)

  double distance_km = distance * 1e-3;//km

  double C_H = 0.8 + ((1.11 * std::log10(frequency_MHz)) - 0.7) * m_SSAntennaHeight - (1.56 * std::log10(frequency_MHz));

  // from the COST231 wiki entry
  // See also http://www.lx.it.pt/cost231/final_report.htm
  // Ch. 4, eq. 4.4.3, pg. 135

  double loss_in_db = 46.3 + (33.9 * std::log10(frequency_MHz)) - (13.82 * std::log10 (m_BSAntennaHeight)) - C_H
		  	  	  + ((44.9 - 6.55 * std::log10 (m_BSAntennaHeight)) * std::log10 (distance_km)) + m_shadowing;

//  double loss_in_db = 46.3 + (33.9 * std::log10(frequency_MHz)) - (13.82 * std::log10 (m_BSAntennaHeight)) - C_H
//  		  	  	  + ((44.9 - 6.55 * std::log10 (m_BSAntennaHeight)) * std::log10 (distance_km)) + m_randomShadowing->GetValue();

  return (0 - loss_in_db);

}

double
Cost231PropagationLossModel::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return txPowerDbm + GetLoss (a, b);
}

int64_t
Cost231PropagationLossModel::DoAssignStreams (int64_t stream)
{
	NS_LOG_UNCOND("Cost231PropagationLossModel::DoAssignStreams StreamId " << stream);
	m_randomShadowing->SetStream(stream);//Add by THANG
    return 0;
}
/*
 * Added by THANG
 */
void
Cost231PropagationLossModel::SetStream(int64_t stream){
	NS_LOG_UNCOND("Cost231PropagationLossModel::SetStream StreamId " << stream);
	m_randomShadowing->SetStream(stream);//Add by THANG
}

int64_t
Cost231PropagationLossModel::GetStream() const{
	return m_randomShadowing->GetStream();
}


}
