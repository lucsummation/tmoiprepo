/*
 * 	TMoIP.c  
 * 	Parse the Telemetry data from the ethernet
 */
#include <stdio.h>
#include <string.h>
#include "tmoip.h"


 /*
 ** Variables
 */
struct TMoIP_Data // TMoIP 218-20 data
{
	struct MPoIP_Ctrl_Words MPoIP_CtrlWord;			// TMoIP 218-20 Control Word fields
	uint8_t DQE_PCM[DQE_PCM_len];					// Data Quality Metric
	uint8_t rawdata[TMoIP_RawData_max_len]; 		// TMoIP raw data

};


/*
** Implementation
*/

/** 
 *  @brief This function will parse the TM over IP data such as Version, PLD, QMC, etc.
 *  @param UDP receiver buffer pointer and the receive data length
 */
void paser_recvd_tmoip_data(char *ptr, int rcvd_len)
{
	int16_t TMoIP_rawdata_len = 0; 		// TMoIP raw data length.
	struct TMoIP_Data TMoIPData;
	
	if (rcvd_len >= sizeof(TMoIPData.MPoIP_CtrlWord)) //  the receive data should be greater the TMoIP control word length,
	{
		// Version identifier.
		TMoIPData.MPoIP_CtrlWord.data_1.bits.ver = (*ptr & TMoIP_Ver_Mask) >> 4; 
		printf("TMoIP version is : %x \n", TMoIPData.MPoIP_CtrlWord.data_1.bits.ver);

		// Payload type
		TMoIPData.MPoIP_CtrlWord.data_1.bits.PLD = (*ptr & TMoIP_PLD_Mask) >> 2;
		printf("PLD is : %x \n", TMoIPData.MPoIP_CtrlWord.data_1.bits.PLD);

		// Minor Frame Sync Status (not applicable for PLD = “00”).
		TMoIPData.MPoIP_CtrlWord.data_1.bits.mFSS = *ptr & TMoIP_mFSS_Mask;
		printf("mFSS is : %x \n", TMoIPData.MPoIP_CtrlWord.data_1.bits.mFSS);
		ptr++;

		// Major Frame Sync Status (not applicable for PLD = “00”).
		TMoIPData.MPoIP_CtrlWord.data_2.bits.MFSS = (*ptr & TMoIP_MMFSS_Mask) >> 6;
		printf("MFSS is : %x \n", TMoIPData.MPoIP_CtrlWord.data_2.bits.MFSS);

		// Timestamp Source Reference.
		TMoIPData.MPoIP_CtrlWord.data_2.bits.TRS = *ptr & TMoIP_TSR_Mask;
		printf("TRS is : %x \n", TMoIPData.MPoIP_CtrlWord.data_2.bits.TRS);
		ptr++;	// move to next byte SEQ NUMBER

		// Sequence Number. 
		memcpy(TMoIPData.MPoIP_CtrlWord.Sequence, ptr, Sequence_len);
		printf("TMoIP's sequence number is : %x %x\n", TMoIPData.MPoIP_CtrlWord.Sequence[0], TMoIPData.MPoIP_CtrlWord.Sequence[1]);
		ptr = ptr + Sequence_len; // move to Timestamps

		// 64-bit, Timestamp – PTP format.
		memcpy(TMoIPData.MPoIP_CtrlWord.Timestamps, ptr, Timestamps_len);
		
		printf("TMoIP's Timestamps is:");
		for (int i = 0; i < sizeof(TMoIPData.MPoIP_CtrlWord.Timestamps); i++)
		{
			printf("%02x", TMoIPData.MPoIP_CtrlWord.Timestamps[i]);
		}
		printf("\n");

		ptr = ptr + Timestamps_len; // move to DQE, PCM or TMoIP raw data


		// look the raw data by PLD option
		switch (TMoIPData.MPoIP_CtrlWord.data_1.bits.PLD) 
		{

		// indicates PCM frame aligned, first or only packet.
		case PCM_frame_alignment:
                        // the receive data should be greater the TMoIP control word length plus PCM length
			if (rcvd_len > sizeof(TMoIPData.MPoIP_CtrlWord) + sizeof(TMoIPData.DQE_PCM)) 
			{
				memcpy(TMoIPData.DQE_PCM, ptr, sizeof(TMoIPData.DQE_PCM));  // copy input stream to PCM, first or only packet.
				TMoIP_rawdata_len = rcvd_len - sizeof(TMoIPData.MPoIP_CtrlWord) - sizeof(TMoIPData.DQE_PCM);
				ptr = ptr + sizeof(TMoIPData.DQE_PCM);
				printf("PCM value is : %x %x\n\n", TMoIPData.DQE_PCM[4], TMoIPData.DQE_PCM[5]);
				
			}
			break;

		// indicates DQE frame aligned, first or only packet. 
		case DQE_frame_alignment:
			// the receive data should be greater the TMoIP control word length plus DQE length
			if (rcvd_len > sizeof(TMoIPData.MPoIP_CtrlWord) + sizeof(TMoIPData.DQE_PCM))  
			{
			
				memcpy(TMoIPData.DQE_PCM, ptr, sizeof(TMoIPData.DQE_PCM));   
				TMoIP_rawdata_len = rcvd_len - sizeof(TMoIPData.MPoIP_CtrlWord) - sizeof(TMoIPData.DQE_PCM);
				ptr = ptr + sizeof(TMoIPData.DQE_PCM);
				printf("Data Quality Metric value is : %x %x\n\n", TMoIPData.DQE_PCM[4], TMoIPData.DQE_PCM[5]);

			}
			break;

		// indicates frame aligned, continuation packet.
		case CON_frame_alignment:  // either PCM or DEQ
                        // the receive data should be greater the TMoIP control word length plus PCM/DQE length
			if (rcvd_len > sizeof(TMoIPData.MPoIP_CtrlWord) + sizeof(TMoIPData.DQE_PCM)) 
			{
				memcpy(TMoIPData.DQE_PCM, ptr, sizeof(TMoIPData.DQE_PCM));  // copy input stream to Con. continuation packet
				TMoIP_rawdata_len = rcvd_len - sizeof(TMoIPData.MPoIP_CtrlWord) - sizeof(TMoIPData.DQE_PCM);
				ptr = ptr + sizeof(TMoIPData.DQE_PCM);
				printf("DQM/PCM/Continue Packek value is : %x %x\n\n", TMoIPData.DQE_PCM[4], TMoIPData.DQE_PCM[5]);

			}

			break;

		// indicates no frame alignment.
		case NO_frame_alignment:
		default:	
			TMoIP_rawdata_len = rcvd_len - sizeof(TMoIPData.MPoIP_CtrlWord);
			break;
		}

		if (TMoIP_rawdata_len > 0)
		{
			// copy TMoIP raw data
			memcpy(TMoIPData.rawdata, ptr, TMoIP_rawdata_len);  // copy input stream to PCM, first or only packet. 

			printf("TMoIP Raw Data\n");
			for (int i = 0; i < TMoIP_rawdata_len; i++)
			{
				printf("%x", TMoIPData.rawdata[i]);

			}
		}
	}
}
	











