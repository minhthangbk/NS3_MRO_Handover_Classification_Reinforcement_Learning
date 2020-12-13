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
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MROSimulation");

class CSONMroAlgorithm : public Object{
public:
	CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap);
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
	std::map<uint16_t,SONCommon::PerformanceReport> m_hoReportList; //mapping between cellId-handover performance report  data, used for periodical report

	std::map<uint16_t, SONCommon::HandoverStatisticReport > m_hoStatisticList; //user for continuous report
	std::map<uint16_t, bool> m_triggerAlgorithm;//indicate if HO performance data is ready, used for continuous report
	std::map<uint16_t, uint8_t > m_neighborNum; //#of neighbor for all cell
	std::set<uint16_t> m_tttSet;//Configuration Set
	std::set<double> m_a3OffsetSet;
	std::set<int8_t> m_cioSet;

	std::list<double>  m_rrDb;//RLF rate database of network system

	double m_kpiTh;
	uint16_t m_sampleLen;

	mutable SystemMutex m_mutex;
	bool m_kpiFirstWrite;
	bool m_enAlm; //enable algorithm

	//	bool m_endSlideWindow;
	bool m_runMROFirstTime;
	RRInfo m_rrInfo;
	bool m_enKPISaveCons;
};

CSONMroAlgorithm::CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap){
	//Initial Setup
	NS_ASSERT(smallcellEnbDevs.GetN() > 0);
	m_smallcellEnbDevs = smallcellEnbDevs;
	NS_ASSERT(initialParamsMap.size() == smallcellEnbDevs.GetN());
	m_paramsMap = initialParamsMap;

	//Set up Standard Values for parameters
	m_tttSet.insert(0); m_tttSet.insert(40); m_tttSet.insert(64); m_tttSet.insert(80); m_tttSet.insert(100); m_tttSet.insert(128);
	m_tttSet.insert(160); m_tttSet.insert(256); m_tttSet.insert(320); m_tttSet.insert(480); m_tttSet.insert(512); m_tttSet.insert(640);
	m_tttSet.insert(1024); m_tttSet.insert(1280); m_tttSet.insert(2560); m_tttSet.insert(5120);

	m_neighborNum[1] = 6; m_neighborNum[2] = 3; m_neighborNum[3] = 3; m_neighborNum[4] = 3; //manually configured based on simulation topology
	m_neighborNum[5] = 3; m_neighborNum[6] = 3;	m_neighborNum[7] = 3;
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

	m_kpiTh = 0.05;//handover key performance indicatior
	m_sampleLen = 20;//sample length

	m_enAlm = true;//Enable algorithm

	Simulator::Schedule(Seconds(60),&CSONMroAlgorithm::TimeSlideWindowStart,this);
	//	m_endSlideWindow = false;
	m_runMROFirstTime = true;
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
	NS_LOG_LOGIC("At main, CSONMroAlgorithm::UpdateMroStatisticalData cellId " << cellId << " MRO Problem " << probInfo.problem);

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
			NS_FATAL_ERROR(" Wrong Message ");
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
				//NS_FATAL_ERROR(" Wrong Message ");
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
				//NS_FATAL_ERROR(" Wrong Message ");
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
				+m_hoStatisticList[cellId].numPingpongHos + m_hoStatisticList[cellId].numHOs);
		sumPP += m_hoStatisticList[cellId].numPingpongHos;

		if(m_enKPISaveCons == false)
		{
			SaveKPI2Log(curTime, cellId, m_hoStatisticList[cellId].numTooLateHos, m_hoStatisticList[cellId].numTooEarlyHos, m_hoStatisticList[cellId].numWrongCellHos,
					m_hoStatisticList[cellId].numPingpongHos,
					m_hoStatisticList[cellId].numHOs+m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos,
					(double)(m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos)/
					(m_hoStatisticList[cellId].numHOs+m_hoStatisticList[cellId].numTooLateHos+m_hoStatisticList[cellId].numTooEarlyHos+m_hoStatisticList[cellId].numWrongCellHos+m_hoStatisticList[cellId].numPingpongHos));
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

		SONCommon::HandoverStatisticReport updatedReport = m_hoStatisticList[cellId];
		std::map<uint16_t, SONCommon::HandoverStatisticReport>::iterator cellHOStatIt = m_hoStatisticList.find(cellId);
		uint16_t NumIssues = updatedReport.numTooEarlyHos + updatedReport.numTooLateHos + updatedReport.numWrongCellHos;// + updatedReport.numPingpongHos;
		if(m_enAlm == true)
		{

			NS_ASSERT(cellHOStatIt != m_hoStatisticList.end());
			std::map<uint16_t, LteEnbDSONSapProvider::MroParameters>::iterator cellParamIt = m_paramsMap.find(cellId);
			NS_ASSERT(cellParamIt != m_paramsMap.end());

			/**********************************Adjust TTT and CIO**********************************/
			//If all the neighboring cell has LHO/EHO or WHO, decrease/increase TTT
			bool allIsLHO = true;
			bool allIsEHOorWHO = true;
			uint8_t numNeighbor = 0;

			bool foundGoodCell = false;
			for (std::map<uint16_t,SONCommon::CellStatistic>::iterator itCell = (cellHOStatIt->second).hoStatistic.begin();
					(itCell != (cellHOStatIt->second).hoStatistic.end()) && (foundGoodCell == false); ++ itCell){
				if(m_paramsMap[cellId].cioList.find(itCell->first) == m_paramsMap[cellId].cioList.end())
				{
					m_paramsMap[cellId].cioList[itCell->first] = 0;
					m_paramsMap[itCell->first].cioList[cellId] = 0;
				}
				if(NumIssues == 0)
				{ //should not count good neighbor cell
					allIsEHOorWHO = false;
					allIsLHO = false;
					foundGoodCell = true; //once good cell is found, quickly quit the loop
				}
				else
				{
					if(itCell->second.m_numLHO >= itCell->second.m_numEHO + itCell->second.m_numWHO)
					{//
						allIsEHOorWHO = false;
						numNeighbor += 1;
					}
					else
					{
						allIsLHO = false;
						numNeighbor += 1;
					}
				}
			}

			std::set<uint16_t>::iterator setTTTIt = m_tttSet.find(cellParamIt->second.ttt);
			NS_ASSERT(setTTTIt != m_tttSet.end());
			NS_ASSERT(numNeighbor <= m_paramsMap[cellId].cioList.size());//At this time, there is no ANR
			if((numNeighbor == m_paramsMap[cellId].cioList.size()) && (allIsLHO == true))
			{//Decrease TTT until 40ms to avoid ping-pong
				NS_ASSERT(*(setTTTIt) > 0);
				uint16_t tttTemp = *(--setTTTIt);
				if(tttTemp > 0)
				{//Decrease
					cellParamIt->second.ttt = tttTemp; //decrease TTT
				}
			}
			else if((numNeighbor == m_paramsMap[cellId].cioList.size()) && (allIsEHOorWHO == true) )
			{//Increase TTT (m_paramsMap[cellId].cioList.size() - 1)
				NS_ASSERT(*(setTTTIt) <= 5120);
				if ((*(setTTTIt) < 5120))
				{
					cellParamIt->second.ttt = *(++setTTTIt); //increase TTT
				}
			}
			else
			{//some has LHO, some has EHO or WHO, some are good neighbors, treat them in different manners
				for(std::map<uint16_t,SONCommon::CellStatistic>::iterator itCell = (cellHOStatIt->second).hoStatistic.begin();
						itCell != (cellHOStatIt->second).hoStatistic.end(); ++ itCell)
				{
					if((itCell->second.m_numLHO + itCell->second.m_numEHO + itCell->second.m_numWHO) > 0)// + itCell->second.m_numPP) > 0)
					{
						std::set<int8_t>::iterator cioSetIt = m_cioSet.find(cellParamIt->second.cioList[itCell->first]);
						NS_ASSERT(cioSetIt != m_cioSet.end());
						if(itCell->second.m_numLHO >= (itCell->second.m_numEHO + itCell->second.m_numWHO))// + itCell->second.m_numPP))
						{//Ncell has LHO -> increase CIO
							//in future work, we need to negotiate Cell 1 and Cell 7 about channgin CIO values
							cellParamIt->second.cioList[itCell->first] = *(++cioSetIt); //CIO is already increased in the loop
						}
						else
						{//Ncell has EHO -> decrease CIO
							//CIO is already increased in the loop, we have to decrease it 2 times
							cellParamIt->second.cioList[itCell->first] = *(--cioSetIt);//(--(--itCIO))->second;
						}
					}
				}

				/**********************************Adjust CIOs and A3Offset**********************************/
				bool allCIOisPos = true;
				bool allCIOisNeg = true;
				double minCIO = 25;//out of CIO range
				double maxCIO = -25;
				for(std::map<uint16_t, int8_t>::iterator itCIO = cellParamIt->second.cioList.begin();
						(itCIO != cellParamIt->second.cioList.end()); ++itCIO){
					if((itCIO->second > 0))
					{
						allCIOisNeg = false;
						if(minCIO > itCIO->second)
						{
							minCIO = itCIO->second;
						}
					}
					else if(itCIO->second < 0)
					{
						allCIOisPos = false;
						if(maxCIO < itCIO->second)
						{
							maxCIO = itCIO->second;
						}
					}
					else
					{//itCIO->second == 0
						allCIOisNeg = false;
						allCIOisPos = false;
						break;
					}
				}

				NS_ASSERT((allCIOisPos && allCIOisNeg) == false);//mutual exclusive condition
				if(allCIOisPos == true)
				{//if all CIO values > 0, decrease it and increase a3 if possible(i.e new CIO and new a3offset is valid)
					bool possible = true;
					for(std::map<uint16_t, int8_t>::iterator itCIO = cellParamIt->second.cioList.begin();
							(itCIO != cellParamIt->second.cioList.end()) && (possible == true); ++itCIO)
					{
						if((m_cioSet.find(itCIO->second - minCIO) == m_cioSet.end()))
						{// && (itCIO->first != cellId)){//invalid CIO
							possible = false;
						}
					}
					if(m_a3OffsetSet.find(cellParamIt->second.a3Offset - minCIO) == m_a3OffsetSet.end())
					{
						possible = false;
					}
					if(possible == true)
					{
						for(std::map<uint16_t, int8_t>::iterator itCIO = cellParamIt->second.cioList.begin();
								(itCIO != cellParamIt->second.cioList.end()); ++itCIO)
						{
							itCIO->second -= minCIO;
						}
						cellParamIt->second.a3Offset -= minCIO;
					}
				}else if(allCIOisNeg == true)
				{//if all CIO values < 0, increase it and decrease a3 if possible(i.e new CIO and new a3offset is valid)
					//concern only neighboring cell, not all of the cells -> change later
					bool possible = true;
					for(std::map<uint16_t, int8_t>::iterator itCIO = cellParamIt->second.cioList.begin();
							(itCIO != cellParamIt->second.cioList.end()) && (possible == true); ++itCIO)
					{
						if((m_cioSet.find(itCIO->second - maxCIO) == m_cioSet.end()))
						{// && (itCIO->first != cellId)){//invalid CIO
							possible = false;
						}
					}
					if(m_a3OffsetSet.find(cellParamIt->second.a3Offset - maxCIO) == m_a3OffsetSet.end())
					{
						possible = false;
					}
					if(possible == true)
					{
						for(std::map<uint16_t, int8_t>::iterator itCIO = cellParamIt->second.cioList.begin();
								(itCIO != cellParamIt->second.cioList.end()); ++itCIO)
						{
							itCIO->second -= maxCIO;
						}
						cellParamIt->second.a3Offset -= maxCIO;
					}
				}
			}
			LteEnbDSONSapProvider::MroParameters mroParams = m_paramsMap[cellId];
			Ptr<LteEnbRrc> rrcPtr = m_smallcellEnbDevs.Get(cellId-1)->GetObject<LteEnbNetDevice>()->GetRrc();
			bool req = true;
			Simulator::ScheduleNow(&LteEnbRrc::RrcSetMroParams, rrcPtr, req, mroParams);
		}else
		{//End > ho_kpi || algorithm is not enable
			LteEnbDSONSapProvider::MroParameters mroParams = m_paramsMap[cellId];
			Ptr<LteEnbRrc> rrcPtr = m_smallcellEnbDevs.Get(cellId-1)->GetObject<LteEnbNetDevice>()->GetRrc();
			bool req = false;
			Simulator::ScheduleNow(&LteEnbRrc::RrcSetMroParams, rrcPtr, req, mroParams);
		}
		/**********************************Resset Memory**********************************/
		cellHOStatIt->second.hoStatistic.clear();
		cellHOStatIt->second.numHOs = 0;
		cellHOStatIt->second.numTooEarlyHos = 0;
		cellHOStatIt->second.numTooLateHos = 0;
		cellHOStatIt->second.numPingpongHos = 0;
		cellHOStatIt->second.numWrongCellHos = 0;
	}//End For
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
	case 1:	case 2: case 3: case 4: case 5: case 6: case 7:
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

	double cell1Hys = 3;    double cell1A3Off = 0;    uint64_t cell1TTT = 480;
	double cell2Hys = 3;    double cell2A3Off = 0;    uint64_t cell2TTT = 480;
	double cell3Hys = 3;    double cell3A3Off = 0;    uint64_t cell3TTT = 480;
	double cell4Hys = 3;    double cell4A3Off = 0;    uint64_t cell4TTT = 480;
	double cell5Hys = 3;    double cell5A3Off = 0;    uint64_t cell5TTT = 480;
	double cell6Hys = 3;    double cell6A3Off = 0;    uint64_t cell6TTT = 480;
	double cell7Hys = 3;    double cell7A3Off = 0;    uint64_t cell7TTT = 480;

	bool generateRem = false;
	double enbTxDbm=24; //24dBm for small cell, 46 dBm for macro cell
	double simTime = 60*15+1; //unit s
	double ueMinSpeed=1.39; //5kmph
	double ueMaxSpeed=4.17; //15kmph
	double ueCxtStore = 2;
	double qInSync = -10;
	double qOutOfSync = -12;
	bool epcDl = true; //Enable full-buffer DL traffic
	bool epcUl = false; //Enable full-buffer UL traffic
	bool useUdp = false; //Disable UDP traffic and enable TCP instead
	//  1.4MHz-6RBs, 3MHz-15RBs, 5MHz-25RBs, 10MHz-50RBs, 15MHz-75RBs, 20MHz-100RBs
	uint16_t smallcellEnbBandwidth=100;
	uint16_t numBearersPerUe = 1;
	double bsAntennaHeight = 5; //like lamp post
	NodeContainer smallcellEnbs;
	int8_t numberOfEnbs = 7;

	smallcellEnbs.Create (numberOfEnbs);

	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();

	//Get parameters from command line
	CommandLine cmd;
	cmd.AddValue("cell1Hys","cellid 1 hysteresis dB",cell1Hys);
	cmd.AddValue("cell1A3Off","cellid 1 a3Offset dB",cell1A3Off);
	cmd.AddValue("cell1TTT","cellid 1 time-to-trigger ms",cell1TTT);

	cmd.AddValue("cell2Hys","cellid 2 hysteresis dB",cell2Hys);
	cmd.AddValue("cell2A3Off","cellid 2 a3Offset dB",cell2A3Off);
	cmd.AddValue("cell2TTT","cellid 2 time-to-trigger ms",cell2TTT);

	cmd.AddValue("cell3Hys","cellid 3 hysteresis dB",cell3Hys);
	cmd.AddValue("cell3A3Off","cellid 3 a3Offset dB",cell3A3Off);
	cmd.AddValue("cell3TTT","cellid 3 time-to-trigger ms",cell3TTT);

	cmd.AddValue("cell4Hys","cellid 4 hysteresis dB",cell4Hys);
	cmd.AddValue("cell4A3Off","cellid 4 a3Offset dB",cell4A3Off);
	cmd.AddValue("cell4TTT","cellid 4 time-to-trigger ms",cell4TTT);

	cmd.AddValue("cell5Hys","cellid 5 hysteresis dB",cell5Hys);
	cmd.AddValue("cell5A3Off","cellid 5 a3Offset dB",cell5A3Off);
	cmd.AddValue("cell5TTT","cellid 5 time-to-trigger ms",cell5TTT);

	cmd.AddValue("cell6Hys","cellid 6 hysteresis dB",cell6Hys);
	cmd.AddValue("cell6A3Off","cellid 6 a3Offset dB",cell6A3Off);
	cmd.AddValue("cell6TTT","cellid 6 time-to-trigger ms",cell6TTT);

	cmd.AddValue("cell7Hys","cellid 7 hysteresis dB",cell7Hys);
	cmd.AddValue("cell7A3Off","cellid 7 a3Offset dB",cell7A3Off);
	cmd.AddValue("cell7TTT","cellid 7 time-to-trigger ms",cell7TTT);

	// parse again so you can override input file default values via command line
	cmd.Parse (argc, argv);

	/****************************Setup Channel Parameters****************************/
	lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");
	lteHelper->SetSpectrumChannelAttribute("ShadowingVariance",DoubleValue(36));

	lteHelper->SetAttribute("EnableIslandMode",BooleanValue(true));
	std::map< uint64_t, std::list< Ptr<IsLandProfile> > > islandList;
	Ptr<CircleIsland> islandWHO = CreateObject<CircleIsland>(35,50,2,17); //(X,Y,radius,gain)

	islandList[6].push_back(islandWHO);

	Ptr<CircleIsland> islandEHO1= CreateObject<CircleIsland>(55,45,2,22);//We have to create new object

	islandList[7].push_back(islandEHO1);
	//	lteHelper->SetIslandDownlinkSpectrum(islandList);

	Ptr<CircleIsland> islandEHO2= CreateObject<CircleIsland>(70,50,2,16);//We have to create new object
	islandList[1].push_back(islandEHO2);
	lteHelper->SetIslandDownlinkSpectrum(islandList);

	lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
	lteHelper->SetFadingModelAttribute ("TraceFilename",
			StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_10kmph.fad"));
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

	/****************************Set up eNB parameters****************************/
	eNBPosList->Add (Vector (50, 50, bsAntennaHeight)); //cellID 1
	eNBPosList->Add (Vector (80, 50, bsAntennaHeight)); //cellID 2
	eNBPosList->Add (Vector (65, 75.98, bsAntennaHeight)); //cellID 3
	eNBPosList->Add (Vector (35, 75.98, bsAntennaHeight));  //cellID 4
	eNBPosList->Add (Vector (20, 50, bsAntennaHeight));  //cellID 5
	eNBPosList->Add (Vector (35, 24, bsAntennaHeight));  //cellID 6
	eNBPosList->Add (Vector (65, 24, bsAntennaHeight));  //cellID 7
	mobility.SetPositionAllocator (eNBPosList);
	mobility.Install (smallcellEnbs);
	lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel"); //omni-directional antenna, gain is same 0db in all directions
	lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (1575));//downlink Frequency
	lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (1575)); //Downlink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (19575));//Uplink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (smallcellEnbBandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (smallcellEnbBandwidth));

	std::map<uint16_t,int8_t> cioVals1; cioVals1[2] = 0; cioVals1[3] = 0; cioVals1[4] = 0; cioVals1[5] = 0; cioVals1[6] = 0; cioVals1[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals1);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell1Hys)); //Enb 1
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell1A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell1TTT)));
	NetDeviceContainer sceNBDevs = lteHelper->InstallEnbDevice(smallcellEnbs.Get(0));
	sceNBDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals2; cioVals2[1] = 0; cioVals2[3] = 0; cioVals2[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals2);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell2Hys)); //Enb 2
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell2A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell2TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(1)));
	sceNBDevs.Get(1)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals3; cioVals3[1] = 0; cioVals3[2] = 0; cioVals3[4] = 0;
	lteHelper->SetInitialCIOValues(cioVals3);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell3Hys)); //Enb 3
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell3A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell3TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(2)));
	sceNBDevs.Get(2)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals4; cioVals4[1] = 0; cioVals4[3] = 0; cioVals4[5] = 0;
	lteHelper->SetInitialCIOValues(cioVals4);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell4Hys)); //Enb 4
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell4A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell4TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(3)));
	sceNBDevs.Get(3)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals5; cioVals5[1] = 0; cioVals5[4] = 0; cioVals5[6] = 0;
	lteHelper->SetInitialCIOValues(cioVals5);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell5Hys)); //Enb 5
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell5A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell5TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(4)));
	sceNBDevs.Get(4)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals6; cioVals6[1] = 0; cioVals6[5] = 0; cioVals6[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals6);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell6Hys)); //Enb 6
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell6A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell6TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(5)));
	sceNBDevs.Get(5)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals7; cioVals7[1] = 0; cioVals7[2] = 0; cioVals7[6] = 0;
	lteHelper->SetInitialCIOValues(cioVals7);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell7Hys)); //Enb 7
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell7A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell7TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(6)));
	sceNBDevs.Get(6)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	for(uint8_t enbID = 0; enbID < sceNBDevs.GetN(); ++enbID){
		Ptr<LteEnbNetDevice> lteEnbDev = sceNBDevs.Get(enbID)->GetObject<LteEnbNetDevice> ();
		Ptr<LteEnbRrc> enbRrc = lteEnbDev->GetRrc ();
		NS_ASSERT(enbRrc > NULL);
		enbRrc->SetAttribute ("Tuecxtstore", TimeValue (Seconds(ueCxtStore)));
		enbRrc->SetAttribute ("SrsPeriodicity", UintegerValue (160));
	}

	lteHelper->AddX2Interface (smallcellEnbs);

	/****************************Counter Clockwise and Clockwise Mobility****************************/
	uint32_t numCircleUes = 20;
	uint32_t numHexagonalUE = 80;
	uint32_t numWayPointUE1 = 5;
	uint32_t numWayPointUE2 = 5;
	uint32_t numWayPointUE3 = 5;

	NodeContainer cntClkwiseUe;
	NodeContainer clkwiseUe;
	cntClkwiseUe.Create (numCircleUes);
	clkwiseUe.Create (numCircleUes);

	mobility.SetMobilityModel ("ns3::CircleMobilityModel",
			"Radius", DoubleValue (22),
			"CenterX", DoubleValue (50),
			"CenterY", DoubleValue (50),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed)); //3kmph
	mobility.Install(cntClkwiseUe);
	NetDeviceContainer cntClkwiseUeDevs = lteHelper->InstallUeDevice (cntClkwiseUe);
	for (NodeContainer::Iterator it = cntClkwiseUe.Begin (); it != cntClkwiseUe.End (); ++it)
	{
		(*it)->Initialize ();
	}

	mobility.SetMobilityModel ("ns3::CircleMobilityModel",
			"Radius", DoubleValue (24),
			"CenterX", DoubleValue (50),
			"CenterY", DoubleValue (50),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed),
			"Clockwise", BooleanValue (true));
	mobility.Install(clkwiseUe);
	NetDeviceContainer clkwiseUeDevs = lteHelper->InstallUeDevice (clkwiseUe);
	for (NodeContainer::Iterator it = clkwiseUe.Begin (); it != clkwiseUe.End (); ++it)
	{
		(*it)->Initialize ();
	}

	NodeContainer waypointUE3;
	waypointUE3.Create(numHexagonalUE);

	mobility.SetMobilityModel ("ns3::RandomEdgeHexagonalMobilityModel", //center of rect is (50,70)
			"CenterX", DoubleValue (50),
			"CenterY", DoubleValue (50),
			"Radius", DoubleValue (25),
			"PauseTime", DoubleValue (ueCxtStore),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed)); //3kmph
	mobility.Install(waypointUE3);
	for (NodeContainer::Iterator it = waypointUE3.Begin ();  it != waypointUE3.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer waypointUeDevs3 = lteHelper->InstallUeDevice (waypointUE3);

	/****************************Back and Forth Mobility****************************/


	NodeContainer waypointUE1;//nearby island 1
	waypointUE1.Create (numWayPointUE1);

	mobility.SetMobilityModelExt1 ("ns3::RandomBackForthCrossBallMobilityModel",
			"MinX", DoubleValue (islandWHO->GetCenterX() - 10),
			"MaxX", DoubleValue (islandWHO->GetCenterX() + 10),
			"MinY", DoubleValue (islandWHO->GetCenterY() - 10),
			"MaxY", DoubleValue (islandWHO->GetCenterY() + 10),
			"PauseTime", DoubleValue (ueCxtStore),
			"BallCenterX", DoubleValue (islandWHO->GetCenterX()),
			"BallCenterY", DoubleValue (islandWHO->GetCenterY()),
			"BallRadius", DoubleValue (islandWHO->GetRadius()),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed)); //3kmph
	mobility.Install(waypointUE1);
	for (NodeContainer::Iterator it = waypointUE1.Begin ();  it != waypointUE1.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer waypointUeDevs1 = lteHelper->InstallUeDevice (waypointUE1);

	NodeContainer waypointUE2;//nearby island 2
	waypointUE2.Create (numWayPointUE2);

	mobility.SetMobilityModelExt1 ("ns3::RandomBackForthCrossBallMobilityModel",
			"MinX", DoubleValue (islandEHO1->GetCenterX() - 10),
			"MaxX", DoubleValue (islandEHO1->GetCenterX() + 10),
			"MinY", DoubleValue (islandEHO1->GetCenterY() - 10),
			"MaxY", DoubleValue (islandEHO1->GetCenterY() + 10),
			"PauseTime", DoubleValue (ueCxtStore),
			"BallCenterX", DoubleValue (islandEHO1->GetCenterX()),
			"BallCenterY", DoubleValue (islandEHO1->GetCenterY()),
			"BallRadius", DoubleValue (islandEHO1->GetRadius()),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed));
	mobility.Install(waypointUE2);
	for (NodeContainer::Iterator it = waypointUE2.Begin ();  it != waypointUE2.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer waypointUeDevs2 = lteHelper->InstallUeDevice (waypointUE2);

	NodeContainer waypointUE4;//nearby island 3
	waypointUE4.Create (numWayPointUE3);

	mobility.SetMobilityModelExt1 ("ns3::RandomBackForthCrossBallMobilityModel",
			"MinX", DoubleValue (islandEHO2->GetCenterX() - 10),
			"MaxX", DoubleValue (islandEHO2->GetCenterX() + 10),
			"MinY", DoubleValue (islandEHO2->GetCenterY() - 10),
			"MaxY", DoubleValue (islandEHO2->GetCenterY() + 10),
			"PauseTime", DoubleValue (ueCxtStore),
			"BallCenterX", DoubleValue (islandEHO2->GetCenterX()),
			"BallCenterY", DoubleValue (islandEHO2->GetCenterY()),
			"BallRadius", DoubleValue (islandEHO2->GetRadius()),
			"MaxSpeed", DoubleValue (ueMaxSpeed),
			"MinSpeed", DoubleValue (ueMinSpeed));
	mobility.Install(waypointUE4);
	for (NodeContainer::Iterator it = waypointUE4.Begin ();  it != waypointUE4.End (); ++it)
	{
		(*it)->Initialize ();
	}
	NetDeviceContainer waypointUeDevs4 = lteHelper->InstallUeDevice (waypointUE4);

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

	ues.Add (cntClkwiseUe);
	ues.Add (clkwiseUe);
	ues.Add (waypointUE1);
	ues.Add (waypointUE2);
	ues.Add (waypointUE3);
	ues.Add (waypointUE4);

	ueDevs.Add (cntClkwiseUeDevs);
	ueDevs.Add (clkwiseUeDevs);
	ueDevs.Add (waypointUeDevs1);
	ueDevs.Add (waypointUeDevs2);
	ueDevs.Add (waypointUeDevs3);
	ueDevs.Add (waypointUeDevs4);

	for(uint8_t ueID = 0; ueID < ueDevs.GetN(); ++ueID){
		Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(ueID)->GetObject<LteUeNetDevice> ();
		Ptr<LteUePhy> uePhy = lteUeDev->GetPhy ();
		NS_ASSERT(qInSync > qOutOfSync);
		uePhy->SetAttribute ("Qinsync", DoubleValue (qInSync));
		uePhy->SetAttribute ("Qoutofsync", DoubleValue (qOutOfSync));
		uePhy->SetAttribute("EnableUplinkPowerControl", BooleanValue(false));
		uePhy->SetAttribute("TxPower", DoubleValue(23)); //23dbm
		ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (1)); //RSRQ
//		Ptr<LteUeRrc> ueRrc = lteUeDev->GetRrc();
//		NS_ASSERT(ueRrc != NULL);
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
	//		initialParamsMap[cellId];

	//Initialize Pthread
	Ptr<CSONMroAlgorithm> mroAlgorithm = CreateObject<CSONMroAlgorithm>(sceNBDevs,initialParamsMap);
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
