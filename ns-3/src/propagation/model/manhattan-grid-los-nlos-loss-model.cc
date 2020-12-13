/*
 * manhattan-grid-los-nlos-loss-model.cc
 *
 *  Created on: Sep 7, 2017
 *      Author: thang
 */

#include "ns3/propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/pointer.h"
#include <cmath>
#include "manhattan-grid-los-nlos-loss-model.h"
#include <algorithm>
#include <math.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ManhattanLOSNLOSLossModel");

NS_OBJECT_ENSURE_REGISTERED (ManhattanLOSNLOSLossModel);
//Thang modifies according to B.1.2.1-1
TypeId
ManhattanLOSNLOSLossModel::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::ManhattanLOSNLOSLossModel")
    		.SetParent<PropagationLossModel> ()
    		.SetGroupName ("Propagation")
    		.AddConstructor<ManhattanLOSNLOSLossModel> ()
    		.AddAttribute("MinimumDistance","meter (m)",
    				DoubleValue(3),
    				MakeDoubleAccessor (&ManhattanLOSNLOSLossModel::SetMinimumDistance,&ManhattanLOSNLOSLossModel::GetMinimumDistance),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("Shadowing",
    				"dB",
    				DoubleValue (0),
    				MakeDoubleAccessor (&ManhattanLOSNLOSLossModel::SetShadowing,&ManhattanLOSNLOSLossModel::GetShadowing),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("Dist1",
    				"m",
    				DoubleValue (18),
    				MakeDoubleAccessor (&ManhattanLOSNLOSLossModel::SetDist1,&ManhattanLOSNLOSLossModel::GetDist1),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("Dist2",
    				"m",
    				DoubleValue (36),
    				MakeDoubleAccessor (&ManhattanLOSNLOSLossModel::SetDist2,&ManhattanLOSNLOSLossModel::GetDist2),
    				MakeDoubleChecker<double> ())
    				;

	return tid;
}

ManhattanLOSNLOSLossModel::ManhattanLOSNLOSLossModel ()
{
	//  m_shadowing = 10;
	rndGen = CreateObject<UniformRandomVariable>();
}


double
ManhattanLOSNLOSLossModel::GetMinimumDistance(void) const
{
	return m_minimumDistance;
}
void
ManhattanLOSNLOSLossModel::SetMinimumDistance (double minimumDistance)
{
	m_minimumDistance = minimumDistance;
}

double
ManhattanLOSNLOSLossModel::GetShadowing (void) const
{
	return m_shadowing;
}
void
ManhattanLOSNLOSLossModel::SetShadowing (double shadowing)
{
	m_shadowing = shadowing;
//	m_minimumDistance = 1000*pow10(-(147.4+m_shadowing)/43.3);
}

void
ManhattanLOSNLOSLossModel::SetDist1 (double dist1)
{
	m_dist1 = dist1;
}

double
ManhattanLOSNLOSLossModel::GetDist1 () const
{
	return m_dist1;
}

void
ManhattanLOSNLOSLossModel::SetDist2 (double dist2)
{
	m_dist2 = dist2;
}

double
ManhattanLOSNLOSLossModel::GetDist2 () const
{
	return m_dist2;
}

double
ManhattanLOSNLOSLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{//36.814 Appendix A

	NS_ASSERT(m_dist1 < m_dist2);
	double distance = a->GetDistanceFrom (b); //meter
	if (distance <= m_minimumDistance) //distance <= 3
	{
		return 0.0;
	}

	double prLOS = std::min((double)m_dist1/distance,(double)1)*(1-std::exp(-distance/m_dist2))+std::exp(-distance/m_dist2);
	double rndVal = rndGen->GetValue();
	double loss_in_db;
	if(rndVal <= prLOS)
	{
		//LOS path loss
		loss_in_db = 103.8+20.9*std::log10(distance * 1e-3);
	}
	else
	{
		//NLOS path loss
		loss_in_db = 145.4+37.5*std::log10(distance * 1e-3);
	}

	loss_in_db += m_shadowing;

	return (0 - loss_in_db);

}

double
ManhattanLOSNLOSLossModel::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	return txPowerDbm + GetLoss (a, b);
}

int64_t
ManhattanLOSNLOSLossModel::DoAssignStreams (int64_t stream)
{
	return 0;
}

}


