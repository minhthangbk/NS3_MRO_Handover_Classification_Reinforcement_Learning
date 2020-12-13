/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include "epc-s1ap-sap.h"
#include "epc-s11-sap.h"

#include "epc-mme.h"
#include "son-common.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcMme");

NS_OBJECT_ENSURE_REGISTERED (EpcMme);

EpcMme::EpcMme ()
: m_s11SapSgw (0)
{
	NS_LOG_FUNCTION (this);
	m_s1apSapMme = new MemberEpcS1apSapMme<EpcMme> (this);
	m_s11SapMme = new MemberEpcS11SapMme<EpcMme> (this);
	m_CSONSapUser = NULL;
	m_CSONSapProvider = new MemberEpcMmeCSONSapProvider<EpcMme>(this);
}


EpcMme::~EpcMme ()
{
	NS_LOG_FUNCTION (this);
}

void
EpcMme::DoDispose ()
{
	NS_LOG_FUNCTION (this);
	delete m_s1apSapMme;
	delete m_s11SapMme;
	m_CSONSapProvider = NULL;
	m_CSONSapUser = NULL;
}

TypeId
EpcMme::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::EpcMme")
    		.SetParent<Object> ()
    		.SetGroupName("Lte")
    		.AddConstructor<EpcMme> ()
    		;
	return tid;
}

EpcS1apSapMme* 
EpcMme::GetS1apSapMme ()
{
	return m_s1apSapMme;
}

void 
EpcMme::SetS11SapSgw (EpcS11SapSgw * s)
{
	m_s11SapSgw = s;
}

EpcS11SapMme* 
EpcMme::GetS11SapMme ()
{
	return m_s11SapMme;
}

void 
EpcMme::AddEnb (uint16_t gci, Ipv4Address enbS1uAddr, EpcS1apSapEnb* enbS1apSap)
{
	NS_LOG_FUNCTION (this << gci << enbS1uAddr);
	Ptr<EnbInfo> enbInfo = Create<EnbInfo> ();
	enbInfo->gci = gci;
	enbInfo->s1uAddr = enbS1uAddr;
	enbInfo->s1apSapEnb = enbS1apSap;
	m_enbInfoMap[gci] = enbInfo;
}

void 
EpcMme::AddUe (uint64_t imsi)
{
	NS_LOG_FUNCTION (this << imsi);
	Ptr<UeInfo> ueInfo = Create<UeInfo> ();
	ueInfo->imsi = imsi;
	ueInfo->mmeUeS1Id = imsi;
	m_ueInfoMap[imsi] = ueInfo;
	ueInfo->bearerCounter = 0;
}

uint8_t
EpcMme::AddBearer (uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer)
{
	NS_LOG_FUNCTION (this << imsi);
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
	NS_ASSERT_MSG (it->second->bearerCounter < 11, "too many bearers already! " << it->second->bearerCounter);
	BearerInfo bearerInfo;
	bearerInfo.bearerId = ++(it->second->bearerCounter);
	bearerInfo.tft = tft;
	bearerInfo.bearer = bearer;
	it->second->bearersToBeActivated.push_back (bearerInfo);
	return bearerInfo.bearerId;
}


// S1-AP SAP MME forwarded methods

void 
EpcMme::DoInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t imsi, uint16_t gci)
{
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
	if((it->second->cellId != 0))
	{
		it->second->rlfCounter++;
		it->second->rlfDetect = true;
	}
	it->second->cellId = gci;
	EpcS11SapSgw::CreateSessionRequestMessage msg;
	msg.imsi = imsi;
	msg. uli.gci = gci;
	NS_LOG_DEBUG("EpcMme::DoInitialUeMessage (it->second->bearersToBeActivated) contains " << it->second->bearersToBeActivated.size() << " items" );
	for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
			bit != it->second->bearersToBeActivated.end ();
			++bit)
	{
		EpcS11SapSgw::BearerContextToBeCreated bearerContext;
		bearerContext.epsBearerId =  bit->bearerId;
		bearerContext.bearerLevelQos = bit->bearer;
		bearerContext.tft = bit->tft;
		msg.bearerContextsToBeCreated.push_back (bearerContext);
	}
	NS_LOG_DEBUG("EpcMme::DoInitialUeMessage msg contains " << msg.bearerContextsToBeCreated.size() << " items" );
	m_s11SapSgw->CreateSessionRequest (msg);
}

