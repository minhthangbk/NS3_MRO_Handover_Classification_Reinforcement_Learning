/*
 * epc-cson.cc
 *
 *  Created on: Nov 12, 2015
 *      Author: thang
 */
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include "epc-mme-cson.h"
#include "son-common.h"
#include <math.h>
#include <fstream> //THANG for printing out message
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcCSON");

NS_OBJECT_ENSURE_REGISTERED (EpcMmeCSON);

EpcMmeCSON::EpcMmeCSON()
{
  NS_LOG_FUNCTION (this);
  m_CSONSapProvider = NULL;
  m_CSONSapUser = new MemberEpcMmeCSONSapUser<EpcMmeCSON> (this);

  m_cioArray[1]=-24; m_cioArray[2]=-22; m_cioArray[3]=-20; m_cioArray[4]=-18; m_cioArray[5]=-16; m_cioArray[6]=-14; m_cioArray[7]=-12;
  m_cioArray[8]=-10; m_cioArray[9]=-8; m_cioArray[10]=-6; m_cioArray[11]=-5; m_cioArray[12]=-4; m_cioArray[13]=-3; m_cioArray[14]=-2;
  m_cioArray[15]=-1; m_cioArray[16]=0; m_cioArray[17]=1; m_cioArray[18]=2; m_cioArray[19]=3; m_cioArray[20]=4; m_cioArray[21]=5;
  m_cioArray[22]=6; m_cioArray[23]=8; m_cioArray[24]=10; m_cioArray[25]=12; m_cioArray[26]=14; m_cioArray[27]=16; m_cioArray[28]=18;
  m_cioArray[29]=20; m_cioArray[30]=22; m_cioArray[31]=24;

  for(uint8_t i = 1; i <= 61; i++){
  	  m_a3OffVals[i] = -15+0.5*(i-1);
  }

  m_tttVals[1] = 0; m_tttVals[2] = 40; m_tttVals[3] = 64; m_tttVals[4] = 80; m_tttVals[5] = 100; m_tttVals[6] = 128;
  m_tttVals[7] = 160; m_tttVals[8] = 256; m_tttVals[9] = 320; m_tttVals[10] = 480; m_tttVals[11] = 512; m_tttVals[12] = 640;
  m_tttVals[13] = 1024; m_tttVals[14] = 1280; m_tttVals[15] = 2560; m_tttVals[16] = 5120;

  m_enableMroAlgorithm = true;
  m_firstWrite = true;
  //Set up initial values for parameters
  Simulator::Schedule(Seconds(2),&EpcMmeCSON::SetUpParamInitialValues,this);
//  NS_LOG_UNCOND("Time\t"<< "cellID\t" << "LHOs\t" << "EHOs\t" << "WHOs\t" << "PPHOs\t" << "totalHOs");
}

void
EpcMmeCSON::SetUpParamInitialValues(){
	m_CSONSapProvider->CSONSendDSONParamReqMme(SONCommon::MRO);
}

EpcMmeCSON::~EpcMmeCSON(){

}

void
EpcMmeCSON::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_CSONSapProvider;
  delete m_CSONSapUser;
}

TypeId
EpcMmeCSON::GetTypeId (void)
{
	//No need to add Attributes
  static TypeId tid = TypeId ("ns3::EpcMmeCSON")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<EpcMmeCSON> ()
//    .AddTraceSource ("EpcMroAlgorithmOutputs",
//                         "trace MroAlgorithmOutputs",
//                         MakeTraceSourceAccessor (&EpcMmeCSON::m_MroAlgorithmOutputs),
//                         "ns3::EpcMmeCSON::ReceiveMroAlgorithmOutputs")
    ;
  return tid;
}
/*
 * SON API Receive
 */
void
EpcMmeCSON::CSONRecvPMReportNotify(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport){
//	ETRIAlgorithm(cellId, msgInfo, pmReport);
	StdDevHysteresisAlgorithm(cellId, msgInfo, pmReport);
	//Send UpdateParamConfig to Mme
}

