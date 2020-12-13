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
#include <sys/stat.h> //For creating directories
//#include "RlfModel_Validation_1.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RlfModelValidation");

class CSONMroAlgorithm : public Object{
public:
	CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap,
			std::string name, std::string savingFolderName);
	virtual ~CSONMroAlgorithm();
	void MroAlgorithm();
	void UpdateMroStatisticalData(uint16_t cellId, SONCommon::MROProblemInfo probInfo);
	void SaveKPI2Log(double curTime, uint16_t cellId, uint16_t loh, uint16_t eho, uint16_t who, uint16_t pho, uint16_t total, double kpi);
	void SaveRlfPP2Log(double curTime, double rrr, double rr);
	void SaveRR2Log(double curTime, double consRR);
	void SaveDetailMroKPI(double curTime, uint16_t sCellId, uint16_t tCellId, uint16_t loh, uint16_t eho, uint16_t who, uint16_t pho, uint16_t total);
	void TimeSlideWindowStart();
	void UpdateRR(double time, bool rlf);

	void SaveA3MeasReport(uint16_t reportCellId, uint16_t targetCellId, uint16_t reportRNTI, LteRrcSap::MeasResults measResults);
	void SaveMeasurementInfo(uint16_t cellId, SONCommon::MROProblemInfo probInfo);

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

	std::string m_name;
	std::string m_savingFolderName;
};

