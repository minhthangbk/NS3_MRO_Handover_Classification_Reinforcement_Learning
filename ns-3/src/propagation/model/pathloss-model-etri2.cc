/*
 * pathloss-model-etri2.cc
 *
 *  Created on: Sep 12, 2017
 *      Author: thang
 */


#include "ns3/propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/pointer.h"
#include <cmath>
#include "pathloss-model-etri2.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PathlossModelETRI2");

NS_OBJECT_ENSURE_REGISTERED (PathlossModelETRI2);
//Thang modifies according to B.1.2.1-1
TypeId
PathlossModelETRI2::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::PathlossModelETRI2")
    		.SetParent<PropagationLossModel> ()
    		.SetGroupName ("Propagation")
    		.AddConstructor<PathlossModelETRI2> ()
    		.AddAttribute("MinimumDistance","meter (m)",
    				DoubleValue(3),
    				MakeDoubleAccessor (&PathlossModelETRI2::SetMinimumDistance,&PathlossModelETRI2::GetMinimumDistance),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("Shadowing",
    				"dB",
    				DoubleValue (0),
    				MakeDoubleAccessor (&PathlossModelETRI2::SetShadowing,&PathlossModelETRI2::GetShadowing),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("MaxLOSDistance", "max LOS distance (m)",
    		        DoubleValue (30),
    		        MakeDoubleAccessor (&PathlossModelETRI2::m_maxLOSDist),
    		        MakeDoubleChecker<double> ())
    		.AddAttribute ("ExtraLoss", "Extra Loss due to NLOS (dB)",
    		        DoubleValue (30),
    		        MakeDoubleAccessor (&PathlossModelETRI2::m_extraLoss),
    		        MakeDoubleChecker<double> ())
    		.AddAttribute ("AngleRange", "Angle Range (radiant)",
    		        DoubleValue (0.1),
    		        MakeDoubleAccessor (&PathlossModelETRI2::m_angleRange),
    		        MakeDoubleChecker<double> ())
    				;
	return tid;
}

PathlossModelETRI2::PathlossModelETRI2 ()
{
	//  m_shadowing = 10;
}


double
PathlossModelETRI2::GetMinimumDistance(void) const
{
	return m_minimumDistance;
}
void
PathlossModelETRI2::SetMinimumDistance (double minimumDistance)
{
	m_minimumDistance = minimumDistance;
}

double
PathlossModelETRI2::GetShadowing (void) const
{
	return m_shadowing;
}
void
PathlossModelETRI2::SetShadowing (double shadowing)
{
	m_shadowing = shadowing;
//	m_minimumDistance = 1000*pow10(-(147.4+m_shadowing)/43.3);
}


double
PathlossModelETRI2::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	double distance = a->GetDistanceFrom (b); //meter
	double absArrivalAngle = std::abs(a->GetAngleFrom(b));
	NS_ASSERT(m_minimumDistance < m_maxLOSDist);
	if (distance <= m_minimumDistance) //distance <= 3
	{
		return 0.0;
	}

	double loss_in_db = 147.4 + 43.3*std::log10(distance * 1e-3) + m_shadowing;
	if(((absArrivalAngle>m_angleRange)&&(absArrivalAngle<(1.57-m_angleRange))) ||
			((absArrivalAngle>(1.57+m_angleRange))&&(absArrivalAngle<(3.14-m_angleRange)))
			)
	{
		loss_in_db += m_extraLoss;
	}

	return (0 - loss_in_db);

}

double
PathlossModelETRI2::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	return txPowerDbm + GetLoss (a, b);
}

int64_t
PathlossModelETRI2::DoAssignStreams (int64_t stream)
{
	return 0;
}

}