void
EpcMmeCSON::StdDevHysteresisAlgorithm(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport){
	//Run Heuristic Algorithm
	switch (msgInfo) {
		case SONCommon::MRO:
		{
//			bool changeHys = false;
			double ppRate;
			if((pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos == 0) ||
					(pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos < 5)){
				ppRate = 0;
			}else{
				ppRate = (double)pmReport.hoReport.numPingpongHos/(pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos);
			}

			NS_LOG_UNCOND("EpcMmeCSON::CSONRecvPMReportNotify Time "<< Simulator::Now ().GetNanoSeconds () / (double) 1e9 << " cellID " << cellId << " LHOs " << pmReport.hoReport.numTooLateHos <<
						" EHOs " << pmReport.hoReport.numTooEarlyHos <<
						" WHOs " << pmReport.hoReport.numWrongCellHos	<<
						" PPHOs " <<pmReport.hoReport.numPingpongHos <<
						" PPRate " << ppRate <<
						" StDRSRP " << pmReport.hoReport.measStdDev <<
						" totalHOs " << pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos);

			std::map<uint16_t, SONCommon::SONParamList>::iterator cellParamIt = m_eNBsParamStore.find(cellId);
			if(cellParamIt == m_eNBsParamStore.end()){
				NS_FATAL_ERROR("EpcMmeCSON::CSONRecvPMReportNotify receive report from unknown eNB");
			}

			//Hysteresis Algorithm
			if( ppRate > 0.05){
//				if((cellParamIt->second).mroParamList.hystersis >= std::ceil(pmReport.hoReport.measStdDev)){
//					(cellParamIt->second).mroParamList.hystersis += 0.5;
//				}else{
//					(cellParamIt->second).mroParamList.hystersis = std::ceil(pmReport.hoReport.measStdDev);
//				}
				(cellParamIt->second).mroParamList.hystersis += std::ceil(pmReport.hoReport.measStdDev);
				(cellParamIt->second).mroParamList.reqUpdate = true;
			}else{
				(cellParamIt->second).mroParamList.reqUpdate = false;
			}

			m_CSONSapProvider->CSONSendUpdateParamConfigMme(cellId, SONCommon::MRO, m_eNBsParamStore[cellId]);
		}
			break;

		default:
			NS_FATAL_ERROR("EpcMmeCSON::CSONRecvPMReportNotify the info is not implemented yet cellID " << cellId);
			break;
	}
}