CSONMroAlgorithm::CSONMroAlgorithm(NetDeviceContainer smallcellEnbDevs, std::map<uint16_t,LteEnbDSONSapProvider::MroParameters> initialParamsMap,
		std::string name, std::string savingFolderName)
{
	//Initial Setup
	NS_ASSERT(smallcellEnbDevs.GetN() > 0);
	m_smallcellEnbDevs = smallcellEnbDevs;
	NS_ASSERT(initialParamsMap.size() == smallcellEnbDevs.GetN());
	m_paramsMap = initialParamsMap;
	m_name = name;
	m_savingFolderName = savingFolderName;
	//Set up Standard Values for parameters
	m_tttSet.insert(0); m_tttSet.insert(40); m_tttSet.insert(64); m_tttSet.insert(80); m_tttSet.insert(100); m_tttSet.insert(128);
	m_tttSet.insert(160); m_tttSet.insert(256); m_tttSet.insert(320); m_tttSet.insert(480); m_tttSet.insert(512); m_tttSet.insert(640);
	m_tttSet.insert(1024); m_tttSet.insert(1280); m_tttSet.insert(2560); m_tttSet.insert(5120);

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

	m_enAlm = false;//Enable algorithm

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
		case SONCommon::RLF_CONNECTION_DROP:
		{
			//			(dbIt->second).numRLFConnDrops += 1;
			//				hoStatNCell.m_numRCD += 1;
			NS_LOG_UNCOND("RLF_CONNECTION_DROP, cellId: " << cellId << " imsi: " << probInfo.imsi);
			break;
		}
		default:
		{
			NS_FATAL_ERROR(" Wrong Message: " << probInfo.problem);
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
			case SONCommon::RLF_CONNECTION_DROP:
			{
				//			(dbIt->second).numRLFConnDrops += 1;
				//				hoStatNCell.m_numRCD += 1;
				NS_LOG_UNCOND("RLF_CONNECTION_DROP, cellId: " << cellId << " imsi: " << probInfo.imsi);
				break;
			}
			default:
			{
				NS_FATAL_ERROR(" Wrong Message ");
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
			case SONCommon::RLF_CONNECTION_DROP:
			{
				//			(dbIt->second).numRLFConnDrops += 1;
				//				hoStatNCell.m_numRCD += 1;
				NS_LOG_UNCOND("RLF_CONNECTION_DROP, cellId: " << cellId << " imsi: " << probInfo.imsi);
				break;
			}
			default:
			{
				NS_FATAL_ERROR(" Wrong Message ");
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
CSONMroAlgorithm::SaveA3MeasReport(uint16_t cellId, uint16_t targetCellId, uint16_t imsi, LteRrcSap::MeasResults measResults)
{
	std::ofstream outFile;

	outFile.open ((m_savingFolderName+"/"+m_name+"_HOInfFactor.txt").c_str(),  std::ios_base::app);

	if (!outFile.is_open ())
	{
		NS_LOG_ERROR ("Can't open the file ");
		return;
	}

	outFile << Simulator::Now ().GetMilliSeconds() << "\t";
	//		outFile << pos.x << "\t" << pos.y << "\t";
	outFile << imsi << "\t";
	outFile << cellId << "\t"; //Serving cellId
	outFile << targetCellId << "\t"; //neighboring cell

	double interferenceFactor = 0;
	double targetCellRSRPLinear = 0;
	for(std::list<LteRrcSap::MeasResultEutra>::iterator measIt = measResults.measResultListEutra.begin();
			measIt != measResults.measResultListEutra.end(); measIt++)
	{
		if(measIt->physCellId != cellId)
		{
			if(measIt->physCellId == targetCellId)
			{
				targetCellRSRPLinear = pow10(measIt->rsrpResult/10);
			}
			else
			{
				interferenceFactor += pow10(measIt->rsrpResult/10);
			}
		}
	}
	interferenceFactor = 1 + interferenceFactor/targetCellRSRPLinear;
	outFile << interferenceFactor << "\t";
	outFile << 10*log10(interferenceFactor) << std::endl; //dB
	outFile.close ();
}

void
CSONMroAlgorithm::SaveMeasurementInfo(uint16_t cellId, SONCommon::MROProblemInfo proInfo)
{
//	Vector pos;//=Vector (0, 0, 0);
//	Ptr<LteUeNetDevice> ueDev;
//	NodeList::Iterator listEnd = NodeList::End();
//	bool foundUe = false;
//	for (NodeList::Iterator it = NodeList::Begin(); (it != listEnd) && (!foundUe); ++it){
//		Ptr<Node> node = *it;
//		int nDevs = node->GetNDevices();
//		for(int j = 0; (j<nDevs) && (!foundUe); j++){
//			ueDev = node->GetDevice(j)->GetObject<LteUeNetDevice>();
//			if(ueDev == 0){
//				continue;
//			}else{
//				if(ueDev->GetImsi() == proInfo.imsi){//this is associated enb
//					pos = node->GetObject<MobilityModel>()->GetPosition();
//					foundUe = true;
//				}
//			}
//		}
//	}

	switch(proInfo.problem)
	{
	case SONCommon::TOO_LATE_HANDOVER:
	case SONCommon::TOO_EARLY_HANDOVER:
	case SONCommon::WRONG_CELL_HANDOVER:
	{
		std::ofstream outFile;
		if(proInfo.problem == SONCommon::TOO_LATE_HANDOVER)
		{
			outFile.open ((m_savingFolderName+"/"+m_name+"_TLInfFactor.txt").c_str(),  std::ios_base::app);

		}
		else if(proInfo.problem == SONCommon::TOO_EARLY_HANDOVER)
		{
			outFile.open ((m_savingFolderName+"/"+m_name+"_TEInfFactor.txt").c_str(),  std::ios_base::app);
		}
		else
		{
			outFile.open ((m_savingFolderName+"/"+m_name+"_WCInfFactor.txt").c_str(),  std::ios_base::app);
		}
		if (!outFile.is_open ())
		{
			NS_LOG_ERROR ("Can't open the file ");
			return;
		}

		outFile << Simulator::Now ().GetMilliSeconds() << "\t";
//		outFile << pos.x << "\t" << pos.y << "\t";
		outFile << proInfo.imsi << "\t";
		outFile << cellId << "\t"; //Serving cellId
		outFile << proInfo.cellId << "\t"; //problem neighboring cell

		double interferenceFactor = 0;
		double targetCellRSRPLinear = 0;
		for(std::list<LteRrcSap::MeasResultEutra>::iterator measIt = proInfo.measData.begin();
				measIt != proInfo.measData.end(); measIt++)
		{
			if(measIt->physCellId != cellId)
			{
				if(measIt->physCellId == proInfo.cellId)
				{
					targetCellRSRPLinear = pow10(measIt->rsrpResult/10);
				}
				else
				{
					interferenceFactor += pow10(measIt->rsrpResult/10);
				}
			}
		}
		interferenceFactor = 1 + interferenceFactor/targetCellRSRPLinear;
		outFile << interferenceFactor << "\t";
		outFile << 10*log10(interferenceFactor) << std::endl; //dB
		outFile.close ();
		break;
	}
	//TODO: How to trace measurements for too-early/wrong-cell and ping-pong/normal handover
//	case SONCommon::TOO_EARLY_HANDOVER:
//	{
//
//		break;
//	}
//	case SONCommon::WRONG_CELL_HANDOVER:
//	{
//
//		break;
//	}
//	case SONCommon::PING_PONG_HANDOVER:
//	{
//
//		break;
//	}
//	case SONCommon::NORMAL_HANDOVER:
//	{
//
//		break;
//	}
//	case SONCommon::RLF_CONNECTION_DROP:
//	{
//
//		break;
//	}
	default:
	{
//		NS_FATAL_ERROR(" Wrong Message ");
	}
	}
}

void
CSONMroAlgorithm::SaveKPI2Log(double curTime, uint16_t cellId, uint16_t lho, uint16_t eho, uint16_t who,
		uint16_t pho, uint16_t total, double kpi){
	std::ostringstream converter;//, converter1; //convert integer to string
	converter << cellId;
//	converter1 << m_simId;
	std::string fileName = m_savingFolderName+"/"+m_name+"_"+"KPICellId"+converter.str()+".txt";
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
CSONMroAlgorithm::SaveRlfPP2Log(double curTime, double ppr, double rr)
{
//	std::ostringstream converter;
//	converter << m_simId;
	std::string fileName = m_savingFolderName+"/"+m_name+"_"+"RRR"+".txt";
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
CSONMroAlgorithm::SaveDetailMroKPI(double curTime, uint16_t sCellId, uint16_t tCellId, uint16_t lho, uint16_t eho, uint16_t who, uint16_t pho, uint16_t normalHO){
	std::ostringstream converter;//,converter1; //convert integer to string
	converter << sCellId;
//	converter1 << m_simId;
	std::string fileName = m_savingFolderName+"/"+m_name+"_"+"DetailKPICellId"+converter.str()+".txt";
	std::ofstream outFile;

	outFile.open (fileName.c_str(),  std::ios_base::app);
	if (!outFile.is_open ()){
		NS_LOG_ERROR ("Can't open file " << fileName.c_str());
		return;
	}

	outFile << curTime << "\t" << sCellId << "\t" << tCellId << "\t" << lho << "\t" << eho << "\t" << who << "\t" << pho << "\t" << normalHO << std::endl;
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
				SaveDetailMroKPI(curTime, cellId, itCell->first, itCell->second.m_numLHO, itCell->second.m_numEHO,
						itCell->second.m_numWHO, itCell->second.m_numPP, itCell->second.m_numNHO);
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
		}
		else
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
	static void RecvA3MeasReport(Ptr<MroStatisticalDataProcessor> outputProcessor, std::string path,
					uint16_t cellId, uint16_t targetCellId, uint16_t rnti, LteRrcSap::MeasResults measResults);
	void SaveA3MeasReport(uint16_t cellId, uint16_t targetCellId, uint16_t rnti, LteRrcSap::MeasResults measResults);
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

void
MroStatisticalDataProcessor::RecvA3MeasReport(Ptr<MroStatisticalDataProcessor> outputProcessor, std::string path,
		uint16_t cellId, uint16_t targetCellId, uint16_t rnti, LteRrcSap::MeasResults measResults)
{
	outputProcessor->SaveA3MeasReport(cellId, targetCellId, rnti, measResults);
}

void
MroStatisticalDataProcessor::SaveA3MeasReport(uint16_t cellId, uint16_t targetCellId, uint16_t rnti, LteRrcSap::MeasResults measResults)
{
	m_mroAlgorithm->SaveA3MeasReport(cellId, targetCellId, rnti, measResults);
}
/*
 * Validation Environment
 */
class RlfModelValidationCase : public Object
{
public:
	RlfModelValidationCase();
	virtual ~RlfModelValidationCase();
	void ValidationCase(uint64_t ttt1, double cio12, uint64_t ttt2, double cio21, double speed);
	void ValidationCase(uint64_t ttt1, double cio12, double a3Offset1, uint64_t ttt2, double cio21, double a3Offset2, double speed, double gammaAngle);
	void ValidationCase(uint64_t ttt1, double cio12, double a3Offset1, uint64_t ttt2, double cio21, double a3Offset2, double speed, double qInsync, double qOutOfSync, double shadowing);

	void RunValidation(std::string name, std::string savingFolderName); //std::string name;
	uint64_t m_ttt1;
	double   m_cio12;
	double   m_a3Offset1;
	uint64_t m_ttt2;
	double   m_cio21;
	double   m_a3Offset2;
	double   m_speed;
	double	 m_gammaAngle;
//	uint16_t m_simId;
	double   m_qInSync;
	double   m_qOutOfSync;
	double   m_shadowing;
};

RlfModelValidationCase::RlfModelValidationCase()
{
	m_ttt1 = 480;
	m_cio12 = 3;
	m_a3Offset1 = 0;

	m_ttt2 = 480;
	m_cio21 = 3;
	m_speed = 1;
	m_a3Offset2 = 0;
}

RlfModelValidationCase::~RlfModelValidationCase()
{}

void
RlfModelValidationCase::ValidationCase(uint64_t ttt1, double cio12, uint64_t ttt2, double cio21, double speed)
{
	m_ttt1 = ttt1;
	m_cio12 = cio12;
	m_a3Offset1 = 0;
	m_ttt2 = ttt2;
	m_cio21 = cio21;
	m_speed = speed;
	m_a3Offset2 = 0;
}

void
RlfModelValidationCase::ValidationCase(uint64_t ttt1, double cio12, double a3Offset1, uint64_t ttt2, double cio21, double a3Offset2, double speed, double gammaAngle)
{
	m_ttt1 = ttt1;
	m_cio12 = cio12;
	m_a3Offset1 = a3Offset1;
	m_ttt2 = ttt2;
	m_cio21 = cio21;
	m_speed = speed;
	m_a3Offset2 = a3Offset2;
	m_gammaAngle = gammaAngle;
}

void
RlfModelValidationCase::ValidationCase(uint64_t ttt1, double cio12, double a3Offset1, uint64_t ttt2, double cio21, double a3Offset2, double speed,
		double qInsync, double qOutOfSync, double shadowing)
{
	m_ttt1 = ttt1;
	m_cio12 = cio12;
	m_a3Offset1 = a3Offset1;
	m_ttt2 = ttt2;
	m_cio21 = cio21;
	m_speed = speed;
	m_a3Offset2 = a3Offset2;
	m_qInSync = qInsync;
	m_qOutOfSync = qOutOfSync;
	m_shadowing = shadowing;
}

void
RlfModelValidationCase::RunValidation(std::string name, std::string savingFolderName)
{
	GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
	Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
	Config::SetDefault ("ns3::UdpClient::PacketSize", UintegerValue(320));//256kbps
	Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024));
	Config::SetDefault ("ns3::RrFfMacScheduler::HarqEnabled", BooleanValue (false)); //Disable HARQ
	Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
	Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
	Config::SetDefault ("ns3::LteHelper::AnrEnabled", BooleanValue (false)); //Used A4 of ANR function
	Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (false)); //AccumulationEnabled
	Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (false)); //

	double cell1Hys = 0;    double cell1A3Off = m_a3Offset1;    			uint64_t cell1TTT = m_ttt1;
	double cell2Hys = 0;    double cell2A3Off = m_a3Offset2;    			uint64_t cell2TTT = m_ttt2;
//	double cell3Hys = 3;    double cell3A3Off = 0;    			uint64_t cell3TTT = 480;
//	double cell4Hys = 3;    double cell4A3Off = 0;    			uint64_t cell4TTT = 480;
//	double cell5Hys = 3;    double cell5A3Off = 0;    			uint64_t cell5TTT = 480;
//	double cell6Hys = 3;    double cell6A3Off = 0;    			uint64_t cell6TTT = 480;
//	double cell7Hys = 3;    double cell7A3Off = 0;    			uint64_t cell7TTT = 480;
//	double cell8Hys = 3;    double cell8A3Off = 0;    			uint64_t cell8TTT = 480;
//	double cell9Hys = 3;    double cell9A3Off = 0;    			uint64_t cell9TTT = 480;
//	double cell10Hys = 3;   double cell10A3Off = 0;    			uint64_t cell10TTT = 480;

	bool generateRem = false;
	double enbTxDbm=24; //24dBm for small cell, 46 dBm for macro cell
	double simTime = 60*5+1; //unit s
	double ueMinSpeed = m_speed;//1.39; //5kmph
	double ueMaxSpeed = m_speed;//4.17; //15kmph
	double ueCxtStore = 2;
	double qInSync = m_qInSync;
	double qOutOfSync = m_qOutOfSync;
	bool epcDl = true; //Enable full-buffer DL traffic
	bool epcUl = false; //Enable full-buffer UL traffic
	bool useUdp = false; //Disable UDP traffic and enable TCP instead
	//  1.4MHz-6RBs, 3MHz-15RBs, 5MHz-25RBs, 10MHz-50RBs, 15MHz-75RBs, 20MHz-100RBs


	uint16_t smallcellEnbBandwidth=100;
	uint16_t numBearersPerUe = 1;
	double bsAntennaHeight = 5; //like lamp post
	NodeContainer smallcellEnbs;
	int8_t numberOfEnbs = 2;//10;
	smallcellEnbs.Create (numberOfEnbs);

	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();

	/****************************Setup Channel Parameters****************************/
	lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");
	lteHelper->SetSpectrumChannelAttribute("ShadowingVariance",DoubleValue(m_shadowing*m_shadowing));

//	lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
//	lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
//	lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
//	lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
//	lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
//	lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
//	lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (smallcellEnbBandwidth));

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
//	eNBPosList->Add (Vector (65, 76, bsAntennaHeight)); //cellID 3
//	eNBPosList->Add (Vector (35, 76, bsAntennaHeight));  //cellID 4
//	eNBPosList->Add (Vector (20, 50, bsAntennaHeight));  //cellID 5
//	eNBPosList->Add (Vector (35, 24, bsAntennaHeight));  //cellID 6
//	eNBPosList->Add (Vector (65, 24, bsAntennaHeight));  //cellID 7
//	eNBPosList->Add (Vector (95, 76, bsAntennaHeight));  //cellID 8
//	eNBPosList->Add (Vector (110, 50, bsAntennaHeight));  //cellID 9
//	eNBPosList->Add (Vector (95, 24, bsAntennaHeight));  //cellID 10
	mobility.SetPositionAllocator (eNBPosList);
	mobility.Install (smallcellEnbs);
	lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel"); //omni-directional antenna, gain is same 0db in all directions
	lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (1575));//downlink Frequency
	lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (1575)); //Downlink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (19575));//Uplink Frequency for eNB
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (smallcellEnbBandwidth));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (smallcellEnbBandwidth));

	std::map<uint16_t,int8_t> cioVals1; cioVals1[2] = m_cio12; cioVals1[3] = 0; cioVals1[4] = 0; cioVals1[5] = 0; cioVals1[6] = 0; cioVals1[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals1);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell1Hys)); //Enb 1
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell1A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell1TTT)));
	NetDeviceContainer sceNBDevs = lteHelper->InstallEnbDevice(smallcellEnbs.Get(0));
	sceNBDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	std::map<uint16_t,int8_t> cioVals2; cioVals2[1] = m_cio21; cioVals2[3] = 0; cioVals2[7] = 0;
	lteHelper->SetInitialCIOValues(cioVals2);
	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell2Hys)); //Enb 2
	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell2A3Off));
	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell2TTT)));
	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(1)));
	sceNBDevs.Get(1)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

