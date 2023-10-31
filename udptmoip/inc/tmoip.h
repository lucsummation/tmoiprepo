#include <stdio.h>
//#include <cstdint>
#include <stdint.h>


/*
**  Define
*/
#define UDP_packet_max_len		1522				// network packet max length.	
#define PORT 60001							// The port on which to listen for incoming data

#define TMoIP_RawData_max_len		1500				// TMoIP Raw Data Max Length.
#define TMoIP_RawData_min_len		1				// TMoIP Raw Data Min Length.
#define Sequence_len			2				// TMoIP control word sequence number length
#define Timestamps_len			8				// TMoIP control word timestamps length
#define DQE_PCM_len				6			// Data Quality Metrics or PCM frame length.

#define TMoIP_Ver_Mask			0xf0				// Version Identifier
#define TMoIP_PLD_Mask			0x0c				// Payload type 
#define TMoIP_mFSS_Mask			0x03				// Minor Frame Sync Status 
#define TMoIP_MMFSS_Mask		0xc0				// Major Frame Sync Status 
#define TMoIP_TSR_Mask			0x01				// Timestamps Source Reference

/*
**  enum, and struct
*/
enum PLD_Type
{
	NO_frame_alignment,						// indicates no frame alignment.
	PCM_frame_alignment,						// indicates PCM frame aligned, first or only packet.
	DQE_frame_alignment,						// indicates DQE frame aligned, first or only packet. 
	CON_frame_alignment						// indicates frame aligned, continuation packet.
};

enum mFSS // Minor/Major Frame Sync Status (not applicable for PLD = "00"). 
{
	mMSearch,							// indicates Search.
	mMCheck,							// indicates Check. 
	mMLock,								// indicates Lock.
	mMFlywheeel							// indicates Flywheel.

};

enum TSR // Timestamp Source Reference 
{
	UCT,								// indicates Universal Coordinated Time.
	IAT								// indicates International Atomic Time.
};


typedef union // TMoIP control data's first byte
{
	struct
	{
		uint8_t ver : 4;					// Version identifier.
		uint8_t PLD : 2;					// Payload type.
		uint8_t mFSS : 2;					// Minor Frame Sync Status (not applicable for PLD = "00"). 
	}bits;
	uint8_t tmoip_data_1;
}tmoip_data_1;


typedef union // TMoIP control data's second byte
{
	struct
	{
		uint8_t MFSS : 2;		// Major Frame Sync Status (not applicable for PLD = "00").
		uint8_t RES : 5;		// Reserved.
		uint8_t TRS : 1;		// Timestamp Source Reference.
	}bits;
	uint8_t tmoip_data_2;
}tmoip_data_2;


struct MPoIP_Ctrl_Words	// TMoIP 218-20 Control Word fields
{

	tmoip_data_1	data_1;
	tmoip_data_2	data_2;
	uint8_t			Sequence[Sequence_len];				// Sequence Number. 
	uint8_t			Timestamps[Timestamps_len]; 			// 64-bit, Timestamp PTP format.

};


/*
**  function
*/
void paser_recvd_tmoip_data(char *ptr, int len);
void error_msg_print( char*s);