void
EpcMmeCSON::ETRIAlgorithm(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport){

	NS_LOG_DEBUG("EpcMmeCSON::ETRIAlgorithm " << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t"
			<< cellId << "\t"
			<< pmReport.hoReport.numTooLateHos << "\t"
			<< pmReport.hoReport.numTooEarlyHos <<	"\t"
			<< pmReport.hoReport.numWrongCellHos << "\t"
			<< pmReport.hoReport.numPingpongHos <<	"\t"
			<< pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos);

	SaveHOPerformance(cellId,
			pmReport.hoReport.numTooLateHos,
			pmReport.hoReport.numTooEarlyHos,
			pmReport.hoReport.numWrongCellHos,
			pmReport.hoReport.numPingpongHos,
			pmReport.hoReport.numHOs + pmReport.hoReport.numTooLateHos);

	switch (msgInfo) {

		case SONCommon::MRO:
		{
			std::map<uint16_t, SONCommon::SONParamList>::iterator cellParamIt = m_eNBsParamStore.find(cellId);
			NS_ASSERT(cellParamIt != m_eNBsParamStore.end());
			NS_ASSERT((pmReport.hoReport.numTooLateHos >= 0) && (pmReport.hoReport.numTooEarlyHos >= 0) && (pmReport.hoReport.numWrongCellHos >= 0) && (pmReport.hoReport.numPingpongHos >= 0));

			SONCommon::HandoverStatisticReport stReport = pmReport.hoReport;
			double hoKPI = (double)(stReport.numTooLateHos + stReport.numTooEarlyHos + stReport.numWrongCellHos)/(stReport.numTooLateHos+stReport.numHOs);

			double kpiThreshold = 0.05;
			double minGap = 2;

			if((hoKPI > kpiThreshold) &&  (m_enableMroAlgorithm == true)){//There is HO issues and Mro algorithm is enable
				//If all the neighboring cell has LHO/EHO or WHO, decrease/increase TTT
				bool allNcellHasLHO = true;
				bool allNcellHasEHOorWHO = true;
				SONCommon::HandoverStatisticReport stReport = pmReport.hoReport;

				bool foundGoodCell = false;
				for (std::map<uint16_t,SONCommon::CellStatistic>::iterator itCell = stReport.hoStatistic.begin();
						(itCell != stReport.hoStatistic.end()) && (foundGoodCell == false); ++ itCell){
					if((itCell->second.m_numLHO + itCell->second.m_numEHO + itCell->second.m_numWHO) == 0){ //should not count good neighbor cell
						allNcellHasEHOorWHO = false;
						allNcellHasLHO = false;
						foundGoodCell = true; //once good cell is found, quickly quit the loop
					}else{
						if(itCell->second.m_numLHO >= itCell->second.m_numEHO+itCell->second.m_numWHO){//
							allNcellHasEHOorWHO = false;
						}else{
							allNcellHasLHO = false;
						}
					}
				}
				//Find current TTT,a3 index in the map
				bool foundTTT = false;
				std::map<uint8_t,uint16_t>::iterator itTTT;
				for(itTTT = m_tttVals.begin(); (itTTT != m_tttVals.end()) && (foundTTT == false); ++itTTT){
					if(itTTT->second == cellParamIt->second.mroParamList.ttt){
						foundTTT = true;
					}
				}
				NS_ASSERT(foundTTT == true);

				bool foundA3Off = false;
				std::map<uint8_t, double>::iterator itA3Off;
				for(itA3Off = m_a3OffVals.begin(); (itA3Off != m_a3OffVals.end()) && (foundA3Off == false); ++itA3Off){
					if(itA3Off->second == cellParamIt->second.mroParamList.a3Offset){
						foundA3Off = true;
					}
				}
				NS_ASSERT(foundA3Off == true);

				if(allNcellHasLHO == true){//Decrease both TTT and A3
					cellParamIt->second.mroParamList.ttt = (--(--itTTT))->second; //decrease TTT
					cellParamIt->second.mroParamList.a3Offset = (--(--itA3Off))->second;
				}else if(allNcellHasEHOorWHO == true){//Increase both TTT and A3
					cellParamIt->second.mroParamList.ttt = itTTT->second;
					cellParamIt->second.mroParamList.a3Offset = itA3Off->second;
				}else{//some has LHO, some has EHO or WHO, some are good neighbors
					for(std::map<uint16_t,SONCommon::CellStatistic>::iterator itCell = stReport.hoStatistic.begin();
							itCell != stReport.hoStatistic.end(); ++ itCell){
						if((itCell->second.m_numLHO + itCell->second.m_numEHO + itCell->second.m_numWHO) > 0){
							std::map<uint8_t, int8_t>::iterator itCIO;
							bool foundCIO = false;
							//check: serving cell always has CIO value for all active neighbor cell
//							NS_LOG_UNCOND("EpcMmeCSON::ETRIAlgorithm ScellId " << cellId << " attempts to change CIO of NcellId " << itCell->first);
							NS_ASSERT((cellParamIt->second.mroParamList.cioList).find(itCell->first) != (cellParamIt->second.mroParamList.cioList).end());

							for(itCIO = m_cioArray.begin(); (itCIO != m_cioArray.end()) && (foundCIO == false); ++itCIO){
								if(itCIO->second == cellParamIt->second.mroParamList.cioList[itCell->first]){
									foundCIO = true;
								}
							}
							//always find out CIO values
							NS_ASSERT(foundCIO == true);

							if(itCell->second.m_numLHO >= (itCell->second.m_numEHO + itCell->second.m_numWHO)){//Ncell has LHO -> increase CIO
								//Assure min GAP is always bigger than 2 to avoid Ping-pong
								if((cellParamIt->second.mroParamList.hystersis + cellParamIt->second.mroParamList.a3Offset - itCIO->second) >= minGap){ //Gap_min = 1
									cellParamIt->second.mroParamList.cioList[itCell->first] = itCIO->second; //CIO is already increased in the loop
								}
							}else{//Ncell has EHO -> decrease CIO
								//CIO is already increased in the loop, we have to decrease it 2 times
								cellParamIt->second.mroParamList.cioList[itCell->first] = (--(--itCIO))->second;
							}
						}else{
							continue;//Should not change parameters of good cells
						}
					}
					//if all CIO values > 0, decrease it and increase a3
					//if all CIO values < 0, increase it and decrease a3
				}
				cellParamIt->second.mroParamList.reqUpdate = true;
//				m_MroAlgorithmOutputs(cellId,m_eNBsParamStore[cellId]);//Send to main
				m_CSONSapProvider->CSONSendUpdateParamConfigMme(cellId, SONCommon::MRO, m_eNBsParamStore[cellId]);//Send to eNB: will be removed later
			}else{//There is no HO issues, no need to change parameters
				cellParamIt->second.mroParamList.reqUpdate = false;
//				m_MroAlgorithmOutputs(cellId,m_eNBsParamStore[cellId]);//Send to main
				m_CSONSapProvider->CSONSendUpdateParamConfigMme(cellId, SONCommon::MRO, m_eNBsParamStore[cellId]); //Send to eNB: will be removed later
			}

		}
		break;

		default:
			break;
	}
}