//	std::map<uint16_t,int8_t> cioVals3; cioVals3[1] = 0; cioVals3[2] = 0; cioVals3[4] = 0;
//	lteHelper->SetInitialCIOValues(cioVals3);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell3Hys)); //Enb 3
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell3A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell3TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(2)));
//	sceNBDevs.Get(2)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals4; cioVals4[1] = 0; cioVals4[3] = 0; cioVals4[5] = 0;
//	lteHelper->SetInitialCIOValues(cioVals4);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell4Hys)); //Enb 4
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell4A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell4TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(3)));
//	sceNBDevs.Get(3)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals5; cioVals5[1] = 0; cioVals5[4] = 0; cioVals5[6] = 0;
//	lteHelper->SetInitialCIOValues(cioVals5);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell5Hys)); //Enb 5
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell5A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds(cell5TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(4)));
//	sceNBDevs.Get(4)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals6; cioVals6[1] = 0; cioVals6[5] = 0; cioVals6[7] = 0;
//	lteHelper->SetInitialCIOValues(cioVals6);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell6Hys)); //Enb 6
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell6A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell6TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(5)));
//	sceNBDevs.Get(5)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals7; cioVals7[1] = 0; cioVals7[2] = 0; cioVals7[6] = 0;
//	lteHelper->SetInitialCIOValues(cioVals7);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell7Hys)); //Enb 7
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell7A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell7TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(6)));
//	sceNBDevs.Get(6)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals8; cioVals8[2] = 0; cioVals8[3] = 0; cioVals8[9] = 0;
//	lteHelper->SetInitialCIOValues(cioVals8);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell8Hys)); //Enb 8
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell8A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell8TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(7)));
//	sceNBDevs.Get(7)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals9; cioVals9[2] = 0; cioVals9[8] = 0; cioVals9[10] = 0;
//	lteHelper->SetInitialCIOValues(cioVals9);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell9Hys)); //Enb 9
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell9A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell9TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(8)));
//	sceNBDevs.Get(8)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);
//
//	std::map<uint16_t,int8_t> cioVals10; cioVals10[2] = 0; cioVals10[7] = 0; cioVals10[9] = 0;
//	lteHelper->SetInitialCIOValues(cioVals10);
//	lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",DoubleValue(cell10Hys)); //Enb 8
//	lteHelper->SetHandoverAlgorithmAttribute("A3Offset",DoubleValue(cell10A3Off));
//	lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",TimeValue (MilliSeconds (cell10TTT)));
//	sceNBDevs.Add(lteHelper->InstallEnbDevice(smallcellEnbs.Get(9)));
//	sceNBDevs.Get(9)->GetObject<LteEnbNetDevice>()->GetPhy()->SetTxPower(enbTxDbm);

	for(uint8_t enbID = 0; enbID < sceNBDevs.GetN(); ++enbID){
		Ptr<LteEnbNetDevice> lteEnbDev = sceNBDevs.Get(enbID)->GetObject<LteEnbNetDevice> ();
		Ptr<LteEnbRrc> enbRrc = lteEnbDev->GetRrc ();
		NS_ASSERT(enbRrc > NULL);
		enbRrc->SetAttribute ("Tuecxtstore", TimeValue (Seconds(ueCxtStore)));
		enbRrc->SetAttribute ("SrsPeriodicity", UintegerValue (160));
	}

	lteHelper->AddX2Interface (smallcellEnbs);


	/****************************Back and Forth Mobility****************************/