void 
EpcMme::DoInitialContextSetupResponse (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapMme::ErabSetupItem> erabSetupList)
{
	NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id);
	NS_FATAL_ERROR ("unimplemented");
}

void 
EpcMme::DoPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
	NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id << gci);

	uint64_t imsi = mmeUeS1Id;
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
	NS_LOG_INFO ("IMSI " << imsi << " old eNB: " << it->second->cellId << ", new eNB: " << gci);
	it->second->cellId = gci;
	it->second->enbUeS1Id = enbUeS1Id;

	EpcS11SapSgw::ModifyBearerRequestMessage msg;
	msg.teid = imsi; // trick to avoid the need for allocating TEIDs on the S11 interface
	msg.uli.gci = gci;
	// bearer modification is not supported for now
	m_s11SapSgw->ModifyBearerRequest (msg);
}


// S11 SAP MME forwarded methods

void 
EpcMme::DoCreateSessionResponse (EpcS11SapMme::CreateSessionResponseMessage msg)
{
	//NS_LOG_FUNCTION (this << msg.teid);
	NS_LOG_DEBUG("EpcMme::DoCreateSessionResponse message contains " << (uint32_t)(msg.bearerContextsCreated.size()) << " items");
	uint64_t imsi = msg.teid;//Tricky for avoiding TeId allocation
	std::list<EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList;
	for (std::list<EpcS11SapMme::BearerContextCreated>::iterator bit = msg.bearerContextsCreated.begin ();
			bit != msg.bearerContextsCreated.end (); ++bit)
	{
		NS_LOG_FUNCTION ("EpcMme::DoCreateSessionResponse: bit->bearerLevelQos: " << (uint32_t)bit->bearerLevelQos.qci);
		EpcS1apSapEnb::ErabToBeSetupItem erab;
		erab.erabId = bit->epsBearerId;
		erab.erabLevelQosParameters = bit->bearerLevelQos;
		erab.transportLayerAddress = bit->sgwFteid.address;
		erab.sgwTeid = bit->sgwFteid.teid;
		erabToBeSetupList.push_back (erab);
	}
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
	uint16_t cellId = it->second->cellId;
	uint16_t enbUeS1Id = it->second->enbUeS1Id;
	uint64_t mmeUeS1Id = it->second->mmeUeS1Id; //IMSI
	std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (cellId);
	NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);
	jt->second->s1apSapEnb->InitialContextSetupRequest (mmeUeS1Id, enbUeS1Id, erabToBeSetupList);
}


void 
EpcMme::DoModifyBearerResponse (EpcS11SapMme::ModifyBearerResponseMessage msg)
{
	NS_LOG_FUNCTION (this << msg.teid);
	NS_ASSERT (msg.cause == EpcS11SapMme::ModifyBearerResponseMessage::REQUEST_ACCEPTED);
	uint64_t imsi = msg.teid;
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
//	if(it->second->rlfDetect)
//	{
//		it->second->rlfDetect = false;
//	}
//	else
//	{
		uint64_t enbUeS1Id = it->second->enbUeS1Id; //==new rnti
		uint64_t mmeUeS1Id = it->second->mmeUeS1Id; // == imsi
		uint16_t cgi = it->second->cellId;
		std::list<EpcS1apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList; // unused for now
		std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (it->second->cellId); //find the cell with cellId
		NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << it->second->cellId);
		jt->second->s1apSapEnb->PathSwitchRequestAcknowledge (enbUeS1Id, mmeUeS1Id, cgi, erabToBeSwitchedInUplinkList); //then send the message to found cellId
//	}

}