void
EpcMmeCSON::SaveHOPerformance(uint16_t cellId,uint16_t numLHO, uint16_t numEHO, uint16_t numWHO, uint16_t numPPHO, uint16_t totalHO){
	std::ofstream outFile;
	if ( m_firstWrite == true )
	{
	  outFile.open ("THANG_KPI.txt");
	  if (!outFile.is_open ())
		{
		  NS_LOG_ERROR ("Can't open file " << "THANG_KPI.txt");
		  return;
		}
	  m_firstWrite = false;
	  outFile << "time\tcellId\tnumLHO\tnumEHO\tnumWHO\tnumPPHO\ttotalLHO";
	  outFile << std::endl;
	}
	else
	{
	  outFile.open ("THANG_KPI.txt",  std::ios_base::app);
	  if (!outFile.is_open ())
		{
		  NS_LOG_ERROR ("Can't open file " << "THANG_KPI.txt");
		  return;
		}
	}

	outFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t";
	outFile << cellId << "\t";
	outFile << numLHO << "\t";
	outFile << numEHO << "\t";
	outFile << numWHO << "\t";
	outFile << numPPHO << "\t";
	outFile << totalHO << std::endl;
	outFile.close ();
}


void
EpcMmeCSON::CSONRecvDSONParamResp(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	//Update the parameters
//	NS_LOG_UNCOND("EpcMmeCSON::CSONRecvDSONParamResp MRO cellId" << cellId << " hysteresis " << paramList.mroParamList.hystersis <<
//			" a3Offset " << paramList.mroParamList.a3Offset << " TTT " << paramList.mroParamList.ttt);
	switch (msgInfo) {
		//Receive this message after 2 seconds
		//We have to manage m_eNBsParamStore carefully
		case SONCommon::MRO:
		{
//			NS_LOG_UNCOND("EpcMmeCSON::CSONRecvDSONParamResp cellId " << cellId << " hysteresis " << paramList.mroParamList.hystersis <<
//						" a3Offset " << paramList.mroParamList.a3Offset << " TTT " << paramList.mroParamList.ttt);
			std::map<uint16_t, SONCommon::SONParamList>::iterator it = m_eNBsParamStore.find(cellId);
			if(it == m_eNBsParamStore.end()){//the cell is not added into database yet
				m_eNBsParamStore[cellId] = paramList;
			}else{//the cell is added into database because ANR message is reveived before, so neighbor list have to be carefully programmed
//				m_eNBsParamStore[cellId].mroParamList = paramList.mroParamList;
				m_eNBsParamStore[cellId].mroParamList.a3Offset = paramList.mroParamList.a3Offset;
				m_eNBsParamStore[cellId].mroParamList.hystersis = paramList.mroParamList.hystersis;
				m_eNBsParamStore[cellId].mroParamList.ttt = paramList.mroParamList.ttt;
				m_eNBsParamStore[cellId].mroParamList.reqUpdate = false;
				for (std::map<uint16_t,int8_t>::iterator itCIO = paramList.mroParamList.cioList.begin();
						itCIO != paramList.mroParamList.cioList.end(); ++itCIO){
					m_eNBsParamStore[cellId].mroParamList.cioList[itCIO->first] = itCIO->second;
				}
			}
		}
			break;

		//This message may be received before the above, so we have carefully manage it
		case SONCommon::ANR:
		{
			//Update new active cell for the CIO List of MRO of reported cell
//			NS_LOG_UNCOND("EpcMmeCSON::CSONRecvDSONParamResp ANR cellId " << cellId << " has new active cellId " << paramList.anrParamList.newActiveCell);

			std::map<uint16_t, SONCommon::SONParamList>::iterator it = m_eNBsParamStore.find(cellId);
			if(it == m_eNBsParamStore.end()){//the cell is not added into database yet
				m_eNBsParamStore[cellId] = paramList; //Add it to database
				m_eNBsParamStore[cellId].mroParamList.cioList[paramList.anrParamList.newActiveCell] = 0;//initial value is 0
			}else{
				m_eNBsParamStore[cellId].mroParamList.cioList[paramList.anrParamList.newActiveCell] = 0;//initial value is 0
			}
//			m_eNBsParamStore[cellId].mroParamList.cioList[paramList.anrParamList.newActiveCell] = 0;//only this line is enough?
		}
			break;

		default:
			//ANR, MLB, FHM, RO, CHS, PCI...
			break;
	}

}


EpcMmeCSONSapUser*
EpcMmeCSON::GetEpcMmeCSONSapUser(){
	return m_CSONSapUser;
}

void
EpcMmeCSON::SetEpcMmeCSONSapProvider(EpcMmeCSONSapProvider* sapProvider){
	m_CSONSapProvider = sapProvider;
	//Test
}

}  // namespace ns3
