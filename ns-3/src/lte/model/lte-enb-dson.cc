/*
 * lte-enb-dson.cc
 *
 *  Created on: Nov 3, 2015
 *      Author: thang
 */

#include <ns3/lte-enb-dson.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <stdint.h>
#include <ns3/boolean.h>
#include <ns3/random-variable-stream.h>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbDSON");
/*
 * Dev by THANG
 * Receive new parameters from CSON
 */
void
LteEnbDSON::DSONRecvUpdateParamConfig(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	//Re-Schedule
//	Simulator::Schedule(m_CSONPeriod,&LteEnbDSON::StartCSONMroAlgorithm,this);

	switch (msgInfo) {
		case SONCommon::MRO:
		{
			//Update new Data
			if(paramList.mroParamList.reqUpdate == true){
				m_curA3Offset = paramList.mroParamList.a3Offset;
				m_curHyst = paramList.mroParamList.hystersis;
				m_curTTT = paramList.mroParamList.ttt;
				m_curCellsToAddModMap = paramList.mroParamList.cioList;

				//Request RRC to update new parameters
				LteEnbDSONSapProvider::MroParameters newMroParams;
				newMroParams.a3Offset = m_curA3Offset;
				newMroParams.hyst = m_curHyst;
				newMroParams.ttt = m_curTTT;
				newMroParams.cioList = m_curCellsToAddModMap;
				m_dSONSapProvider->DSONSetParamEnbRrc(newMroParams);
			}
		}
			break;

		default:
			NS_FATAL_ERROR("LteEnbDSON::DSONRecvUpdateParamConfig Info is not implemented yet");
			break;
	}

}

void
LteEnbDSON::StartCSONMroAlgorithm(){
	//Let RRC Send Handover Counters to CSON
	if(m_dSONSapProvider == NULL){
		NS_FATAL_ERROR("m_dSONSapProvider is not initialized");
	}
	m_dSONSapProvider->DSONPMReportNotifyEnbRrc(SONCommon::MRO);
	Simulator::Schedule(m_CSONPeriod,&LteEnbDSON::StartCSONMroAlgorithm,this);//Loop the timer
}
/*
 * Dev by THANG
 * Receive Parameters Request from RRC
 * Collect and Send back RRC
 */
void
LteEnbDSON::DSONRecvDSONParamReq(SONCommon::SONMsgInfo msgInfo){
	switch (msgInfo) {
		case SONCommon::MRO:
		{
			//Construct the msg
			SONCommon::MroParamList mroParamList;
			mroParamList.a3Offset = m_curA3Offset;
			mroParamList.hystersis = m_curHyst;
			mroParamList.ttt = m_curTTT;
			mroParamList.cioList = m_curCellsToAddModMap;
			SONCommon::SONParamList paramList; //make it as global variable to store more configuration for many SON algoritm (MLB, PCI, ...)
			paramList.mroParamList = mroParamList;
			//Send response to CSON
			m_dSONSapProvider->DSONSendDSONParamRespEnbRrc(SONCommon::MRO, paramList);
		}
			break;

		default:

			break;
	}
}

/*
 * For Algorithm1, Algorithm3
 */
void
LteEnbDSON::DoSetUpInitialHandoverParams(double hyst, double a3Off, uint16_t ttt, std::map<uint16_t,int8_t> cioList){
	NS_LOG_LOGIC("LteEnbDSON::SetUpInitialHandoverParams hysteresis " << hyst << " a3off " << a3Off << " ttt " << ttt);
	m_curA3Offset = a3Off;
	m_curHyst = hyst;
	m_curTTT = ttt;
	m_curCellsToAddModMap = cioList;
}

NS_OBJECT_ENSURE_REGISTERED(LteEnbDSON);

