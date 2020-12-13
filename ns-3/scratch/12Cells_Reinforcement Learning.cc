#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store-module.h>
#include <ns3/buildings-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/applications-module.h>
#include <ns3/log.h>
#include <iomanip>
#include <ios>
#include <string>
#include <vector>

#include <ns3/mobility-building-info.h>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/building.h>
#include <ns3/propagation-environment.h>
#include <ns3/enum.h>
#include "ns3/random-variable-stream.h"
#include <ns3/lte-enb-dson-sap.h>
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/ptr.h"
#include <ns3/son-common.h>
#include <ns3/lte-enb-rrc.h>
#include "ns3/simulator.h"
#include "ns3/realtime-simulator-impl.h"
#include "ns3/nstime.h"
#include "ns3/system-thread.h"
#include "ns3/config.h"
#include "ns3/global-value.h"
#include <unistd.h>
#include <sys/time.h>
#include <ns3/system-mutex.h> //Mutex for Pthread
#include <set>

#include <algorithm>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MROSimulation");

class CSONMroAlgorithm : public Object{
public:
	CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap);
	CSONMroAlgorithm();
	void SetParams(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap);
	virtual ~CSONMroAlgorithm();
	void MroAlgorithm();
	void UpdateMroStatisticalData(uint16_t cellId, SONCommon::MROProblemInfo probInfo);
	void SaveKPI2Log(double curTime, uint16_t cellId, uint16_t loh, uint16_t eho, uint16_t who, uint16_t pho, uint16_t total, double kpi);
	void SaveRlfPP2Log(double curTime, double rrr, double rr);
	void SaveRR2Log(double curTime, double consRR);
	void TimeSlideWindowStart();
	void UpdateRR(double time, bool rlf);

	struct RLFTiming{
		bool rlf;
		double time;
	};
	struct RRInfo{
		std::queue<RLFTiming> rlfHistory;
		uint32_t aggNumIssues; //aggregated
		uint32_t aggnumHO;
	};

	NetDeviceContainer m_smallcellEnbDevs;//Set of eNB
	std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> m_paramsMap; //mapping between cellId-MroParam
	//	std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> m_prevParamsMap1; //Save the previous settings
	//	std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> m_prevParamsMap2; //Save the previous settings
	//	std::map<uint16_t, std::map<uint16_t,bool> > m_prevChange; //1: params changed 0: params no changed
	std::map<uint16_t,SONCommon::PerformanceReport> m_hoReportList; //mapping between cellId-handover performance report  data, used for periodical report

	std::map<uint16_t, SONCommon::HandoverStatisticReport > m_hoStatisticList; //user for continuous report
	//	std::map<uint16_t, bool> m_triggerAlgorithm;//indicate if HO performance data is ready, used for continuous report
	//	std::map<uint16_t, uint8_t > m_neighborNum; //#of neighbor for all cell
	std::set<uint16_t> m_tttSet;//Configuration Set
	std::set<double> m_a3OffsetSet;
	std::set<int8_t> m_cioSet;

	//	std:list<double> m_rrDb;
	//	std::map<uint16_t, std::map<uint16_t, double> >  m_prevHOPDb;//Save the previous performance of a ncell

	//	double m_kpiTh;
	//	uint16_t m_sampleLen;

	mutable SystemMutex m_mutex;
	bool m_kpiFirstWrite;

	bool m_enAlm;
	//	std::map<uint16_t, std::map<uint16_t, bool> > m_enAlgmCellPair; //enable algorithm for each cell pair

	//	bool m_endSlideWindow;
	//	bool m_runMROFirstTime;
	RRInfo m_rrInfo;
	bool m_enKPISaveCons;
	double m_statOffset;

	struct QMROAction
	{
		double m_hys;
		uint16_t m_ttt;
		double val;
	};

	struct QMRORegime
	{
		bool r2;
		bool r3;
	};

	//Paper: 2016-Cognitive Cellular Networks: A Q-Learning Framework for Self-Organizing Networks
	std::vector<QMROAction> R1PolicyGen(uint16_t cellId);
	std::vector<QMROAction> R2PolicyGen(QMROAction r1Action, uint16_t cellId);
	std::vector<QMROAction> R3PolicyGen(QMROAction r2Action, uint16_t cellId);
	std::map<uint16_t, std::vector<QMROAction> > m_qMROReward;
	std::map<uint16_t, uint16_t> m_qMROActionId;
	std::map<uint16_t, QMRORegime> m_qMRORegime;
	std::map<uint16_t,bool> m_qMROEn;
	static bool SortActions(QMROAction a, QMROAction b);
};

bool
CSONMroAlgorithm::SortActions(QMROAction a, QMROAction b)
{
	return (a.val > b.val); //Maximum value is at the first place
}

std::vector<CSONMroAlgorithm::QMROAction>
CSONMroAlgorithm::R1PolicyGen(uint16_t cellId)
{
	std::vector<QMROAction> r1ActionList;
	QMROAction action1; action1.m_hys = 4; action1.m_ttt = 256; action1.val=0; r1ActionList.push_back(action1);
	QMROAction action2; action2.m_hys = 3; action2.m_ttt = 256; action2.val=0; r1ActionList.push_back(action2);
	QMROAction action3; action3.m_hys = 2; action3.m_ttt = 256; action3.val=0; r1ActionList.push_back(action3);
	QMROAction action4; action4.m_hys = 1; action4.m_ttt = 256; action4.val=0; r1ActionList.push_back(action4);
	std::random_shuffle(r1ActionList.begin(),r1ActionList.end());
	for(std::vector<CSONMroAlgorithm::QMROAction>::iterator it = r1ActionList.begin(); it != r1ActionList.end(); it++)
	{
		NS_LOG_UNCOND("CellId: " << cellId << " R1Policy " << " a3Offset: " << it->m_hys << " ttt: " << it->m_ttt);
	}
	m_qMROReward[cellId] = r1ActionList;
	m_qMROActionId[cellId] = 0;
	return r1ActionList;
}

std::vector<CSONMroAlgorithm::QMROAction>
CSONMroAlgorithm::R2PolicyGen(CSONMroAlgorithm::QMROAction r1Action, uint16_t cellId)
{
	std::vector<CSONMroAlgorithm::QMROAction> r2ActionList;
	if(((r1Action.m_hys==2) && (r1Action.m_ttt==256)))
	{
		QMROAction action1; action1.m_hys = 3; action1.m_ttt = 320; action1.val=0; r2ActionList.push_back(action1);
		//		QMROAction action2; action2.m_hys = 4; action2.m_ttt = 480; action2.val=0; r2ActionList.push_back(action2);
		QMROAction action3; action3.m_hys = 1; action3.m_ttt = 128; action3.val=0; r2ActionList.push_back(action3);
		QMROAction action4; action4.m_hys = 0; action4.m_ttt = 100; action4.val=0; r2ActionList.push_back(action4);
		//		QMROAction action5; action5.m_hys = 5; action5.m_ttt = 512; action5.val=0; r2ActionList.push_back(action5);
	}
	else if((r1Action.m_hys==4) && (r1Action.m_ttt==256))
	{
		//		QMROAction action1; action1.m_hys = 5; action1.m_ttt = 320; action1.val=0; r2ActionList.push_back(action1);
		//		QMROAction action2; action2.m_hys = 6; action2.m_ttt = 480; action2.val=0; r2ActionList.push_back(action2);
		QMROAction action3; action3.m_hys = 3; action3.m_ttt = 128; action3.val=0; r2ActionList.push_back(action3);
		QMROAction action4; action4.m_hys = 2; action4.m_ttt = 100; action4.val=0; r2ActionList.push_back(action4);
		QMROAction action5; action5.m_hys = 1; action5.m_ttt = 40;  action5.val=0; r2ActionList.push_back(action5);
	}
	else if((r1Action.m_hys==3) && (r1Action.m_ttt==256))
	{
		QMROAction action3; action3.m_hys = 2; action3.m_ttt = 128; action3.val=0; r2ActionList.push_back(action3);
		QMROAction action4; action4.m_hys = 1; action4.m_ttt = 100; action4.val=0; r2ActionList.push_back(action4);
		QMROAction action5; action5.m_hys = 4; action5.m_ttt = 320; action5.val=0; r2ActionList.push_back(action5);
		QMROAction action6; action6.m_hys = 0; action6.m_ttt = 40;  action6.val=0; r2ActionList.push_back(action6);
	}
	else if((r1Action.m_hys==1) && (r1Action.m_ttt==256))
	{
		QMROAction action3; action3.m_hys = 2; action3.m_ttt = 320; action3.val=0; r2ActionList.push_back(action3);
		QMROAction action4; action4.m_hys = 0; action4.m_ttt = 128; action4.val=0; r2ActionList.push_back(action4);
		//		QMROAction action5; action5.m_hys = 4; action5.m_ttt = 320; action5.val=0; r2ActionList.push_back(action5);
	}
	else
	{
		NS_FATAL_ERROR("Error Case, Hys: " << r1Action.m_hys << " TTT: " << r1Action.m_ttt);
	}
	std::random_shuffle(r2ActionList.begin(),r2ActionList.end());
	for(std::vector<CSONMroAlgorithm::QMROAction>::iterator it = r2ActionList.begin(); it != r2ActionList.end(); it++)
	{
		NS_LOG_UNCOND("CellId: " << cellId << " R2Policy " << " a3Offset: " << it->m_hys << " ttt: " << it->m_ttt);
	}
	m_qMROReward[cellId] = r2ActionList;
	m_qMROActionId[cellId] = 0;
	return r2ActionList;
}