//	Ptr<ListPositionAllocator> backgroundTrafficUEPosList = CreateObject<ListPositionAllocator> ();
//	Ptr<RandomDiscPositionAllocator> rndPosGen = CreateObject<RandomDiscPositionAllocator> ();
//	rndPosGen->SetMaxRho(5);
//	rndPosGen->SetMinRho(3);
//	uint16_t numFixUEs = 0;
//	rndPosGen->SetX(eNBPosList->GetVectorPositionList().at(0).x);
//	rndPosGen->SetY(eNBPosList->GetVectorPositionList().at(0).y);
//	backgroundTrafficUEPosList->Add(Vector(rndPosGen->GetNext())); numFixUEs++;
//	backgroundTrafficUEPosList->Add(Vector(rndPosGen->GetNext())); numFixUEs++;
//
//	rndPosGen->SetX(eNBPosList->GetVectorPositionList().at(1).x);
//	rndPosGen->SetY(eNBPosList->GetVectorPositionList().at(1).y);
//	backgroundTrafficUEPosList->Add(Vector(rndPosGen->GetNext())); numFixUEs++;
//	backgroundTrafficUEPosList->Add(Vector(rndPosGen->GetNext())); numFixUEs++;
//
//	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//	NodeContainer backgroundTrafficUEs;
//	backgroundTrafficUEs.Create(numFixUEs);
//	mobility.SetPositionAllocator (backgroundTrafficUEPosList);
//	mobility.Install(backgroundTrafficUEs);
//	for (NodeContainer::Iterator it = backgroundTrafficUEs.Begin ();  it != backgroundTrafficUEs.End (); ++it)
//	{
//		(*it)->Initialize ();
//	}
//	NetDeviceContainer backgroundTrafficDevs = lteHelper->InstallUeDevice (backgroundTrafficUEs);

	double gamma1 = 3.14 / 8;
	double gamma2 = 3.14 / 10;
	double gamma3 = 3.14 / 12;
	double gamma4 = 3.14 / 14;
	double gamma5 = 3.14 / 16;

	uint16_t numMobilityUE = 20;// 1
	NodeContainer bfUEs;//nearby island 1
	bfUEs.Create (numMobilityUE);

	double cell1X = eNBPosList->GetVectorPositionList().at(0).x;
	double cell1Y = eNBPosList->GetVectorPositionList().at(0).y;

	double x11 = cell1X - 4; double y11 = cell1Y + 20 * std::tan(gamma1); double x12 = cell1X + 30 + 4; double y12 = cell1Y - 18 * std::tan(gamma1);
	double x21 = cell1X - 4; double y21 = cell1Y + 20 * std::tan(gamma2); double x22 = cell1X + 30 + 4; double y22 = cell1Y - 18 * std::tan(gamma2);
	double x31 = cell1X - 4; double y31 = cell1Y + 20 * std::tan(gamma3); double x32 = cell1X + 30 + 4; double y32 = cell1Y - 18 * std::tan(gamma3);
	double x41 = cell1X - 4; double y41 = cell1Y + 20 * std::tan(gamma4); double x42 = cell1X + 30 + 4; double y42 = cell1Y - 18 * std::tan(gamma4);
	double x51 = cell1X - 4; double y51 = cell1Y + 20 * std::tan(gamma5); double x52 = cell1X + 30 + 4; double y52 = cell1Y - 18 * std::tan(gamma5);

	uint16_t k = 0;
	for (NodeContainer::Iterator it = bfUEs.Begin ();  it != bfUEs.End (); ++it)
	{
		if(k<=3)
		{
			mobility.SetMobilityModelExt1 ("ns3::BackForthBetweenTwoPointsMobilityModel",
					"X1", DoubleValue (x11),
					"Y1", DoubleValue (y11),
					"X2", DoubleValue (x12),
					"Y2", DoubleValue (y12),
					"PauseTime", DoubleValue (ueCxtStore),
					"MaxSpeed", DoubleValue (ueMaxSpeed),
					"MinSpeed", DoubleValue (ueMinSpeed),
					"InitialTime", DoubleValue (200)); //3kmph
			mobility.Install((*it));
			(*it)->Initialize ();
		}
		else if(k<=7)
		{
			mobility.SetMobilityModelExt1 ("ns3::BackForthBetweenTwoPointsMobilityModel",
					"X1", DoubleValue (x21),
					"Y1", DoubleValue (y21),
					"X2", DoubleValue (x22),
					"Y2", DoubleValue (y22),
					"PauseTime", DoubleValue (ueCxtStore),
					"MaxSpeed", DoubleValue (ueMaxSpeed),
					"MinSpeed", DoubleValue (ueMinSpeed),
					"InitialTime", DoubleValue (200)); //3kmph
			mobility.Install((*it));
			(*it)->Initialize ();
		}
		else if(k<=11)
		{
			mobility.SetMobilityModelExt1 ("ns3::BackForthBetweenTwoPointsMobilityModel",
					"X1", DoubleValue (x31),
					"Y1", DoubleValue (y31),
					"X2", DoubleValue (x32),
					"Y2", DoubleValue (y32),
					"PauseTime", DoubleValue (ueCxtStore),
					"MaxSpeed", DoubleValue (ueMaxSpeed),
					"MinSpeed", DoubleValue (ueMinSpeed),
					"InitialTime", DoubleValue (200)); //3kmph
			mobility.Install((*it));
			(*it)->Initialize ();
		}
		else if(k<=15)
		{
			mobility.SetMobilityModelExt1 ("ns3::BackForthBetweenTwoPointsMobilityModel",
					"X1", DoubleValue (x41),
					"Y1", DoubleValue (y41),
					"X2", DoubleValue (x42),
					"Y2", DoubleValue (y42),
					"PauseTime", DoubleValue (ueCxtStore),
					"MaxSpeed", DoubleValue (ueMaxSpeed),
					"MinSpeed", DoubleValue (ueMinSpeed),
					"InitialTime", DoubleValue (200)); //3kmph
			mobility.Install((*it));
			(*it)->Initialize ();
		}
		else
		{
			mobility.SetMobilityModelExt1 ("ns3::BackForthBetweenTwoPointsMobilityModel",
					"X1", DoubleValue (x51),
					"Y1", DoubleValue (y51),
					"X2", DoubleValue (x52),
					"Y2", DoubleValue (y52),
					"PauseTime", DoubleValue (ueCxtStore),
					"MaxSpeed", DoubleValue (ueMaxSpeed),
					"MinSpeed", DoubleValue (ueMinSpeed),
					"InitialTime", DoubleValue (200)); //3kmph
			mobility.Install((*it));
			(*it)->Initialize ();
		}
		k++;
	}

	NetDeviceContainer bfUEsDevs = lteHelper->InstallUeDevice (bfUEs);

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

	ues.Add (bfUEs);
