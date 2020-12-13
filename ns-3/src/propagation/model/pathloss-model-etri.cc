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
#include "pathloss-model-etri.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PathlossModelETRI");

NS_OBJECT_ENSURE_REGISTERED (PathlossModelETRI);
//Thang modifies according to B.1.2.1-1
TypeId
PathlossModelETRI::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::PathlossModelETRI")
    		.SetParent<PropagationLossModel> ()
    		.SetGroupName ("Propagation")
    		.AddConstructor<PathlossModelETRI> ()
    		.AddAttribute("MinimumDistance","meter (m)",
    				DoubleValue(3),
    				MakeDoubleAccessor (&PathlossModelETRI::SetMinimumDistance,&PathlossModelETRI::GetMinimumDistance),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("Shadowing",
    				"dB",
    				DoubleValue (0),
    				MakeDoubleAccessor (&PathlossModelETRI::SetShadowing,&PathlossModelETRI::GetShadowing),
    				MakeDoubleChecker<double> ())
    				;

	return tid;
}

PathlossModelETRI::PathlossModelETRI ()
{
	//  m_shadowing = 10;
}


double
PathlossModelETRI::GetMinimumDistance(void) const
{
	return m_minimumDistance;
}
void
PathlossModelETRI::SetMinimumDistance (double minimumDistance)
{
	m_minimumDistance = minimumDistance;
}

double
PathlossModelETRI::GetShadowing (void) const
{
	return m_shadowing;
}
void
PathlossModelETRI::SetShadowing (double shadowing)
{
	m_shadowing = shadowing;
//	m_minimumDistance = 1000*pow10(-(147.4+m_shadowing)/43.3);
}


double
PathlossModelETRI::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

	double distance = a->GetDistanceFrom (b); //meter
	if (distance <= m_minimumDistance) //distance <= 3
	{
		return 0.0;
	}

	double loss_in_db = 147.4 + 43.3*std::log10(distance * 1e-3) + m_shadowing;

	return (0 - loss_in_db);

}

double
PathlossModelETRI::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	return txPowerDbm + GetLoss (a, b);
}

int64_t
PathlossModelETRI::DoAssignStreams (int64_t stream)
{
	return 0;
}

}