std::vector<CSONMroAlgorithm::QMROAction>
CSONMroAlgorithm::R3PolicyGen(CSONMroAlgorithm::QMROAction r2Action, uint16_t cellId)
{
	std::vector<CSONMroAlgorithm::QMROAction> r3ActionList;
	QMROAction action1; action1.m_hys = r2Action.m_hys-0.5; action1.m_ttt = r2Action.m_ttt; action1.val = 0; r3ActionList.push_back(action1);
	QMROAction action2; action2.m_hys = r2Action.m_hys-0.5; action2.m_ttt = r2Action.m_ttt; action2.val = 0; r3ActionList.push_back(action2);
	std::set<uint16_t>::iterator tttIt = m_tttSet.find(r2Action.m_ttt);
	QMROAction action3; action3.m_hys = r2Action.m_hys;   action3.m_ttt = *(--tttIt); action3.val = 0; r3ActionList.push_back(action3);
	QMROAction action4; action4.m_hys = r2Action.m_hys;   action4.m_ttt = *(++(++tttIt)); action4.val = 0; r3ActionList.push_back(action4);
	std::random_shuffle(r3ActionList.begin(),r3ActionList.end());
	for(std::vector<CSONMroAlgorithm::QMROAction>::iterator it = r3ActionList.begin(); it != r3ActionList.end(); it++)
	{
		NS_LOG_UNCOND("CellId: " << cellId << " R3Policy " << " a3Offset: " << it->m_hys << " ttt: " << it->m_ttt);
	}
	m_qMROReward[cellId] = r3ActionList;
	m_qMROActionId[cellId] = 0;
	return r3ActionList;
}

CSONMroAlgorithm::CSONMroAlgorithm()
{
	m_tttSet.insert(0); m_tttSet.insert(40); m_tttSet.insert(64); m_tttSet.insert(80); m_tttSet.insert(100); m_tttSet.insert(128);
	m_tttSet.insert(160); m_tttSet.insert(256); m_tttSet.insert(320); m_tttSet.insert(480); m_tttSet.insert(512); m_tttSet.insert(640);
	m_tttSet.insert(1024); m_tttSet.insert(1280); m_tttSet.insert(2560); m_tttSet.insert(5120);

	for(uint8_t i = 1; i <= 61; i++)
	{
		m_a3OffsetSet.insert(-15+0.5*(i-1));
	}

	for(int8_t i = -12; i <= -4 ; ++i){
		m_cioSet.insert(2*i);//-24,-20,..,-8
	}
	for(int8_t i = -6; i <= 6 ; ++i){
		m_cioSet.insert(i);//-6,-5,...,6
	}
	for(int8_t i = 4; i <= 12 ; ++i){
		m_cioSet.insert(2*i);//8,10,...,24
	}

	m_statOffset = 80;

	//	m_enAlm = true;//Enable algorithm

	//	Simulator::Schedule(Seconds(60),&CSONMroAlgorithm::TimeSlideWindowStart,this);
	//	m_endSlideWindow = false;
	//	m_runMROFirstTime = true;
	m_kpiFirstWrite = true;
	m_rrInfo.aggNumIssues = 0; m_rrInfo.aggnumHO= 0; NS_ASSERT(m_rrInfo.rlfHistory.empty() == true);
	m_enKPISaveCons = false;
}

void
CSONMroAlgorithm::SetParams(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap)
{
	NS_ASSERT(smallcellEnbDevs.GetN() > 0);
	m_smallcellEnbDevs = smallcellEnbDevs;
	NS_ASSERT(initialParamsMap.size() == smallcellEnbDevs.GetN());
	m_paramsMap = initialParamsMap;
	Simulator::Schedule(Seconds(60),&CSONMroAlgorithm::TimeSlideWindowStart,this);

	for(uint16_t i = 1; i <= smallcellEnbDevs.GetN();i++)
	{
		m_qMRORegime[i].r2 = false;
		m_qMRORegime[i].r3 = false;
		m_qMROEn[i]=true;
	}
}

CSONMroAlgorithm::CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap){
	//Initial Setup
	NS_ASSERT(smallcellEnbDevs.GetN() > 0);
	m_smallcellEnbDevs = smallcellEnbDevs;
	NS_ASSERT(initialParamsMap.size() == smallcellEnbDevs.GetN());
	m_paramsMap = initialParamsMap;
	//	m_prevParamsMap1 = initialParamsMap;
	//	m_prevParamsMap2 = initialParamsMap;

	//	for(uint16_t i = 1; i <= smallcellEnbDevs.GetN();i++)
	//	{
	////		for(std::map<uint16_t,int8_t>::iterator cellIt = m_paramsMap[i].cioList.begin();
	////				cellIt != m_paramsMap[i].cioList.end(); cellIt++)
	////		{
	////			m_enAlgmCellPair[i][cellIt->first] = true;
	////			m_prevChange[i][cellIt->first] = false;
	////		}
	//		m_qMROReward[i]=R1PolicyGen(); //Generate R1 policy for all the cell
	//	}

	//Set up Standard Values for parameters
	m_tttSet.insert(0); m_tttSet.insert(40); m_tttSet.insert(64); m_tttSet.insert(80); m_tttSet.insert(100); m_tttSet.insert(128);
	m_tttSet.insert(160); m_tttSet.insert(256); m_tttSet.insert(320); m_tttSet.insert(480); m_tttSet.insert(512); m_tttSet.insert(640);
	m_tttSet.insert(1024); m_tttSet.insert(1280); m_tttSet.insert(2560); m_tttSet.insert(5120);

	//	m_neighborNum[1] = 6; m_neighborNum[2] = 3; m_neighborNum[3] = 3; m_neighborNum[4] = 3; //manually configured based on simulation topology
	//	m_neighborNum[5] = 3; m_neighborNum[6] = 3;	m_neighborNum[7] = 3;
	for(uint8_t i = 1; i <= 61; i++){
		m_a3OffsetSet.insert(-15+0.5*(i-1));
	}

	for(int8_t i = -12; i <= -4 ; ++i){
		m_cioSet.insert(2*i);//-24,-20,..,-8
	}
	for(int8_t i = -6; i <= 6 ; ++i){
		m_cioSet.insert(i);//-6,-5,...,6
	}
	for(int8_t i = 4; i <= 12 ; ++i){
		m_cioSet.insert(2*i);//8,10,...,24
	}

	//	m_kpiTh = 0.01;//handover key performance indicatior
	//	m_sampleLen = 20;//sample length
	m_statOffset = 80;

	m_enAlm = true;//Enable algorithm

	Simulator::Schedule(Seconds(60),&CSONMroAlgorithm::TimeSlideWindowStart,this);
	//	m_endSlideWindow = false;
	//	m_runMROFirstTime = true;
	m_kpiFirstWrite = true;
	m_rrInfo.aggNumIssues = 0; m_rrInfo.aggnumHO= 0; NS_ASSERT(m_rrInfo.rlfHistory.empty() == true);
	m_enKPISaveCons = false;
}

CSONMroAlgorithm::~CSONMroAlgorithm(){}

void
CSONMroAlgorithm::TimeSlideWindowStart(){
	//	NS_LOG_UNCOND("CSONMroAlgorithm::TimeSlideWindowStart at time " <<Simulator::Now ().GetNanoSeconds () / (double) 1e9);
	//	m_endSlideWindow = true;
	MroAlgorithm();
	Simulator::Schedule(Seconds(60),&CSONMroAlgorithm::TimeSlideWindowStart,this);
}

void
CSONMroAlgorithm::UpdateRR(double curTime, bool rlf){
	RLFTiming rlfTiming; rlfTiming.rlf = rlf; rlfTiming.time = curTime;
	m_rrInfo.rlfHistory.push(rlfTiming);
	NS_LOG_LOGIC("CSONMroAlgorithm::UpdateRR push Time "<< rlfTiming.time << " RLF "<<rlfTiming.rlf);
	m_rrInfo.aggnumHO += 1;
	if(rlf == true){
		m_rrInfo.aggNumIssues += 1;
	}
	if(m_rrInfo.rlfHistory.empty() == false){
		while((curTime - m_rrInfo.rlfHistory.front().time) >= 60){
			m_rrInfo.aggnumHO -= 1;
			if(m_rrInfo.rlfHistory.front().rlf == true){
				m_rrInfo.aggNumIssues -= 1;
			}
			NS_LOG_LOGIC("CSONMroAlgorithm::UpdateRR Pop Time "<< m_rrInfo.rlfHistory.front().time << " RLF "<<m_rrInfo.rlfHistory.front().rlf);
			m_rrInfo.rlfHistory.pop();
		}
	}
	//	SaveRR2Log(curTime,(double)m_rrInfo.aggNumIssues/m_rrInfo.aggnumHO);
}

