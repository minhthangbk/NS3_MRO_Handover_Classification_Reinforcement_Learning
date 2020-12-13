/*
 * manhattan-grid-mobility-model.cc
 *
 *  Created on: Apr 28, 2017
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "manhattan-grid-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ManhattanGridMobilityModel);

TypeId
ManhattanGridMobilityModel::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::ManhattanGridMobilityModel")
	    .SetParent<MobilityModel> ()
	    .SetGroupName ("Mobility")
	    .AddConstructor<ManhattanGridMobilityModel> ()
	    .AddAttribute ("MinSpeed",
	                   "Minimum speed value, [m/s]",
	                   DoubleValue (3),
	                   MakeDoubleAccessor (&ManhattanGridMobilityModel::m_minSpeed),
	                   MakeDoubleChecker<double> ())
	    .AddAttribute ("MaxSpeed",
	                   "Maximum speed value, [m/s]",
	                   DoubleValue (5),
	                   MakeDoubleAccessor (&ManhattanGridMobilityModel::m_maxSpeed),
	                   MakeDoubleChecker<double> ())
	    .AddAttribute ("XBlocks",
	                   "Number of blocks on x-axis",
					   UintegerValue (5),
					   MakeUintegerAccessor (&ManhattanGridMobilityModel::m_xBlocks),
					   MakeUintegerChecker<uint8_t> ())
	   .AddAttribute ("YBlocks",
					  "Number of blocks on y-axis",
					  UintegerValue (5),
					  MakeUintegerAccessor (&ManhattanGridMobilityModel::m_yBlocks),
					  MakeUintegerChecker<uint8_t> ())
	   .AddAttribute ("TurnProbability",
				  	  "Turn Probability",
					  DoubleValue (0.5),
					  MakeDoubleAccessor (&ManhattanGridMobilityModel::m_turnProb),
					  MakeDoubleChecker<double> ())
	   .AddAttribute ("UpdateDist",
				"minimum Y, [m]",
				DoubleValue (2),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_updateDist),
				MakeDoubleChecker<double> ())
		.AddAttribute ("SpeedChangeProbability",
		     	"Speed Change Probability",
				DoubleValue (0.5),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_speedChangePro),
				MakeDoubleChecker<double> ())
		.AddAttribute ("PauseProbability",
				"Pause Probability",
				DoubleValue (0.5),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_pausePro),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MaxPause",
				"Maximum pause time [s].",
				DoubleValue (0),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_maxPause),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MinPause",
				"Minimum pause time [s].",
				DoubleValue (0),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_minPause),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MaxX",
				"Maximum of X",
				DoubleValue (100),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_xMax),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MinX",
				"Minimum of X",
				DoubleValue (0),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_xMin),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MaxY",
				"Maximum of Y",
				DoubleValue (100),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_yMax),
				MakeDoubleChecker<double> ())
		.AddAttribute ("MinY",
				"Minimum of Y",
				DoubleValue (0),
				MakeDoubleAccessor (&ManhattanGridMobilityModel::m_yMin),
				MakeDoubleChecker<double> ())
	    ;

	  return tid;
}

ManhattanGridMobilityModel::ManhattanGridMobilityModel()
{
//	m_speedRV = CreateObject<UniformRandomVariable>();
//	m_speedChangeRV = CreateObject<UniformRandomVariable>();
//	m_turnRV = CreateObject<UniformRandomVariable>();
	m_rndGen = CreateObject<UniformRandomVariable>();
	m_curDir = 0; //0: Up; 1: Down; 2: Right; 3: Left
//	m_event = 0;
	m_alreadyStarted =false;
	m_gridDist = 0;
}

void
ManhattanGridMobilityModel::DoInitialize(void)
{
	NS_ASSERT(m_xMax > m_xMin);
	NS_ASSERT(m_yMax > m_yMin);
	NS_ASSERT((m_yBlocks >= 1) && (m_xBlocks >= 1));

	m_xDim = (m_xMax-m_xMin)/(double)m_xBlocks;
	m_yDim = (m_yMax-m_yMin)/(double)m_yBlocks;

	m_helper.Update();
	m_helper.Pause();

	double init_xh = (m_xMax-m_xMin)*(double)(m_xBlocks+1);
	double init_xr = init_xh/(init_xh+(m_yMax-m_yMin)*(double)(m_yBlocks+1));
	double xInit;
	double yInit;
	if(init_xr > m_rndGen->GetValue())
	{//First move along x
		xInit = (m_rndGen->GetValue())*(m_xMax-m_xMin)+m_xMin;
		yInit = (uint16_t)((m_rndGen->GetValue())*(double)(m_yBlocks+1))*m_yDim+m_yMin;
		m_curDir = (uint8_t)(m_rndGen->GetValue() * 2.) + 2; //always in [2,3]
		m_gridDist = (xInit-m_xMin)-(double)((uint16_t)((xInit-m_xMin)/m_xDim)*m_xDim);
		if(m_curDir == 2)//If move right
		{
			m_gridDist = m_xDim-m_gridDist;
		}
	}
	else
	{//First move along y
		xInit = (uint16_t)((m_rndGen->GetValue())*(double)(m_xBlocks+1))*m_xDim+m_xMin;
		yInit = (m_rndGen->GetValue())*(m_yMax-m_yMin)+m_yMin;
		m_curDir = (uint8_t)(m_rndGen->GetValue() * 2.); //always <= 3
		m_gridDist = (yInit-m_yMin)-(double)((uint16_t)((yInit-m_yMin)/m_yDim)*m_yDim);
		if(m_curDir == 0)//If move Up
		{
			m_gridDist = m_yDim-m_gridDist;
		}
	}

	m_helper.SetPosition(Vector(xInit,yInit,0));
	m_dist = m_updateDist;
	m_curSpeed = m_rndGen->GetValue(m_minSpeed, m_maxSpeed);

	NS_ASSERT(!m_event.IsRunning());
	m_event = Simulator::ScheduleNow(&ManhattanGridMobilityModel::BeginWalk, this);
	m_alreadyStarted = true;
	NotifyCourseChange();
	MobilityModel::DoInitialize();
//	NS_LOG_FUNCTION("ManhattanGridMobilityModel::DoInitialize "
////			<< " m_xMax: " << m_xMax << " m_xMin: " << m_xMin << " m_xBlocks: " << (uint16_t)m_xBlocks
////			<< " m_yMax: " << m_yMax << " m_yMin: " << m_yMin << " m_yBlocks: " << (uint16_t)m_yBlocks
//			<<" m_xDim: " << m_xDim << " m_yDim: " << m_yDim
//			<< " xInit: " << xInit << " yInit: " << yInit
//			<< " m_curDir: " << (uint16_t)m_curDir << " m_gridDist: " << m_gridDist << " m_dist: "
//			<< m_dist << " m_curSpeed: " << m_curSpeed);
}

//void
//ManhattanGridMobilityModel::Start()
//{
//
//}

void
ManhattanGridMobilityModel::BeginWalk()
{
	m_helper.Update();
	m_helper.Pause();
	Vector curPos = m_helper.GetCurrentPosition();

	//Get new position
	Vector dst = GetDestination(curPos,m_dist,m_curDir);
	bool exactlyHitBoundary = (MustTurn(dst, m_curDir) || (m_gridDist == 0));
	bool dstOutOfBound = OutOfBounds(dst);
	bool turnAtCrossroads = ((m_gridDist <= m_dist)//going to reach a crossroad
			&& (m_rndGen->GetValue() < m_turnProb));
	double travelTime;

	if(exactlyHitBoundary || turnAtCrossroads || dstOutOfBound)
	{//turn is decided
//		if(dstOutOfBound)
//		{
//			NS_LOG_FUNCTION("BeginWalk (OutOfBound) NodeId: " << GetNodeId() <<
//					" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir
//					<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//					<< " dst.x: " << dst.x << " dst.y: " << dst.y);
//		}
//		else
//		if(exactlyHitBoundary)
//		{
//			NS_LOG_FUNCTION("BeginWalk (exactlyHit) NodeId: " << GetNodeId() <<
//					" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir
//					<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//					<< " dst.x: " << dst.x << " dst.y: " << dst.y);
//		}
//		else if(turnAtCrossroads)
//		{
//			NS_LOG_FUNCTION("BeginWalk (turnAtCrossroads) NodeId: " << GetNodeId() <<
//					" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir
//					<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//					<< " dst.x: " << dst.x << " dst.y: " << dst.y);
//		}

		double mdist;
		if(exactlyHitBoundary)
		{
			mdist = m_dist;
			m_dist = m_updateDist;
		}
		else
		{//turnAtCrossroads
			mdist = m_gridDist;
			if(m_dist > mdist)
			{
				m_dist -= mdist;
			}
			else
			{
				m_dist = m_updateDist;
			}
		}
		NS_ASSERT((mdist > 0));// && (m_gridDist > 0));
		travelTime = mdist/m_curSpeed;
		dst = AlignPosition(GetDestination(curPos,mdist,m_curDir));

		if(m_curDir < 2)
		{//moving along y
			if(dst.x > m_xMin)
			{
				if(dst.x < m_xMax)
				{// x in [xMin, xMax] -> choose random
					m_curDir = (uint8_t)(m_rndGen->GetValue() * 2.) + 2; //always in [2,3] (move right or left)
				}
				else
				{//> yMax
					m_curDir = 3; //force to move left
				}
			}
			else
			{
				m_curDir = 2; //force to move right
			}
		}
		else if (dst.y > m_yMin)
		{//moving along x
			if(dst.y < m_yMax)
			{// in range [yMin, yMax]
				m_curDir = (uint8_t)(m_rndGen->GetValue() * 2.);//always in [0,1] (move up or down)
			}
			else
			{//out of range ( > yMax)
				m_curDir = 1; //force to move down
			}
		}
		else
		{//out of range ( < yMin)
			m_curDir = 0; //force to move up
		}

		if(m_curDir < 2)
		{//if moving along y
			m_gridDist = m_yDim;
		}
		else
		{//if moving along x
			m_gridDist = m_xDim;
		}

		double dx = dst.x-curPos.x;
		double dy = dst.y-curPos.y;
		double dz = dst.z-curPos.z;
		double k = m_curSpeed/std::sqrt(dx*dx+dy*dy+dz*dz);

		m_helper.SetVelocity(Vector(k*dx, k*dy, k*dz));
		m_helper.Unpause();
//		NS_LOG_FUNCTION("BeginWalk (turn) NodeId: " << GetNodeId() <<
//				" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir <<" travelTime: " << travelTime
//				<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//				<< " dst.x: " << dst.x << " dst.y: " << dst.y) ;
		m_event = Simulator::Schedule (Seconds(travelTime),&ManhattanGridMobilityModel::BeginWalk, this);
	}
	else
	{//not turn
		double rndVal = m_rndGen->GetValue();
		if(rndVal > m_pausePro)
		{//keep moving
			double mdist = m_dist;

			m_gridDist -= m_dist;//Update current grid distance
			m_dist = m_updateDist;//Reset the update distance
			if(m_gridDist < 0)//
			{
				if(m_curDir < 2)
				{
					m_gridDist += m_yDim;
				}
				else
				{
					m_gridDist += m_xDim;
				}
			}
			//		double rndVal = m_rndGen->GetValue();

			if(rndVal < m_speedChangePro)
			{//same speed
				travelTime = mdist / m_curSpeed;

				double dx = dst.x-curPos.x;
				double dy = dst.y-curPos.y;
				double dz = dst.z-curPos.z;
				double k = m_curSpeed/std::sqrt(dx*dx+dy*dy+dz*dz);

				m_helper.SetVelocity(Vector(k*dx, k*dy, k*dz));
				m_helper.Unpause();
//				NS_LOG_FUNCTION("BeginWalk (keep moving+keep speed) NodeId: " << GetNodeId() <<
//						" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir<<" travelTime: " << travelTime
//						<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//						<< " dst.x: " << dst.x << " dst.y: " << dst.y);
				m_event = Simulator::Schedule (Seconds(travelTime),&ManhattanGridMobilityModel::BeginWalk, this);
			}
			else
			{//change speed
				m_curSpeed = m_rndGen->GetValue(m_minSpeed, m_maxSpeed);
				travelTime = mdist / m_curSpeed;

				double dx = dst.x-curPos.x;
				double dy = dst.y-curPos.y;
				double dz = dst.z-curPos.z;
				double k = m_curSpeed/std::sqrt(dx*dx+dy*dy+dz*dz);

				m_helper.SetVelocity(Vector(k*dx, k*dy, k*dz));
				m_helper.Unpause();
//				NS_LOG_FUNCTION("BeginWalk (keep moving+change speed) NodeId: " << GetNodeId() <<
//						" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir<<" travelTime: " << travelTime
//						<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//						<< " dst.x: " << dst.x << " dst.y: " << dst.y);
				m_event = Simulator::Schedule (Seconds(travelTime),&ManhattanGridMobilityModel::BeginWalk, this);
			}
		}
		else
		{//paused for a while
			//			m_helper.Pause();
			double pauseDuration = m_rndGen->GetValue(m_minPause, m_maxPause);
//			NS_LOG_FUNCTION("BeginWalk (pause) NodeId: " << GetNodeId() <<" pauseDuration: " << pauseDuration <<
//					" m_dist: " << m_dist << " m_gridDist: " << m_gridDist << " m_curDir: " << (uint16_t)m_curDir
//					<< " pos.x: " << curPos.x << " pos.y: " << curPos.y
//					<< " dst.x: " << dst.x << " dst.y: " << dst.y);
			m_event = Simulator::Schedule (Seconds(pauseDuration), &ManhattanGridMobilityModel::BeginWalk, this);
		}
	}
	NotifyCourseChange();
}

bool
ManhattanGridMobilityModel::OutOfBounds(Vector pos)
{
	return ((pos.x < m_xMin) ||
			(pos.x > m_xMax) ||
			(pos.y < m_yMin) ||
			(pos.y > m_yMax));
}

bool
ManhattanGridMobilityModel::MustTurn(Vector pos, uint8_t dir)
{
	return (((dir == 0) && (pos.y == m_yMax)) //moving up
			|| ((dir == 1) && (pos.y == m_yMin)) //moving down
			|| ((dir == 2) && (pos.x == m_xMax)) //moving right
			|| ((dir == 3) && (pos.x == m_xMin))); //moving left
}

Vector
ManhattanGridMobilityModel::AlignPosition(Vector pos)
{//Rearrange the position by approximation
	Vector newPos;
	newPos.x = (double)((uint16_t)((pos.x-m_xMin)/m_xDim + 0.5))*m_xDim+m_xMin;
	newPos.y = (double)((uint16_t)((pos.y-m_yMin)/m_yDim + 0.5))*m_yDim+m_yMin;
	newPos.z = 0;
	return newPos;
}

Vector
ManhattanGridMobilityModel::GetDestination(Vector pos, double dist, uint8_t dir)
{
	Vector dst;
	dst.z = 0;
	switch(dir)
	{
	case 0://Up
	{
		dst.x = pos.x;
		dst.y = pos.y+dist;
	}
	break;

	case 1://Down
	{
		dst.x = pos.x;
		dst.y = pos.y-dist;
	}
	break;

	case 2://Right
	{
		dst.x = pos.x+dist;
		dst.y = pos.y;
	}
	break;

	case 3://Left
	{
		dst.x = pos.x-dist;
		dst.y = pos.y;
	}
	break;
	}
	return dst;
}

Vector
ManhattanGridMobilityModel::DoGetPosition() const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}

void
ManhattanGridMobilityModel::DoSetPosition (const Vector &position)
{

}

Vector
ManhattanGridMobilityModel::DoGetVelocity() const
{
	return m_helper.GetVelocity();
}

int64_t
ManhattanGridMobilityModel::DoAssignStreams (int64_t stream)
{
//	m_speedRV->SetStream(stream);
//	m_speedChangeRV->SetStream(stream+1);
//	m_turnRV->SetStream(stream+2);
	m_rndGen->SetStream(stream);
	return 1;
}

}//End NS3 Namespace


