/*
 * lte-son-common.h
 *
 *  Created on: Nov 15, 2015
 *      Author: thang
 */

#ifndef LTE_SON_COMMON_H_
#define LTE_SON_COMMON_H_
#include <map>
#include <list>
#include "ns3/lte-rrc-sap.h"
namespace ns3 {

/*
 * developed by THANG
 * Define CSON,DSON API
 */
class	SONCommon{
public:
	/*
	 * Merge with Handover Issue later!!!
	 */
	struct CellStatistic{
		uint16_t m_numLHO;//Too Late HO
		uint16_t m_numEHO;//Too Early HO
		uint16_t m_numWHO;//Wrongcell HO
		uint16_t m_numPP; //Ping pong HO
		uint16_t m_numNHO; //Normall HO
		//uint16_t m_numRCD; //RLF Connection Drop;
	};

	struct HandoverStatisticReport{
		uint16_t numTooLateHos;
		uint16_t numTooEarlyHos;
		uint16_t numWrongCellHos;
		uint16_t numPingpongHos;
		uint16_t numRLFConnDrops;
		uint16_t numHOs;
		double measStdDev; //Standard Deviation of Measurement in Measurement Report
		std::map<uint16_t,CellStatistic> hoStatistic; //Database for each neighbor cell
	};

	enum MROProblem{//Creating for pThread app
		TOO_LATE_HANDOVER = 0,
		TOO_EARLY_HANDOVER,
		WRONG_CELL_HANDOVER,
		PING_PONG_HANDOVER,
		NORMAL_HANDOVER,
		RLF_CONNECTION_DROP
	};

	struct ResourceAllocationInfo{
		uint32_t frame;
		uint32_t subframe;
		uint16_t rnti;
		//uint8_t mcs0;
		uint32_t tbs0Size;
		uint32_t rbBitmap;
		//uint8_t mcs1;
		uint32_t tbs1Size;
		uint16_t rbRIV;
	};

	struct MROProblemInfo{
		MROProblem problem;
		uint16_t cellId;
		uint64_t imsi;
		std::list<LteRrcSap::MeasResultEutra> measData;
	};

	struct PerformanceReport{
		HandoverStatisticReport hoReport;
		//Other Structs can be described below (PCI, MLB, RO...)
	};
	/*
	 * Change to CIO Value later
	 */
	enum CIOValue{
		CIO_24 = 0,	CIO_22,	CIO_20,	CIO_18,	CIO_16,	CIO_14,	CIO_12,	CIO_10,	CIO_8, CIO_6,CIO_5,	CIO_4, CIO_3, CIO_2, CIO_1, CIO0,
		CIO1, CIO2, CIO3, CIO4, CIO5, CIO6, CIO8, CIO10, CIO12, CIO14, CIO16, CIO18, CIO20, CIO22, CIO24
	};

	enum SONMsgInfo{//~Flag field in header
		MRO = 0,
		//MLB
		ANR
	};
	struct MroParamList{ //Payload structure
		std::map<uint16_t, int8_t>	cioList;
		double a3Offset;
		double hystersis;
		uint16_t ttt;
		bool reqUpdate;
	};
	struct AnrParamList{
//		std::list<uint16_t> activeCells;
		uint16_t newActiveCell;
	};
	//Aggregate many types of structure to avoid building many API
	struct SONParamList{
		MroParamList mroParamList;
		AnrParamList anrParamList;
		//MLB list, ANR list, ...
	};

};//End SONCommon


}  // namespace ns3

#endif /* LTE_SON_COMMON_H_ */