void
CSONMroAlgorithm::UpdateMroStatisticalData(uint16_t cellId, SONCommon::MROProblemInfo probInfo){
	//	NS_LOG_UNCOND("At main, CSONMroAlgorithm::UpdateMroStatisticalData cellId " << cellId << " problem: " << probInfo.problem
	//			<< " IsMeasListEmpty: " << (uint16_t)probInfo.measData.empty());

	//	CriticalSection cs (m_mutex);//Lock the related memory

	//Update handover problem counters for each neighboring cell
	double curTime = Simulator::Now ().GetNanoSeconds () / (double) 1e9;
	std::map<uint16_t, SONCommon::HandoverStatisticReport >::iterator dbIt =  m_hoStatisticList.find(cellId);
	if(dbIt == m_hoStatisticList.end())
	{//At the initial time, there is no database, so we have to build it
		SONCommon::HandoverStatisticReport hoStatReport;
		hoStatReport.numPingpongHos = 0;	hoStatReport.numTooEarlyHos = 0; hoStatReport.numTooLateHos = 0; hoStatReport.numWrongCellHos = 0; hoStatReport.numHOs = 0;
		SONCommon::CellStatistic hoStatNCell;
		hoStatNCell.m_numLHO = 0;	hoStatNCell.m_numEHO = 0;	hoStatNCell.m_numWHO = 0; hoStatNCell.m_numPP = 0; hoStatNCell.m_numNHO = 0; //initialize all HO counters for this neihbour
		std::map<uint16_t, SONCommon::CellStatistic> statNcellMem; //memory that stores neighboring cell handover problems

		switch(probInfo.problem)
		{
		case SONCommon::TOO_LATE_HANDOVER:
		{
			hoStatNCell.m_numLHO += 1;
			hoStatReport.numTooLateHos += 1;
			UpdateRR(curTime,true);
			break;
		}
		case SONCommon::TOO_EARLY_HANDOVER:
		{
			hoStatNCell.m_numEHO += 1;
			hoStatReport.numTooEarlyHos += 1;
			UpdateRR(curTime,true);
			break;
		}
		case SONCommon::WRONG_CELL_HANDOVER:
		{
			hoStatNCell.m_numWHO += 1;
			hoStatReport.numWrongCellHos = 0;
			UpdateRR(curTime,true);
			break;
		}
		case SONCommon::PING_PONG_HANDOVER:
		{
			hoStatNCell.m_numPP += 1;
			hoStatReport.numPingpongHos += 1;
			UpdateRR(curTime,false);
			break;
		}
		case SONCommon::NORMAL_HANDOVER:
		{
			hoStatNCell.m_numNHO += 1;
			hoStatReport.numHOs += 1;
			UpdateRR(curTime,false);
			break;
		}
		default:
		{
			NS_LOG_UNCOND(" CONNECTION DROP CellId: " << probInfo.cellId << " IMSI: " << probInfo.imsi);
		}
		}//End Switch

		hoStatReport.hoStatistic[probInfo.cellId] = hoStatNCell;
		m_hoStatisticList[cellId] = hoStatReport; //Update for reporting cell

	}
	else
	{
		std::map<uint16_t, SONCommon::CellStatistic>::iterator statNcellMemIt = (dbIt->second).hoStatistic.find(probInfo.cellId); //Find the database of this neighboring cell
		if(statNcellMemIt == (dbIt->second).hoStatistic.end())
		{//If the neighboring cell database is new
			SONCommon::CellStatistic hoStatNCell;
			hoStatNCell.m_numLHO = 0;
			hoStatNCell.m_numEHO = 0;
			hoStatNCell.m_numWHO = 0;
			hoStatNCell.m_numPP = 0;
			hoStatNCell.m_numNHO = 0; //initialize all HO counters for this neihbour

			switch(probInfo.problem)
			{
			case SONCommon::TOO_LATE_HANDOVER:
			{
				hoStatNCell.m_numLHO += 1;
				(dbIt->second).numTooLateHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::TOO_EARLY_HANDOVER:
			{
				hoStatNCell.m_numEHO += 1;
				(dbIt->second).numTooEarlyHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::WRONG_CELL_HANDOVER:
			{
				hoStatNCell.m_numWHO += 1;
				(dbIt->second).numWrongCellHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::PING_PONG_HANDOVER:
			{
				hoStatNCell.m_numPP += 1;
				(dbIt->second).numPingpongHos += 1;
				UpdateRR(curTime,false);
				break;
			}
			case SONCommon::NORMAL_HANDOVER:
			{
				hoStatNCell.m_numNHO += 1;
				(dbIt->second).numHOs += 1;
				UpdateRR(curTime,false);
				break;
			}
			default:
			{
				NS_LOG_UNCOND(" CONNECTION DROP CellId: " << probInfo.cellId << " IMSI: " << probInfo.imsi);
			}
			}//End Switch

			(dbIt->second).hoStatistic[probInfo.cellId] = hoStatNCell;//Create new statistic for this neigboring cell
		}else{//If the neighboring cell database is already existed
			switch(probInfo.problem)
			{
			case SONCommon::TOO_LATE_HANDOVER:
			{
				statNcellMemIt->second.m_numLHO += 1;
				(dbIt->second).numTooLateHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::TOO_EARLY_HANDOVER:
			{
				statNcellMemIt->second.m_numEHO += 1;
				(dbIt->second).numTooEarlyHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::WRONG_CELL_HANDOVER:
			{
				statNcellMemIt->second.m_numWHO += 1;
				(dbIt->second).numWrongCellHos += 1;
				UpdateRR(curTime,true);
				break;
			}
			case SONCommon::PING_PONG_HANDOVER:
			{
				statNcellMemIt->second.m_numPP += 1;
				(dbIt->second).numPingpongHos += 1;
				UpdateRR(curTime,false);
				break;
			}
			case SONCommon::NORMAL_HANDOVER:
			{
				statNcellMemIt->second.m_numNHO += 1;
				(dbIt->second).numHOs += 1;
				UpdateRR(curTime,false);
				break;
			}
			default:
			{
				NS_LOG_UNCOND(" CONNECTION DROP CellId: " << probInfo.cellId << " IMSI: " << probInfo.imsi);
			}
			}//End Switch
		}
	}
	std::map<uint16_t, SONCommon::HandoverStatisticReport >::iterator latestDBIt =  m_hoStatisticList.find(cellId);
	NS_ASSERT(latestDBIt != m_hoStatisticList.end());
	SONCommon::HandoverStatisticReport updatedReport = m_hoStatisticList[cellId];

	//Continuously save KPI for each eNB
	if(m_enKPISaveCons)
	{
		SaveKPI2Log(curTime, cellId, updatedReport.numTooLateHos, updatedReport.numTooEarlyHos, updatedReport.numWrongCellHos, updatedReport.numPingpongHos,
				updatedReport.numHOs+updatedReport.numTooLateHos+updatedReport.numTooEarlyHos+updatedReport.numWrongCellHos+updatedReport.numPingpongHos,
				(double)(updatedReport.numTooLateHos+updatedReport.numTooEarlyHos+updatedReport.numWrongCellHos)/
				(updatedReport.numHOs+updatedReport.numTooLateHos+updatedReport.numTooEarlyHos+updatedReport.numWrongCellHos+updatedReport.numPingpongHos));
	}

}

void
CSONMroAlgorithm::SaveKPI2Log(double curTime, uint16_t cellId, uint16_t lho, uint16_t eho, uint16_t who,
		uint16_t pho, uint16_t total, double kpi){
	std::ostringstream converter; //convert integer to string
	converter << cellId;
	std::string fileName = "KPICellId"+converter.str()+".txt";
	std::ofstream outFile;
	if (m_kpiFirstWrite == true){
		outFile.open (fileName.c_str());
		if (!outFile.is_open ()){
			NS_LOG_ERROR ("Can't open file " << fileName.c_str());
			return;
		}
		m_kpiFirstWrite = false;
		//		outFile << "time\tcellId\t#LHOs\t#EHOs\t#WHOs\t#PHOs\t#Total\tKPI";
		//		outFile << std::endl;
	}else{
		outFile.open (fileName.c_str(),  std::ios_base::app);
		if (!outFile.is_open ()){
			NS_LOG_ERROR ("Can't open file " << fileName.c_str());
			return;
		}
	}
	outFile << curTime << "\t";	outFile << cellId << "\t";	outFile << lho << "\t";		outFile << eho << "\t";
	outFile << who << "\t";		outFile << pho << "\t";		outFile << total << "\t";	outFile << kpi << std::endl;
	outFile.close ();
}

void
CSONMroAlgorithm::SaveRlfPP2Log(double curTime, double ppr, double rr){
	std::string fileName = "RRR.txt";
	std::ofstream outFile;
	outFile.open (fileName.c_str(),  std::ios_base::app);
	if (!outFile.is_open ()){
		NS_LOG_ERROR ("Can't open file " << fileName.c_str());
		return;
	}
	outFile << curTime << "\t" << ppr << "\t" << rr << std::endl;
	outFile.close ();
}

void
CSONMroAlgorithm::SaveRR2Log(double curTime, double consRR){
	std::string fileName = "RR.txt";
	std::ofstream outFile;
	outFile.open (fileName.c_str(),  std::ios_base::app);
	if (!outFile.is_open ()){
		NS_LOG_ERROR ("Can't open file " << fileName.c_str());
		return;
	}
	outFile << curTime << "\t" << consRR << std::endl;
	outFile.close ();
}

void
CSONMroAlgorithm::MroAlgorithm(){
	/**********************************Calculate RRR**********************************/
	uint16_t sumRLF = 0;
	uint16_t sumHO = 0;
	uint16_t sumPP = 0;
	double curRR = 0;
	double curPPR = 0;
	double curTime = Simulator::Now ().GetNanoSeconds () / (double) 1e9;
	for(uint16_t cellId = 1; cellId <= m_smallcellEnbDevs.GetN(); cellId++){
		sumRLF += (m_hoStatisticList[cellId].numTooLateHos + m_hoStatisticList[cellId].numTooEarlyHos + m_hoStatisticList[cellId].numWrongCellHos);
		sumHO += (m_hoStatisticList[cellId].numTooLateHos + m_hoStatisticList[cellId].numTooEarlyHos + m_hoStatisticList[cellId].numWrongCellHos
				+m_hoStatisticList[cellId].numPingpongHos + m_hoStatisticList[cellId].numHOs)+m_statOffset;
		sumPP += m_hoStatisticList[cellId].numPingpongHos;

		if(m_enKPISaveCons == false)
		{
			SaveKPI2Log(curTime, cellId, m_hoStatisticList[cellId].numTooLateHos, m_hoStatisticList[cellId].numTooEarlyHos, m_hoStatisticList[cellId].numWrongCellHos,
					m_hoStatisticList[cellId].numPingpongHos,
					m_hoStatisticList[cellId].numHOs+m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos+m_statOffset,
					(double)(m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos)/
					(m_hoStatisticList[cellId].numHOs+m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos+m_statOffset));
		}
	}
	curRR = (double)sumRLF/sumHO;
	curPPR = (double)sumPP/sumHO;
	SaveRlfPP2Log(curTime, curPPR, curRR);

	//	m_rrDb.push_back(curRR);
	//	if(m_runMROFirstTime == true)
	//	{
	//		m_runMROFirstTime = false;
	//		SaveRlfPP2Log(curTime, 0, curRR);
	//	}
	//	else
	//	{
	//		std::list<double>::reverse_iterator curRRDbIt = m_rrDb.rbegin(); ++curRRDbIt;
	//		NS_ASSERT(curRRDbIt != m_rrDb.rend());
	//		double preRR = *curRRDbIt;
	//		//				std::list<double>::reverse_iterator preRRDbIt = ++curRRDbIt;
	//		double curTime = Simulator::Now ().GetNanoSeconds () / (double) 1e9;
	//		SaveRlfPP2Log(curTime, 1-(curRR)/(preRR), curRR);
	//	}
	for(uint16_t cellId = 1; cellId <= m_smallcellEnbDevs.GetN(); cellId++)
	{
		//		SONCommon::HandoverStatisticReport updatedReport = m_hoStatisticList[cellId];
		//		std::map<uint16_t, SONCommon::HandoverStatisticReport>::iterator cellHOStatIt = m_hoStatisticList.find(cellId);
		//		uint16_t NumIssues = updatedReport.numTooEarlyHos + updatedReport.numTooLateHos + updatedReport.numWrongCellHos + updatedReport.numPingpongHos;
		//		uint16_t NumHO = updatedReport.numTooEarlyHos + updatedReport.numTooLateHos + updatedReport.numWrongCellHos
		//				+ updatedReport.numPingpongHos + updatedReport.numHOs + m_statOffset;
		//		double cellRR = (double)NumIssues/NumHO;

		if(m_qMROEn[cellId])
		{
			double curLHO = (double)m_hoStatisticList[cellId].numTooLateHos /
					(m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos+m_hoStatisticList[cellId].numHOs+m_statOffset);

			double curEHO = (double)(m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos) /
					(m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos+m_hoStatisticList[cellId].numHOs+m_statOffset);

			double curPP = (double)m_hoStatisticList[cellId].numPingpongHos /
					(m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos+m_hoStatisticList[cellId].numHOs+m_statOffset);

			double curHOP = -(1*curPP + 1*curEHO + 1*curLHO)/3; //Eq.8

			NS_LOG_UNCOND("cellId: " << cellId << " curA3Offset: " << m_paramsMap[cellId].a3Offset <<
					" curTTT: " << m_paramsMap[cellId].ttt << " HOP: " << curHOP);

			m_qMROReward[cellId].at(m_qMROActionId[cellId]).val = curHOP;
			uint16_t qMRORewardSize = m_qMROReward[cellId].size();
			if((m_qMROActionId[cellId]+1)==qMRORewardSize)
			{
				//select the best action
				std::sort(m_qMROReward[cellId].begin(),m_qMROReward[cellId].end(),CSONMroAlgorithm::SortActions);
				for(std::vector<CSONMroAlgorithm::QMROAction>::iterator it = m_qMROReward[cellId].begin(); it != m_qMROReward[cellId].end(); it++)
				{
					NS_LOG_UNCOND("CellId: " << cellId << " Sorting Policy " << " a3Offset: " << it->m_hys << " ttt: " << it->m_ttt << " Val: " << it->val);
				}
				m_paramsMap[cellId].a3Offset = m_qMROReward[cellId].at(0).m_hys;
				m_paramsMap[cellId].ttt = m_qMROReward[cellId].at(0).m_ttt;
				//make the next regime;
				if(m_qMRORegime[cellId].r2==false)
				{
					m_qMRORegime[cellId].r2 = true;
					m_qMROReward[cellId] = R2PolicyGen(m_qMROReward[cellId].at(0),cellId);
				}
				else if(m_qMRORegime[cellId].r3==false)
				{
					m_qMRORegime[cellId].r3 = true;
					m_qMROReward[cellId] = R3PolicyGen(m_qMROReward[cellId].at(0),cellId);
				}
				else
				{//m_qMRORegime[cellId].r3==true
					m_qMROEn[cellId]=false;
				}
			}
			else
			{
				m_qMROActionId[cellId] = m_qMROActionId[cellId] + 1;
				m_paramsMap[cellId].a3Offset = m_qMROReward[cellId].at(m_qMROActionId[cellId]).m_hys;
				m_paramsMap[cellId].ttt = m_qMROReward[cellId].at(m_qMROActionId[cellId]).m_ttt;
			}

			//Update new action
			LteEnbDSONSapProvider::MroParameters mroParams = m_paramsMap[cellId];
			Ptr<LteEnbRrc> rrcPtr = m_smallcellEnbDevs.Get(cellId-1)->GetObject<LteEnbNetDevice>()->GetRrc();
			bool req = true;
			Simulator::ScheduleNow(&LteEnbRrc::RrcSetMroParams, rrcPtr, req, mroParams);
		}
		else
		{
			//Update new action
			LteEnbDSONSapProvider::MroParameters mroParams = m_paramsMap[cellId];
			Ptr<LteEnbRrc> rrcPtr = m_smallcellEnbDevs.Get(cellId-1)->GetObject<LteEnbNetDevice>()->GetRrc();
			bool req = false;
			Simulator::ScheduleNow(&LteEnbRrc::RrcSetMroParams, rrcPtr, req, mroParams);
		}

		/**********************************Resset Memory**********************************/
		//		cellHOStatIt->second.hoStatistic.clear();
		//		cellHOStatIt->second.numHOs = 0;
		//		cellHOStatIt->second.numTooEarlyHos = 0;
		//		cellHOStatIt->second.numTooLateHos = 0;
		//		cellHOStatIt->second.numPingpongHos = 0;
		//		cellHOStatIt->second.numWrongCellHos = 0;
		m_hoStatisticList[cellId].hoStatistic.clear();
		m_hoStatisticList[cellId].numHOs = 0;
		m_hoStatisticList[cellId].numTooLateHos = 0;
		m_hoStatisticList[cellId].numTooEarlyHos = 0;
		m_hoStatisticList[cellId].numPingpongHos = 0;
		m_hoStatisticList[cellId].numWrongCellHos = 0;
	}//End For
	//	m_prevParamsMap2 = m_prevParamsMap1;
	//	m_prevParamsMap1 = m_paramsMap; //Save for next adjustment
}

class MroStatisticalDataProcessor : public Object{
public:
	MroStatisticalDataProcessor(Ptr<CSONMroAlgorithm> mroAlgorithm);
	virtual ~MroStatisticalDataProcessor();
	static void RecvMroStaticalDataCallback(Ptr<MroStatisticalDataProcessor> outputProcessor, std::string path,
			uint16_t cellId, SONCommon::MROProblemInfo probInfo);//For continuous udpate
	void SaveMroStatisticalData(uint16_t cellId, SONCommon::MROProblemInfo probInfo);
	Ptr<CSONMroAlgorithm> m_mroAlgorithm;//There must be conflict about CSONMroAlgorithm pointer object between NS-3 and Pthread
};

MroStatisticalDataProcessor::MroStatisticalDataProcessor(Ptr<CSONMroAlgorithm> mroAlgorithm){
	m_mroAlgorithm = mroAlgorithm;
}

MroStatisticalDataProcessor::~MroStatisticalDataProcessor(){

}

void
MroStatisticalDataProcessor::RecvMroStaticalDataCallback(Ptr<MroStatisticalDataProcessor> outputProcessor, std::string path,
		uint16_t cellId, SONCommon::MROProblemInfo probInfo){
	outputProcessor->SaveMroStatisticalData(cellId, probInfo); //Connect to trace source
}

void
MroStatisticalDataProcessor::SaveMroStatisticalData(uint16_t cellId, SONCommon::MROProblemInfo statData){
	switch(cellId){
	case 1:	case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
		m_mroAlgorithm->UpdateMroStatisticalData(cellId, statData);//Send Statistic Data to MRO Algorithm
		break;

	default:
		NS_FATAL_ERROR("There is not cellId " << cellId);
		break;
	}
}


int main (int argc, char *argv[])
{
	GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
	Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (100)));
	Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024));
	Config::SetDefault ("ns3::RrFfMacScheduler::HarqEnabled", BooleanValue (false)); //Disable HARQ
	Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
	Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
	Config::SetDefault ("ns3::LteHelper::AnrEnabled", BooleanValue (false)); //Used A4 of ANR function
	Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (false)); //AccumulationEnabled
	Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false)); //

	bool generateRem = false;
	double enbTxDbm=24; //24dBm for small cell, 46 dBm for macro cell
	double simTime = 60*30+1; //unit s
	double ueMinSpeedKmh=5; //5kmph
	double ueMaxSpeedKmh=7; //15kmph
	double ueCxtStore = 2;
	double qInSync = -2;
	double qOutOfSync = -4;
	bool epcDl = true; //Enable full-buffer DL traffic
	bool epcUl = false; //Enable full-buffer UL traffic
	bool useUdp = false; //Disable UDP traffic and enable TCP instead
	//  1.4MHz-6RBs, 3MHz-15RBs, 5MHz-25RBs, 10MHz-50RBs, 15MHz-75RBs, 20MHz-100RBs
	uint16_t smallcellEnbBandwidth=100;
	uint16_t numBearersPerUe = 1;
	double bsAntennaHeight = 5; //like lamp post
	double shadowingVar = 36;
	double varBound = 25;//1.5*std::sqrt(shadowingVar);
	double cellHysCommon = 3;
	double cellTTTCommon = 256;
	uint16_t measMetric = 2; //0: RSRP, 1: RSRQ, 2: SNRI
	double offset=3; //Island offset
	//	uint16_t sinrType = 1; //0: all nCells; 1: strongest nCell

	uint16_t m_numberOfRWP        = 149;
	uint16_t m_numberOfManhattan  = 3;//Total is 200 UEs

	CommandLine cmd;
	cmd.AddValue("ShadowingVar","Shadowing Variance (dB)",shadowingVar);
	cmd.AddValue("VarBound","Bound of the random around the std value (dB)",varBound);
	cmd.AddValue("ComHys","Common Hys value for all cells (dB)", cellHysCommon);
	cmd.AddValue("ComTTT","Common TTT value for all cells (ms)", cellTTTCommon);
	//	cmd.AddValue("SinrType","0: all nCells; 1: strongest nCell", sinrType);
	cmd.AddValue("MeasMetric","0: RSRP, 1: RSRQ, 2: SINR", measMetric);
	cmd.AddValue("MaxSpeed","maximum speed value (km/h)", ueMaxSpeedKmh);
	cmd.AddValue("MinSpeed","minimum speed value (km/h)", ueMinSpeedKmh);
	cmd.AddValue("QInSync"," (dB) ", qInSync);
	cmd.AddValue("QOutOfSync"," (dB) ", qOutOfSync);
	cmd.AddValue("IslandOffset","effect of island (dB)", offset);
	// parse again so you can override input file default values via command line
	cmd.Parse (argc, argv);

	double ueMinSpeed=ueMinSpeedKmh/3.6; //5kmph
	double ueMaxSpeed=ueMaxSpeedKmh/3.6; //15kmph
	// parse again so you can override input file default values via command line

	NS_LOG_UNCOND("ShadowingVar: " <<shadowingVar << " VarBound: "
			<< varBound << " TTT: " << cellTTTCommon << " Hys(A3Offset): " << cellHysCommon
			<< " QInSync: " << qInSync << " QOutOfSync: " << qOutOfSync << " MeasMetric: " << measMetric //<< " SINRType: " << sinrType
			<< " MinSpeed (kmh): " << ueMinSpeedKmh << " MaxSpeed (kmh): " << ueMaxSpeedKmh << " IslandOffset (dB): " << offset);

	double cell1Hys = cellHysCommon;    double cell1A3Off = 0;    uint64_t cell1TTT = cellTTTCommon;
	double cell2Hys = cellHysCommon;    double cell2A3Off = 0;    uint64_t cell2TTT = cellTTTCommon;
	double cell3Hys = cellHysCommon;    double cell3A3Off = 0;    uint64_t cell3TTT = cellTTTCommon;
	double cell4Hys = cellHysCommon;    double cell4A3Off = 0;    uint64_t cell4TTT = cellTTTCommon;
	double cell5Hys = cellHysCommon;    double cell5A3Off = 0;    uint64_t cell5TTT = cellTTTCommon;
	double cell6Hys = cellHysCommon;    double cell6A3Off = 0;    uint64_t cell6TTT = cellTTTCommon;
	double cell7Hys = cellHysCommon;    double cell7A3Off = 0;    uint64_t cell7TTT = cellTTTCommon;
	double cell8Hys = cellHysCommon;    double cell8A3Off = 0;    uint64_t cell8TTT = cellTTTCommon;
	double cell9Hys = cellHysCommon;    double cell9A3Off = 0;    uint64_t cell9TTT = cellTTTCommon;
	double cell10Hys = cellHysCommon;    double cell10A3Off = 0;    uint64_t cell10TTT = cellTTTCommon;
	double cell11Hys = cellHysCommon;    double cell11A3Off = 0;    uint64_t cell11TTT = cellTTTCommon;
	double cell12Hys = cellHysCommon;    double cell12A3Off = 0;    uint64_t cell12TTT = cellTTTCommon;

	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();

	/****************************Setup Channel Parameters****************************/
	lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");
	lteHelper->SetSpectrumChannelAttribute("ShadowingVariance",DoubleValue(shadowingVar));
	lteHelper->SetSpectrumChannelAttribute("VarianceBound",DoubleValue(varBound));
	//
	//	lteHelper->SetAttribute("EnableIslandMode",BooleanValue(true));
	//	std::map< uint64_t, std::list< Ptr<IsLandProfile> > > islandList;
	//	Ptr<CircleIsland> islandWHO = CreateObject<CircleIsland>(35,50,2,17); //(X,Y,radius,gain)
	//
	//	islandList[6].push_back(islandWHO);
	//
	//	Ptr<CircleIsland> islandEHO1= CreateObject<CircleIsland>(55,45,2,22);//We have to create new object
	//
	//	islandList[7].push_back(islandEHO1);
	//	//	lteHelper->SetIslandDownlinkSpectrum(islandList);
	//
	//	Ptr<CircleIsland> islandEHO2= CreateObject<CircleIsland>(70,50,2,16);//We have to create new object
	//	islandList[1].push_back(islandEHO2);
	//	lteHelper->SetIslandDownlinkSpectrum(islandList);

	lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
	if (ueMaxSpeedKmh < 10)
	{
		lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
	}
	else if(ueMaxSpeedKmh < 40)
	{
		lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_30kmph.fad"));
	}
	else
	{
		lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_60kmph.fad"));
	}
	//	lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
	//	lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
	lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
	lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
	lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
	lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (smallcellEnbBandwidth));

	lteHelper->SetAttribute("PathlossModel",StringValue("ns3::PathlossModelETRI"));//Modified Path loss
	lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
	lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
	lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (true));
	Ptr<PointToPointEpcHelper> epcHelper;
	epcHelper = CreateObject<PointToPointEpcHelper> ();
	lteHelper->SetEpcHelper (epcHelper);

	Ptr<ListPositionAllocator> eNBPosList = CreateObject<ListPositionAllocator> ();
	eNBPosList->Add (Vector (0, 0, bsAntennaHeight)); //cellID 1
	eNBPosList->Add (Vector (30, 0, bsAntennaHeight)); //cellID 2
	eNBPosList->Add (Vector (75, 30, bsAntennaHeight)); //cellID 3
	eNBPosList->Add (Vector (15, 40, bsAntennaHeight));  //cellID 4
	eNBPosList->Add (Vector (-15, 40, bsAntennaHeight));  //cellID 5
	eNBPosList->Add (Vector (-65, 30, bsAntennaHeight));  //cellID 6
	eNBPosList->Add (Vector (-30, -30, bsAntennaHeight));  //cellID 7
	eNBPosList->Add (Vector (40, -30, bsAntennaHeight));  //cellID 8
	eNBPosList->Add (Vector (75, 0, bsAntennaHeight));  //cellID 9
	eNBPosList->Add (Vector (75, -30, bsAntennaHeight));  //cellID 10
	eNBPosList->Add (Vector (-45, 0, bsAntennaHeight));  //cellID 11
	eNBPosList->Add (Vector (0, -30, bsAntennaHeight));  //cellID 12

	Ptr<CSONMroAlgorithm> mroAlgorithm = CreateObject<CSONMroAlgorithm>();//sceNBDevs,initialParamsMap
	//mroAlgorithm->SetParams(sceNBDevs,initialParamsMap);
	std::vector<CSONMroAlgorithm::QMROAction> param1 = mroAlgorithm->R1PolicyGen(1);
	std::vector<CSONMroAlgorithm::QMROAction> param2 = mroAlgorithm->R1PolicyGen(2);
	std::vector<CSONMroAlgorithm::QMROAction> param3 = mroAlgorithm->R1PolicyGen(3);
	std::vector<CSONMroAlgorithm::QMROAction> param4 = mroAlgorithm->R1PolicyGen(4);
	std::vector<CSONMroAlgorithm::QMROAction> param5 = mroAlgorithm->R1PolicyGen(5);
	std::vector<CSONMroAlgorithm::QMROAction> param6 = mroAlgorithm->R1PolicyGen(6);
	std::vector<CSONMroAlgorithm::QMROAction> param7 = mroAlgorithm->R1PolicyGen(7);
	std::vector<CSONMroAlgorithm::QMROAction> param8 = mroAlgorithm->R1PolicyGen(8);
	std::vector<CSONMroAlgorithm::QMROAction> param9 = mroAlgorithm->R1PolicyGen(9);
	std::vector<CSONMroAlgorithm::QMROAction> param10 = mroAlgorithm->R1PolicyGen(10);
	std::vector<CSONMroAlgorithm::QMROAction> param11 = mroAlgorithm->R1PolicyGen(11);
	std::vector<CSONMroAlgorithm::QMROAction> param12 = mroAlgorithm->R1PolicyGen(12);

	/****************************Set up eNB parameters****************************/

	NodeContainer smallcellEnbs;
	//	int8_t numberOfEnbs = 11;
	int8_t numberOfEnbs = eNBPosList->GetVectorPositionList().size();

	smallcellEnbs.Create (numberOfEnbs);
	mobility.SetPositionAllocator (eNBPosList);
	mobility.Install (smallcellEnbs);
	lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel"); //omni-directional antenna, gain is same 0db in all directions
	lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (1575));//downlink Frequency
	lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (1575)); //Downlink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (19575));//Uplink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (smallcellEnbBandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (smallcellEnbBandwidth));

	//	double offset=-3; //Can be considered as island effect
	uint32_t cellId = 0; //CELL 1
	//	cell1A3Off = -2;
	std::map<uint16_t,int8_t> cioVals1; cioVals1[2] = 0; cioVals1[5] = offset; cioVals1[11] = 0; cioVals1[12] = 0;// to force increase RLF
	lteHelper->SetInitialCIOValues(cioVals1);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 1
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param1.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(param1.at(0).m_ttt)));
	NetDeviceContainer sceNBDevs = lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 2
	std::map<uint16_t,int8_t> cioVals2; cioVals2[1] = 0; cioVals2[3] = 0; cioVals2[4] = 0; cioVals2[8] = 0; cioVals2[9] = 0; //cioVals2[10] = 0;
	lteHelper->SetInitialCIOValues(cioVals2);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 2
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param2.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(param2.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 3
	std::map<uint16_t,int8_t> cioVals3; cioVals3[2] = 0; cioVals3[9] = 0; //cioVals3[4] = 0;
	lteHelper->SetInitialCIOValues(cioVals3);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 3
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param3.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(param3.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 4
	//	cell4TTT = 160;
	std::map<uint16_t,int8_t> cioVals4; cioVals4[2] = 0; cioVals4[5] = 0;
	lteHelper->SetInitialCIOValues(cioVals4);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 4
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param4.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(param4.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 5
	//	cell5A3Off = -1;
	std::map<uint16_t,int8_t> cioVals5; cioVals5[1] = offset; cioVals5[4] = 0; cioVals5[6] = 0;
	lteHelper->SetInitialCIOValues(cioVals5);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 5
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param5.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(param5.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 6
	//	cell6A3Off = -1;
	//	cell6TTT = 160;
	std::map<uint16_t,int8_t> cioVals6; cioVals6[5] = 0; cioVals6[11] = 0;
	lteHelper->SetInitialCIOValues(cioVals6);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 6
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param6.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param6.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 7
	std::map<uint16_t,int8_t> cioVals7; cioVals7[1] = 0; cioVals7[6] = 0; cioVals7[11] = 0; cioVals7[12] = 0;
	lteHelper->SetInitialCIOValues(cioVals7);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 7
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param7.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param7.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 8
	//	cell8TTT = 160;
	std::map<uint16_t,int8_t> cioVals8; cioVals8[2] = 0; cioVals8[10] = 0; cioVals8[12] = 0;
	lteHelper->SetInitialCIOValues(cioVals8);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 8
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param8.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param8.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 9
	//	cell9TTT = 160;
	std::map<uint16_t,int8_t> cioVals9; cioVals9[2] = 0; cioVals9[3] = 0; cioVals9[10] = 0;
	lteHelper->SetInitialCIOValues(cioVals9);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 9
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param9.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param9.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 10
	//	cell10A3Off = -1;
	std::map<uint16_t,int8_t> cioVals10; cioVals10[8] = 0; cioVals10[9] = 0;
	lteHelper->SetInitialCIOValues(cioVals10);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 10
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param10.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param10.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 11
	//	cell11A3Off = -1;
	std::map<uint16_t,int8_t> cioVals11; cioVals11[1] = 0; cioVals11[6] = 0; cioVals11[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals11);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 11
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param11.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param11.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	cellId++; //CELL 12
	//	cell12A3Off = -1;
	std::map<uint16_t,int8_t> cioVals12; cioVals12[1] = 0; cioVals12[7] = 0; cioVals12[8] = 0;
	lteHelper->SetInitialCIOValues(cioVals12);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(0)); //Enb 12
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(param12.at(0).m_hys));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (param12.at(0).m_ttt)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(cellId)));
	sceNBDevs.Get(cellId)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	NS_ASSERT_MSG((cellId + 1) == sceNBDevs.GetN(),"Wrong cellId: " << cellId << " numCell: " << sceNBDevs.GetN());

	for(uint8_t enbID = 0; enbID < sceNBDevs.GetN(); ++enbID){
		Ptr<LteEnbNetDevice> lteEnbDev = sceNBDevs.Get(enbID)->GetObject<LteEnbNetDevice> ();
		Ptr<LteEnbRrc> enbRrc = lteEnbDev->GetRrc ();
		NS_ASSERT(enbRrc > NULL);
		enbRrc->SetAttribute ("Tuecxtstore", TimeValue (Seconds(ueCxtStore)));
		enbRrc->SetAttribute ("SrsPeriodicity", UintegerValue (160));
	}

	lteHelper->AddX2Interface (smallcellEnbs);

	/****************************Mobility Models****************************/

	uint32_t numRwpUEs = m_numberOfRWP;
	NodeContainer nodeRwpUEs;
	nodeRwpUEs.Create(numRwpUEs);
	mobility.SetMobilityModel ("ns3::RandomEdgeHexagonalMobilityModel", //center of rect is (50,70)
			"Radius", DoubleValue (40),
			"CenterX", DoubleValue (0),
			"CenterY", DoubleValue (20),
			"MinSpeed", DoubleValue (ueMinSpeed), //5-7kmh
			"MaxSpeed", DoubleValue (ueMaxSpeed), //5-7kmh
			"PauseTime", DoubleValue (0));
	mobility.Install(nodeRwpUEs);
	for (NodeContainer::Iterator it = nodeRwpUEs.Begin ();  it != nodeRwpUEs.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceRwpUEs = lteHelper->InstallUeDevice (nodeRwpUEs);

	//	uint32_t numUEs = 120;
	uint32_t numBackForthUEs = m_numberOfManhattan;//10 * 17 + 20 = 190 UE in total
	//	NodeContainer nodeUEs;
	//	nodeUEs.Create(numUEs);
	//	mobility.SetMobilityModel ("ns3::RealMapMobilityModel", //center of rect is (50,70)
	////			"PauseTime", DoubleValue (ueCxtStore),
	//			"MaxSpeed", DoubleValue (ueMaxSpeed),
	//			"MinSpeed", DoubleValue (ueMinSpeed)); //3kmph
	//	mobility.Install(nodeUEs);
	//	for (NodeContainer::Iterator it = nodeUEs.Begin ();  it != nodeUEs.End (); ++it)
	//	{
	//		(*it)->Initialize ();
	//	}
	//	NetDeviceContainer deviceUEs = lteHelper->InstallUeDevice (nodeUEs);

	NodeContainer nodeBackForthUEs_1_2;
	nodeBackForthUEs_1_2.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(1).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(1).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_1_2);
	for (NodeContainer::Iterator it = nodeBackForthUEs_1_2.Begin ();  it != nodeBackForthUEs_1_2.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_1_2 = lteHelper->InstallUeDevice (nodeBackForthUEs_1_2);

	//	NodeContainer nodeBackForthUEs_17;
	//	nodeBackForthUEs_17.Create(numBackForthUEs);
	//	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
	//			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).x),
	//			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).y),
	//			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
	//			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
	//			"PauseTime", DoubleValue (ueCxtStore),
	//			"MaxSpeed", DoubleValue (ueMaxSpeed),
	//			"MinSpeed", DoubleValue (ueMinSpeed),
	//			"InitialTime", DoubleValue (200)); //3kmph
	//	mobility.Install(nodeBackForthUEs_17);
	//	for (NodeContainer::Iterator it = nodeBackForthUEs_17.Begin ();  it != nodeBackForthUEs_17.End (); ++it)
	//	{
	//		(*it)->Initialize ();
	//	}
	//	NetDeviceContainer deviceBackForthUEs_17 = lteHelper->InstallUeDevice (nodeBackForthUEs_17);

	NodeContainer nodeBackForthUEs_2_8;
	nodeBackForthUEs_2_8.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(7).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(7).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_2_8);
	for (NodeContainer::Iterator it = nodeBackForthUEs_2_8.Begin ();  it != nodeBackForthUEs_2_8.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_2_8 = lteHelper->InstallUeDevice (nodeBackForthUEs_2_8);

	NodeContainer nodeBackForthUEs_2_9;
	nodeBackForthUEs_2_9.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(8).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(8).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_2_9);
	for (NodeContainer::Iterator it = nodeBackForthUEs_2_9.Begin ();  it != nodeBackForthUEs_2_9.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_2_9 = lteHelper->InstallUeDevice (nodeBackForthUEs_2_9);

	NodeContainer nodeBackForthUEs_2_3;
	nodeBackForthUEs_2_3.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(2).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(2).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_2_3);
	for (NodeContainer::Iterator it = nodeBackForthUEs_2_3.Begin ();  it != nodeBackForthUEs_2_3.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_2_3 = lteHelper->InstallUeDevice (nodeBackForthUEs_2_3);

	NodeContainer nodeBackForthUEs_2_4;
	nodeBackForthUEs_2_4.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(1).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(3).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(3).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_2_4);
	for (NodeContainer::Iterator it = nodeBackForthUEs_2_4.Begin ();  it != nodeBackForthUEs_2_4.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_2_4 = lteHelper->InstallUeDevice (nodeBackForthUEs_2_4);

	NodeContainer nodeBackForthUEs_4_5;
	nodeBackForthUEs_4_5.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(3).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(3).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(4).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(4).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_4_5);
	for (NodeContainer::Iterator it = nodeBackForthUEs_4_5.Begin ();  it != nodeBackForthUEs_4_5.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_4_5 = lteHelper->InstallUeDevice (nodeBackForthUEs_4_5);

	NodeContainer nodeBackForthUEs_5_6;
	nodeBackForthUEs_5_6.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(5).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(5).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_5_6);
	for (NodeContainer::Iterator it = nodeBackForthUEs_5_6.Begin ();  it != nodeBackForthUEs_5_6.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_5_6 = lteHelper->InstallUeDevice (nodeBackForthUEs_5_6);

	//	NodeContainer nodeBackForthUEs_57;
	//	nodeBackForthUEs_57.Create(numBackForthUEs);
	//	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
	//			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).x),
	//			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).y),
	//			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
	//			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
	//			"PauseTime", DoubleValue (ueCxtStore),
	//			"MaxSpeed", DoubleValue (ueMaxSpeed),
	//			"MinSpeed", DoubleValue (ueMinSpeed),
	//			"InitialTime", DoubleValue (200)); //3kmph
	//	mobility.Install(nodeBackForthUEs_57);
	//	for (NodeContainer::Iterator it = nodeBackForthUEs_57.Begin ();  it != nodeBackForthUEs_57.End (); ++it)
	//	{
	//		(*it)->Initialize ();
	//	}
	//	NetDeviceContainer deviceBackForthUEs_57 = lteHelper->InstallUeDevice (nodeBackForthUEs_57);

	NodeContainer nodeBackForthUEs_5_1;
	nodeBackForthUEs_5_1.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(4).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(0).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(0).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_5_1);
	for (NodeContainer::Iterator it = nodeBackForthUEs_5_1.Begin ();  it != nodeBackForthUEs_5_1.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_5_1 = lteHelper->InstallUeDevice (nodeBackForthUEs_5_1);

	//	NodeContainer nodeBackForthUEs_87;
	//	nodeBackForthUEs_87.Create(numBackForthUEs);
	//	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
	//			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(7).x),
	//			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(7).y),
	//			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
	//			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
	//			"PauseTime", DoubleValue (ueCxtStore),
	//			"MaxSpeed", DoubleValue (ueMaxSpeed),
	//			"MinSpeed", DoubleValue (ueMinSpeed),
	//			"InitialTime", DoubleValue (200)); //3kmph
	//	mobility.Install(nodeBackForthUEs_87);
	//	for (NodeContainer::Iterator it = nodeBackForthUEs_87.Begin ();  it != nodeBackForthUEs_87.End (); ++it)
	//	{
	//		(*it)->Initialize ();
	//	}
	//	NetDeviceContainer deviceBackForthUEs_87 = lteHelper->InstallUeDevice (nodeBackForthUEs_87);

	NodeContainer nodeBackForthUEs_8_10;
	nodeBackForthUEs_8_10.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(7).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(7).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(9).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(9).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_8_10);
	for (NodeContainer::Iterator it = nodeBackForthUEs_8_10.Begin ();  it != nodeBackForthUEs_8_10.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_8_10 = lteHelper->InstallUeDevice (nodeBackForthUEs_8_10);

	//	NodeContainer nodeBackForthUEs_67;
	//	nodeBackForthUEs_67.Create(numBackForthUEs);
	//	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
	//			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(5).x),
	//			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(5).y),
	//			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
	//			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
	//			"PauseTime", DoubleValue (ueCxtStore),
	//			"MaxSpeed", DoubleValue (ueMaxSpeed),
	//			"MinSpeed", DoubleValue (ueMinSpeed),
	//			"InitialTime", DoubleValue (200)); //3kmph
	//	mobility.Install(nodeBackForthUEs_67);
	//	for (NodeContainer::Iterator it = nodeBackForthUEs_67.Begin ();  it != nodeBackForthUEs_67.End (); ++it)
	//	{
	//		(*it)->Initialize ();
	//	}
	//	NetDeviceContainer deviceBackForthUEs_67 = lteHelper->InstallUeDevice (nodeBackForthUEs_67);

	NodeContainer nodeBackForthUEs_9_3;
	nodeBackForthUEs_9_3.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(8).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(8).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(2).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(2).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_9_3);
	for (NodeContainer::Iterator it = nodeBackForthUEs_9_3.Begin ();  it != nodeBackForthUEs_9_3.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_9_3 = lteHelper->InstallUeDevice (nodeBackForthUEs_9_3);

	NodeContainer nodeBackForthUEs_9_10;
	nodeBackForthUEs_9_10.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(8).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(8).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(9).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(9).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_9_10);
	for (NodeContainer::Iterator it = nodeBackForthUEs_9_10.Begin ();  it != nodeBackForthUEs_9_10.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_9_10 = lteHelper->InstallUeDevice (nodeBackForthUEs_9_10);

	NodeContainer nodeBackForthUEs_11_1;
	nodeBackForthUEs_11_1.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(0).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_11_1);
	for (NodeContainer::Iterator it = nodeBackForthUEs_11_1.Begin ();  it != nodeBackForthUEs_11_1.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_11_1 = lteHelper->InstallUeDevice (nodeBackForthUEs_11_1);

	NodeContainer nodeBackForthUEs_6_11;
	nodeBackForthUEs_6_11.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(5).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(5).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_6_11);
	for (NodeContainer::Iterator it = nodeBackForthUEs_6_11.Begin ();  it != nodeBackForthUEs_6_11.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_6_11 = lteHelper->InstallUeDevice (nodeBackForthUEs_6_11);

	NodeContainer nodeBackForthUEs_7_11;
	nodeBackForthUEs_7_11.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(10).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_7_11);
	for (NodeContainer::Iterator it = nodeBackForthUEs_7_11.Begin ();  it != nodeBackForthUEs_7_11.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_7_11 = lteHelper->InstallUeDevice (nodeBackForthUEs_7_11);

	NodeContainer nodeBackForthUEs_12_1;
	nodeBackForthUEs_12_1.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(0).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(0).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_12_1);
	for (NodeContainer::Iterator it = nodeBackForthUEs_12_1.Begin ();  it != nodeBackForthUEs_12_1.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_12_1 = lteHelper->InstallUeDevice (nodeBackForthUEs_12_1);

	NodeContainer nodeBackForthUEs_12_7;
	nodeBackForthUEs_12_7.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(6).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_12_7);
	for (NodeContainer::Iterator it = nodeBackForthUEs_12_7.Begin ();  it != nodeBackForthUEs_12_7.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_12_7 = lteHelper->InstallUeDevice (nodeBackForthUEs_12_7);

	NodeContainer nodeBackForthUEs_12_8;
	nodeBackForthUEs_12_8.Create(numBackForthUEs);
	mobility.SetMobilityModel ("ns3::BackForthBetweenTwoPointsMobilityModel", //center of rect is (50,70)
			"X1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).x),
			"Y1", DoubleValue (eNBPosList->GetVectorPositionList().at(11).y),
			"X2", DoubleValue (eNBPosList->GetVectorPositionList().at(7).x),
			"Y2", DoubleValue (eNBPosList->GetVectorPositionList().at(7).y),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"InitialTime", DoubleValue (200)); //3kmph
	mobility.Install(nodeBackForthUEs_12_8);
	for (NodeContainer::Iterator it = nodeBackForthUEs_12_8.Begin ();  it != nodeBackForthUEs_12_8.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer deviceBackForthUEs_12_8 = lteHelper->InstallUeDevice (nodeBackForthUEs_12_8);


	/****************************Set up Epc Connections for all Devices****************************/
	Ipv4Address remoteHostAddr;
	NodeContainer ues;
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ipv4InterfaceContainer ueIpIfaces;
	Ptr<Node> remoteHost;
	NetDeviceContainer ueDevs;

	//  if (epc)
	//    {
	NS_LOG_LOGIC ("setting up internet and remote host");

	// Create a single RemoteHost
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create (1);
	remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);

	// Create the Internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
	Ptr<Node> pgw = epcHelper->GetPgwNode ();
	//Create Connection between remote host node and PGW node
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	// in this container, interface 0 is the pgw, 1 is the remoteHost
	remoteHostAddr = internetIpIfaces.GetAddress (1);

	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

	ues.Add (nodeRwpUEs);

	ues.Add (nodeBackForthUEs_1_2);
	//	ues.Add (nodeBackForthUEs_17);
	ues.Add (nodeBackForthUEs_11_1);
	ues.Add (nodeBackForthUEs_2_8);
	ues.Add (nodeBackForthUEs_2_9);
	ues.Add (nodeBackForthUEs_2_3);
	ues.Add (nodeBackForthUEs_2_4);
	ues.Add (nodeBackForthUEs_4_5);
	ues.Add (nodeBackForthUEs_5_6);
	//	ues.Add (nodeBackForthUEs_57);
	ues.Add (nodeBackForthUEs_5_1);
	//	ues.Add (nodeBackForthUEs_87);
	ues.Add (nodeBackForthUEs_8_10);
	//	ues.Add (nodeBackForthUEs_67);
	ues.Add (nodeBackForthUEs_9_3);
	ues.Add (nodeBackForthUEs_9_10);
	ues.Add (nodeBackForthUEs_6_11);
	ues.Add (nodeBackForthUEs_7_11);
	ues.Add (nodeBackForthUEs_12_1);
	ues.Add (nodeBackForthUEs_12_7);
	ues.Add (nodeBackForthUEs_12_8);



	ueDevs.Add (deviceRwpUEs);

	ueDevs.Add (deviceBackForthUEs_1_2);
	//	ueDevs.Add (deviceBackForthUEs_17);
	ueDevs.Add (deviceBackForthUEs_11_1);
	ueDevs.Add (deviceBackForthUEs_2_8);
	ueDevs.Add (deviceBackForthUEs_2_9);
	ueDevs.Add (deviceBackForthUEs_2_3);
	ueDevs.Add (deviceBackForthUEs_2_4);
	ueDevs.Add (deviceBackForthUEs_4_5);
	ueDevs.Add (deviceBackForthUEs_5_6);
	//	ueDevs.Add (deviceBackForthUEs_57);
	ueDevs.Add (deviceBackForthUEs_5_1);
	//	ueDevs.Add (deviceBackForthUEs_87);
	ueDevs.Add (deviceBackForthUEs_8_10);
	//	ueDevs.Add (deviceBackForthUEs_67);
	ueDevs.Add (deviceBackForthUEs_9_3);
	ueDevs.Add (deviceBackForthUEs_9_10);
	ueDevs.Add (deviceBackForthUEs_6_11);
	ueDevs.Add (deviceBackForthUEs_7_11);
	ueDevs.Add (deviceBackForthUEs_12_1);
	ueDevs.Add (deviceBackForthUEs_12_7);
	ueDevs.Add (deviceBackForthUEs_12_8);

	for(uint8_t ueID = 0; ueID < ueDevs.GetN(); ++ueID){
		Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(ueID)->GetObject<LteUeNetDevice> ();
		Ptr<LteUePhy> uePhy = lteUeDev->GetPhy ();
		NS_ASSERT(qInSync > qOutOfSync);
		//		uePhy->SetAttribute ("Qinsync", DoubleValue (qInSync));
		//		uePhy->SetAttribute ("Qoutofsync", DoubleValue (qOutOfSync));
		uePhy->SetAttribute("EnableUplinkPowerControl", BooleanValue(false));
		uePhy->SetAttribute("TxPower", DoubleValue(23)); //23dbm
		Ptr<LteUeRrc> ueRrc = lteUeDev->GetRrc();
		NS_ASSERT(ueRrc != NULL);
		if(ueID < m_numberOfRWP)
		{
			//		ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (2)); //SINR
			ueRrc->SetAttribute ("Qinsync", DoubleValue (-3));
			ueRrc->SetAttribute ("Qoutofsync", DoubleValue (-5));
			ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (measMetric)); //SINR
		}
		else
		{
			//		ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (2)); //SINR
			ueRrc->SetAttribute ("Qinsync", DoubleValue (qInSync));
			ueRrc->SetAttribute ("Qoutofsync", DoubleValue (qOutOfSync));
			ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (measMetric)); //SINR
		}

		//		ueRrc->SetAttribute ("SinrType",UintegerValue(sinrType));
		//		ueRrc->SetAttribute("EnableTracePosition",BooleanValue(true));
		//		ueRrc->SetAttribute("EnableTraceMeasurement",BooleanValue(true));
	}

	// Install the IP stack on the UEs
	internet.Install (ues);
	ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

	lteHelper->Attach(ueDevs);
	NS_LOG_LOGIC ("setting up applications");

	// Install and start applications on UEs and remote host
	uint16_t dlPort = 10000;
	uint16_t ulPort = 20000;

	// randomize a bit start times to avoid simulation artifacts
	// (e.g., buffer overflows due to packet transmissions happening
	// exactly at the same time)
	Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
	if (useUdp)
	{
		startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
		startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));
	}
	else
	{
		// TCP needs to be started late enough so that all UEs are connected
		// otherwise TCP SYN packets will get lost
		startTimeSeconds->SetAttribute ("Min", DoubleValue (0.100));
		startTimeSeconds->SetAttribute ("Max", DoubleValue (0.110));
	}

	for (uint32_t u = 0; u < ues.GetN (); ++u)
	{
		Ptr<Node> ue = ues.Get (u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

		for (uint32_t b = 0; b < numBearersPerUe; ++b)
		{
			++dlPort;
			++ulPort;

			ApplicationContainer clientApps;
			ApplicationContainer serverApps;

			if (useUdp)
			{
				if (epcDl)
				{
					NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
					UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
					clientApps.Add (dlClientHelper.Install (remoteHost));
					PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
							InetSocketAddress (Ipv4Address::GetAny (), dlPort));
					serverApps.Add (dlPacketSinkHelper.Install (ue));
				}
				if (epcUl)
				{
					NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
					UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
					clientApps.Add (ulClientHelper.Install (ue));
					PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
							InetSocketAddress (Ipv4Address::GetAny (), ulPort));
					serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
				}
			}
			else // use TCP
			{
				if (epcDl)
				{
					NS_LOG_LOGIC ("installing TCP DL app for UE " << u);
					BulkSendHelper dlClientHelper ("ns3::TcpSocketFactory",
							InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort));
					dlClientHelper.SetAttribute ("MaxBytes", UintegerValue (0));
					clientApps.Add (dlClientHelper.Install (remoteHost));
					PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory",
							InetSocketAddress (Ipv4Address::GetAny (), dlPort));
					serverApps.Add (dlPacketSinkHelper.Install (ue));
				}
				if (epcUl)
				{
					NS_LOG_LOGIC ("installing TCP UL app for UE " << u);
					BulkSendHelper ulClientHelper ("ns3::TcpSocketFactory",
							InetSocketAddress (remoteHostAddr, ulPort));
					ulClientHelper.SetAttribute ("MaxBytes", UintegerValue (0));
					clientApps.Add (ulClientHelper.Install (ue));
					PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory",
							InetSocketAddress (Ipv4Address::GetAny (), ulPort));
					serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
				}
			} // end if (useUdp)

			Ptr<EpcTft> tft = Create<EpcTft> (); //Create Traffic Flow Template
			if (epcDl)
			{
				EpcTft::PacketFilter dlpf;
				dlpf.localPortStart = dlPort;
				dlpf.localPortEnd = dlPort;
				tft->Add (dlpf);
			}
			if (epcUl)
			{
				EpcTft::PacketFilter ulpf;
				ulpf.remotePortStart = ulPort;
				ulpf.remotePortEnd = ulPort;
				tft->Add (ulpf);
			}

			if (epcDl || epcUl){
				//            //Set up default radio bearer
				//            enum EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
				//            EpsBearer bearer (q);
				//            lteHelper->ActivateDataRadioBearer (ueDevs.Get (u), bearer);

				//Set up dedicated radio bearer, should be you when epc is used
				EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
				lteHelper->ActivateDedicatedEpsBearer (ueDevs.Get (u), bearer, tft);
			}
			Time startTime = Seconds (startTimeSeconds->GetValue ());
			serverApps.Start (startTime);
			clientApps.Start (startTime);

		}
	}
	//  BuildingsHelper::MakeMobilityModelConsistent ();

	/****************************Set up Initial Parameters for CSON Algorithm****************************/
	std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap;
	LteEnbDSONSapProvider::MroParameters params;
	params.a3Offset = cell1A3Off; params.hyst = cell1Hys; params.ttt = cell1TTT; params.cioList = cioVals1;
	initialParamsMap[1] = params;

	params.a3Offset = cell2A3Off; params.hyst = cell2Hys; params.ttt = cell2TTT; params.cioList = cioVals2;
	initialParamsMap[2] = params;

	params.a3Offset = cell3A3Off; params.hyst = cell3Hys; params.ttt = cell3TTT; params.cioList = cioVals3;
	initialParamsMap[3] = params;

	params.a3Offset = cell4A3Off; params.hyst = cell4Hys; params.ttt = cell4TTT; params.cioList = cioVals4;
	initialParamsMap[4] = params;

	params.a3Offset = cell5A3Off; params.hyst = cell5Hys; params.ttt = cell5TTT; params.cioList= cioVals5;
	initialParamsMap[5] = params;

	params.a3Offset = cell6A3Off; params.hyst = cell6Hys; params.ttt = cell6TTT; params.cioList = cioVals6;
	initialParamsMap[6] = params;

	params.a3Offset = cell7A3Off; params.hyst = cell7Hys; params.ttt = cell7TTT; params.cioList = cioVals7;
	initialParamsMap[7] = params;

	params.a3Offset = cell8A3Off; params.hyst = cell8Hys; params.ttt = cell8TTT; params.cioList = cioVals8;
	initialParamsMap[8] = params;

	params.a3Offset = cell9A3Off; params.hyst = cell9Hys; params.ttt = cell9TTT; params.cioList = cioVals9;
	initialParamsMap[9] = params;

	params.a3Offset = cell10A3Off; params.hyst = cell10Hys; params.ttt = cell10TTT; params.cioList = cioVals10;
	initialParamsMap[10] = params;

	params.a3Offset = cell11A3Off; params.hyst = cell11Hys; params.ttt = cell11TTT; params.cioList = cioVals11;
	initialParamsMap[11] = params;

	params.a3Offset = cell12A3Off; params.hyst = cell12Hys; params.ttt = cell12TTT; params.cioList = cioVals12;
	initialParamsMap[12] = params;
	//		initialParamsMap[cellId];

	//Initialize Pthread