//	ues.Add (backgroundTrafficUEs);

	ueDevs.Add(bfUEsDevs);
//	ueDevs.Add (backgroundTrafficDevs);

	for(uint8_t ueID = 0; ueID < ueDevs.GetN(); ++ueID){
		Ptr<LteUeNetDevice> lteUeDev = ueDevs.Get(ueID)->GetObject<LteUeNetDevice> ();
		Ptr<LteUePhy> uePhy = lteUeDev->GetPhy ();
		NS_ASSERT(qInSync > qOutOfSync);
		uePhy->SetAttribute ("Qinsync", DoubleValue (qInSync));
		uePhy->SetAttribute ("Qoutofsync", DoubleValue (qOutOfSync));
		uePhy->SetAttribute("EnableUplinkPowerControl", BooleanValue(false));
		uePhy->SetAttribute("TxPower", DoubleValue(23)); //23dbm
		Ptr<LteUeRrc> ueRrc = lteUeDev->GetRrc();
		NS_ASSERT(ueRrc != NULL);
//		ueRrc->SetAttribute("EnableTracePosition",BooleanValue(true));
		ueRrc->SetAttribute ("Qinsync", DoubleValue (qInSync));
		ueRrc->SetAttribute ("Qoutofsync", DoubleValue (qOutOfSync));
		ueRrc->SetAttribute ("MeasurementMetric", UintegerValue (2)); //SINR
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

//	params.a3Offset = cell3A3Off; params.hyst = cell3Hys; params.ttt = cell3TTT; params.cioList = cioVals3;
//	initialParamsMap[3] = params;
//
//	params.a3Offset = cell4A3Off; params.hyst = cell4Hys; params.ttt = cell4TTT; params.cioList = cioVals4;
//	initialParamsMap[4] = params;
//
//	params.a3Offset = cell5A3Off; params.hyst = cell5Hys; params.ttt = cell5TTT; params.cioList= cioVals5;
//	initialParamsMap[5] = params;
//
//	params.a3Offset = cell6A3Off; params.hyst = cell6Hys; params.ttt = cell6TTT; params.cioList = cioVals6;
//	initialParamsMap[6] = params;
//
//	params.a3Offset = cell7A3Off; params.hyst = cell7Hys; params.ttt = cell7TTT; params.cioList = cioVals7;
//	initialParamsMap[7] = params;
//
//	params.a3Offset = cell8A3Off; params.hyst = cell8Hys; params.ttt = cell8TTT; params.cioList = cioVals8;
//	initialParamsMap[8] = params;
//
//	params.a3Offset = cell9A3Off; params.hyst = cell9Hys; params.ttt = cell9TTT; params.cioList = cioVals9;
//	initialParamsMap[9] = params;
//
//	params.a3Offset = cell10A3Off; params.hyst = cell10Hys; params.ttt = cell10TTT; params.cioList = cioVals10;
//	initialParamsMap[10] = params;
	//		initialParamsMap[cellId];

	//Initialize Pthread
	Ptr<CSONMroAlgorithm> mroAlgorithm = CreateObject<CSONMroAlgorithm>(sceNBDevs, initialParamsMap, name, savingFolderName);
	Ptr<MroStatisticalDataProcessor> mroStatDataProcessor = CreateObject<MroStatisticalDataProcessor>(mroAlgorithm);
	Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMroStatData",
			MakeBoundCallback (&MroStatisticalDataProcessor::RecvMroStaticalDataCallback,mroStatDataProcessor));
	Config::Connect ("/NodeList/*/DeviceList/0/$ns3::LteEnbNetDevice/LteHandoverAlgorithm/$ns3::A3RsrpHandoverAlgorithm/RecvMeasReportA3",
			MakeBoundCallback (&MroStatisticalDataProcessor::RecvA3MeasReport,mroStatDataProcessor)); //Receive A3 Event Trace
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