void
EpcMme::DoErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
	NS_LOG_DEBUG("EpcMme::DoErabReleaseIndication IMSI " << mmeUeS1Id << " RNTI " << enbUeS1Id);
	uint64_t imsi = mmeUeS1Id;
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);

	EpcS11SapSgw::DeleteBearerCommandMessage msg;
	// trick to avoid the need for allocating TEIDs on the S11 interface
	msg.teid = imsi;

	for (std::list<EpcS1apSapMme::ErabToBeReleasedIndication>::iterator bit = erabToBeReleaseIndication.begin (); bit != erabToBeReleaseIndication.end (); ++bit)
	{
		EpcS11SapSgw::BearerContextToBeRemoved bearerContext;
		bearerContext.epsBearerId =  bit->erabId;
		msg.bearerContextsToBeRemoved.push_back (bearerContext);
	}
	//Delete Bearer command towards epc-sgw-pgw-application
	m_s11SapSgw->DeleteBearerCommand (msg);
}

void
EpcMme::DoDeleteBearerRequest (EpcS11SapMme::DeleteBearerRequestMessage msg)
{
	NS_LOG_DEBUG("EpcMme::DoDeleteBearerRequest IMSI " << msg.teid);
	uint64_t imsi = msg.teid;
	std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
	NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
	EpcS11SapSgw::DeleteBearerResponseMessage res;

	res.teid = imsi;

	for (std::list<EpcS11SapMme::BearerContextRemoved>::iterator bit = msg.bearerContextsRemoved.begin ();
			bit != msg.bearerContextsRemoved.end ();
			++bit)
	{
		EpcS11SapSgw::BearerContextRemovedSgwPgw bearerContext;
		bearerContext.epsBearerId = bit->epsBearerId;
		res.bearerContextsRemoved.push_back (bearerContext);

		RemoveBearer (it->second, bearerContext.epsBearerId); //schedules function to erase, context of de-activated bearer
	}
	//schedules Delete Bearer Response towards epc-sgw-pgw-application
	m_s11SapSgw->DeleteBearerResponse (res);
}

void EpcMme::RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId)
{
	NS_LOG_DEBUG("EpcMme::RemoveBearer RNTI " << ueInfo->enbUeS1Id << " IMSI " << ueInfo->enbUeS1Id << " epsBearerId " << (uint32_t)epsBearerId);
	//NS_LOG_FUNCTION (this << epsBearerId);
	for (std::list<BearerInfo>::iterator bit = ueInfo->bearersToBeActivated.begin ();
			bit != ueInfo->bearersToBeActivated.end ();
			++bit)
	{
		if (bit->bearerId == epsBearerId)
		{
			ueInfo->bearersToBeActivated.erase (bit);
			break;
		}
	}
}

EpcMmeCSONSapProvider*
EpcMme::GetEpcMmeCSONSapProvider(){
	return m_CSONSapProvider;
}

void
EpcMme::SetEpcMmeCSONSapUser(EpcMmeCSONSapUser* sapUser){
	m_CSONSapUser = sapUser;
	//Test
	//	m_CSONSapUser->RecvHandoverPerformanceNotification(1,1);
}


/*
 * Receive Handover Counters, then forward to CSON/OAM layer
 */
void
EpcMme::MmeRcvPMReportNotify(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport){
	//Forward to OAM layer
	//	NS_LOG_UNCOND("EpcMme::MmeRcvPMReportNotify cellId " << cellId);//for debug
	m_CSONSapUser->MmeSendPMReportNotifyCSON(cellId, msgInfo, pmReport);
}

void
EpcMme::MmeRecvDSONParamResp(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	//Forward to OAM layer
	m_CSONSapUser->MmeSendDSONParamRespCSON(cellId,msgInfo,paramList);
}

void
EpcMme::MmeRecvUpdateParamConfig(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	std::map<uint16_t, Ptr<EnbInfo> >::iterator it = m_enbInfoMap.find (cellId);
	NS_ASSERT_MSG (it != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);
	it->second->s1apSapEnb->MmeSendUpdateParamConfigEnbS1ap(msgInfo,paramList);
}

void
EpcMme::MmeRecvDSONParamReq(SONCommon::SONMsgInfo msgInfo){
	if(msgInfo == SONCommon::MRO){
		std::map<uint16_t, Ptr<EnbInfo> >::iterator it;
		//broadcast the request to all Enb
		for(it = m_enbInfoMap.begin(); it != m_enbInfoMap.end(); ++it){
			it->second->s1apSapEnb->MmeSendDSONParamReqEnbS1ap(msgInfo);
		}
	}
}

} // namespace ns3