//	mroAlgorithm = CreateObject<CSONMroAlgorithm>(sceNBDevs,initialParamsMap);
	mroAlgorithm->SetParams(sceNBDevs,initialParamsMap);
	Ptr<MroStatisticalDataProcessor> mroStatDataProcessor = CreateObject<MroStatisticalDataProcessor>(mroAlgorithm);
	Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMroStatData",
			MakeBoundCallback (&MroStatisticalDataProcessor::RecvMroStaticalDataCallback,mroStatDataProcessor));
	//	Ptr<SystemThread> sysThread = Create<SystemThread>(MakeCallback(&CSONMroAlgorithm::MroAlgorithm, mroAlgorithm));
	//	sysThread->Start();

	NS_LOG_UNCOND("time\tcellId\t#LHOs\t#EHOs\t#WHOs\t#PHOs\t#Total\tKPI");

	Ptr<RadioEnvironmentMapHelper> remHelper;
	if (generateRem)
	{
		remHelper = CreateObject<RadioEnvironmentMapHelper> ();
		remHelper->SetAttribute ("Earfcn",UintegerValue(1575));
		remHelper->SetAttribute ("Bandwidth",UintegerValue(smallcellEnbBandwidth));
		remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
		remHelper->SetAttribute ("OutputFile", StringValue ("THANG1.rem"));
		remHelper->SetAttribute ("XMin", DoubleValue (0));
		remHelper->SetAttribute ("XMax", DoubleValue (100));
		remHelper->SetAttribute ("YMin", DoubleValue (0));
		remHelper->SetAttribute ("YMax", DoubleValue (100));
		remHelper->SetAttribute ("Z", DoubleValue (1.5));
		//		remHelper->SetAttribute ("UseDataChannel",BooleanValue(true));
		remHelper->SetAttribute ("XRes",UintegerValue(200));
		remHelper->Install ();
	}
	else
	{
		Simulator::Stop (Seconds (simTime));
	}

	Simulator::Run ();

	lteHelper = 0;
	Simulator::Destroy ();

}