LteEnbDSON::LteEnbDSON(){//Constructor
	m_dSONSapUser = new MemberLteEnbDSONSapUser<LteEnbDSON>(this);
	m_dSONSapProvider = NULL;

	m_a3OffVals[1]=-15; m_a3OffVals[2]=-14.5; m_a3OffVals[3]=-14; m_a3OffVals[4]=-13.5; m_a3OffVals[5]=-13; m_a3OffVals[6]=-12.5;
	m_a3OffVals[7]=-12; m_a3OffVals[8]=-11.5; m_a3OffVals[9]=-11; m_a3OffVals[10]=-10.5; m_a3OffVals[11]=-10; m_a3OffVals[12]=-9.5;
	m_a3OffVals[13]=-9; m_a3OffVals[14]=-8.5; m_a3OffVals[15]=-8; m_a3OffVals[16]=-7.5; m_a3OffVals[17]=-7; m_a3OffVals[18]=-6;
	m_a3OffVals[19]=-5.5; m_a3OffVals[20]=-5; m_a3OffVals[21]=-4.5; m_a3OffVals[22]=-4; m_a3OffVals[23]=-3.5; m_a3OffVals[24]=-3;
	m_a3OffVals[25]=-2.5; m_a3OffVals[26]=-2; m_a3OffVals[27]=-1.5; m_a3OffVals[28]=-1; m_a3OffVals[29]=-0.5; m_a3OffVals[30]=0;
	m_a3OffVals[31]=0.5;  m_a3OffVals[32]=1; m_a3OffVals[33]=1.5; m_a3OffVals[34]=2; m_a3OffVals[35]=2.5; m_a3OffVals[36]=3; m_a3OffVals[37]=3.5;
	m_a3OffVals[38]=4; m_a3OffVals[39]=4.5; m_a3OffVals[40]=5; m_a3OffVals[41]=5.5; m_a3OffVals[42]=6; m_a3OffVals[43] = 6.5;
	m_a3OffVals[44]=7; m_a3OffVals[45]=7.5; m_a3OffVals[46]=8; m_a3OffVals[47]=8.5; m_a3OffVals[48]=9; m_a3OffVals[49] = 9.5;
	m_a3OffVals[50]=10; m_a3OffVals[51]=10.5; m_a3OffVals[52]=11; m_a3OffVals[53]=11.5; m_a3OffVals[54]=12; m_a3OffVals[55] = 12.5;
	m_a3OffVals[56]=13; m_a3OffVals[57]=13.5; m_a3OffVals[58]=14; m_a3OffVals[59]=14.5; m_a3OffVals[60]=15;

	m_tttVals[1] = 0; m_tttVals[2] = 40; m_tttVals[3] = 64; m_tttVals[4] = 80; m_tttVals[5] = 100; m_tttVals[6] = 128;
	m_tttVals[7] = 160; m_tttVals[8] = 256; m_tttVals[9] = 320; m_tttVals[10] = 480; m_tttVals[11] = 512; m_tttVals[12] = 640;
	m_tttVals[13] = 1024; m_tttVals[14] = 1280; m_tttVals[15] = 2560; m_tttVals[16] = 5120;

	m_cioList[1]=-24; m_cioList[2]=-22; m_cioList[3]=-20; m_cioList[4]=-18; m_cioList[5]=-16; m_cioList[6]=-14; m_cioList[7]=-12;
	m_cioList[8]=-10; m_cioList[9]=-8; m_cioList[10]=-6; m_cioList[11]=-5; m_cioList[12]=-4; m_cioList[13]=-3; m_cioList[14]=-2;
	m_cioList[15]=-1; m_cioList[16]=0; m_cioList[17]=1; m_cioList[18]=2; m_cioList[19]=3; m_cioList[20]=4; m_cioList[21]=5;
	m_cioList[22]=6; m_cioList[23]=8; m_cioList[24]=10; m_cioList[25]=12; m_cioList[26]=14; m_cioList[27]=16; m_cioList[28]=18;
	m_cioList[29]=20; m_cioList[30]=22; m_cioList[31]=24;

	m_CSONPeriod = Seconds(60);
	Simulator::Schedule(m_CSONPeriod,&LteEnbDSON::StartCSONMroAlgorithm,this);
}

TypeId
LteEnbDSON::GetTypeId (void){
	static TypeId tid = TypeId ("ns3::LteEnbDSON")
	    .SetParent<Object> ()
	    .SetGroupName("Lte")
	    .AddConstructor<LteEnbDSON> ();
	return tid;
}

LteEnbDSON::~LteEnbDSON(){

}
void
LteEnbDSON::DoDispose(){
	m_cioList.clear();
	m_a3OffVals.clear();
	m_tttVals.clear();
	m_curCellsToAddModMap.clear();

	delete m_dSONSapProvider;
	delete m_dSONSapUser;
}

LteEnbDSONSapUser*
LteEnbDSON::GetEnbDSONSapUser(){
	return m_dSONSapUser;
}
void

LteEnbDSON::SetEnbDSONSapProvider(LteEnbDSONSapProvider* provider){
	m_dSONSapProvider = provider;
}

}  // namespace ns3