int main (int argc, char *argv[])
{

	//Get parameters from command line
	double speed = 20;
	double qInSync = -2;
	double qOutOfSync = -4;
	CommandLine cmd;
	cmd.AddValue("QInSync","QInSync (dB)",qInSync);
	cmd.AddValue("QOutOfSync","QOutOfSync (dB)",qOutOfSync);
	cmd.AddValue("Speed","km/h",speed);
	cmd.Parse (argc, argv);// parse again so you can override input file default values via command line

	double shadowingVal = 0;
	double shadowingStep = 6;
	while(shadowingVal <= 12)
	{
		std::ostringstream converter5;
		converter5 << shadowingVal;
		std::string folderName = "Shadowing"+converter5.str();
		mkdir(folderName.c_str(),0700); //("Shadowing"+converter5.str()).c_str()

		std::ifstream ifTraceFile;
		ifTraceFile.open ("parameters", std::ifstream::in);

		Ptr<RlfModelValidationCase> rlfModelValidationCase = CreateObject<RlfModelValidationCase>();

		while(!ifTraceFile.eof())
		{
			double cio12 = 0;		ifTraceFile >> cio12;
			double a3Offset1 = 0;	ifTraceFile >> a3Offset1;
			uint64_t ttt1 = 0;		ifTraceFile >> ttt1;

			double cio21 = 0;		ifTraceFile >> cio21;
			double a3Offset2 = 0;	ifTraceFile >> a3Offset2;
			uint64_t ttt2 = 0;		ifTraceFile >> ttt2;

			rlfModelValidationCase->ValidationCase(ttt1,cio12,a3Offset1,ttt2,cio21,a3Offset2,speed/3.6,
					qInSync,qOutOfSync,shadowingVal);

			std::ostringstream converter1;
			std::ostringstream converter2;
			std::ostringstream converter3;
			std::ostringstream converter4;
			std::ostringstream converter5;

			converter1 << (cio12-a3Offset1);
			converter2 << ttt1;

			converter3 << (cio21-a3Offset2);
			converter4 << ttt2;

			converter5 << speed;

			std::string fileName = converter1.str()+"dB"+converter2.str()+"ms_"+converter3.str()+"dB"+converter4.str()+"ms_"+converter5.str()+"kmph";

			rlfModelValidationCase->RunValidation(fileName, folderName);
		}
		shadowingVal += shadowingStep;
	}

	NS_LOG_UNCOND("End Simulation");

}
