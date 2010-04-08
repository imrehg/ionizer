#ifndef _MESSAGING_H_
#define _MESSAGING_H_


#define MSG_STD_PAYLOAD_SIZE (220)

//! Class to encapsulate messages that are transmitted between PC and FPGA via Gigabit Ethernet.
/*
 * The FPGA is big-endian, so that is the convention used.
 */
class GbE_msg
{
public:
   GbE_msg() {}

   //! send message, return number of bytes sent
   int snd();

   //! receive message, return number of bytes received
   int rcv();

   //! insert an unsigned int into the payload at location loc
   void insertU(unsigned loc, unsigned u);

   //! insert a string into the payload at location loc
   void insertS(unsigned loc, const char* s);

   //! insert two strings into the payload at location loc separated by a null.
   void insert2S(unsigned loc, const char* s1, const char* s2);

   //! extract an unsigned int from the payload at location loc
   unsigned extractU(unsigned loc) const;

   //! extract a char* from the payload at location loc
   char* extractS(unsigned loc);

    //! extract a const char* from the payload at location loc
   const char* extractSC(unsigned loc) const;

   //! counts number of messages, to make sure none were lost.  Are we checking this?
   unsigned id;

   //! what the message is about. codes are defined below, e.g. C2S_CHANGE_PARAMS
   unsigned what;

protected:

    //! message contents
   unsigned m[MSG_STD_PAYLOAD_SIZE];
};


#define MIN_MSG_LENGTH (sizeof(GbE_msg))
#define MAX_STR_LENGTH (sizeof(unsigned)*MSG_STD_PAYLOAD_SIZE - 1)

/* Client (PC) to Server (FPGA) messages (C2S)

   The message format for m is specified for each message.
   After each C2S message, the server will acknowledge with S2C_ACK.
   If there is a reply message specified, this is the payload of S2C_ACK.

*/
/********* Interface definitions ****/

#define C2S_ACK					(0x00F0)
#define C2S_QUIT				(0x0100)
#define C2S_SEND_IRQ			(0x0101)

//C2S: nothing
//S2C: m[0] = number of interfaces available
#define C2S_NUM_INTERFACES 		(0x0F00)

//C2S: m[0]       = interface number
//S2C: m[0,1,...] = text stream describing interface elements
#define C2S_RP_TYPE				(0x0F02)
#define C2S_RP_NAME				(0x0F03)

//C2S: m[0]       = number of params to update
//C2S: m[1,...]   = name = value pairs
#define C2S_CHANGE_PARAMS       	(0x0A00)
#define C2S_NUM_EXP_PARAMS       	(0x0A01)
#define C2S_DEFINE_EXP_PARAM       	(0x0A02)
#define C2S_RUN_EXP			       	(0x0A03)
#define C2S_NUM_EXP			       	(0x0A04)
#define C2S_EXP_NAME			  	(0x0A05)
#define C2S_NUM_XITIONS				(0x0A06)
#define C2S_NUM_DATA_CHANNELS		(0x0A07)
#define C2S_DATA_CHANNEL_NAME		(0x0A08)
#define C2S_DETECTION_HISTOGRAM		(0x0A09)
#define C2S_RESET_STATS				(0x0A0A)
#define C2S_NUM_REMOTE_ACTIONS	(0x0A0B)
#define C2S_REMOTE_ACTION_NAME	(0x0A0C)
#define C2S_CALL_REMOTE_ACTION	(0x0A0D)
#define C2S_EXPLAIN_EXP_PARAM   (0x0A0E)
#define C2S_HIST_NAME           (0x0A0F)
#define C2S_CHANGE_PARAM_BIN  	(0x0A10)
#define C2S_GET_PARAM_VAL_STR  	(0x0A11)
#define C2S_NUM_DATA_PLOTS		(0x0A12)
#define C2S_GET_PLOT_DATA		(0x0A13)
#define C2S_SET_COEFFICIENTS	(0x0A14)

#define EXP_ID_DETECT       		(0x0B00)
#define EXP_ID_HEAT		       		(0x0B01)

/********* DDS messages *************/

//C2S: m[0] = nDDS, m[1] = ftw
//S2C: nothing
#define C2S_SET_DDS_FREQ 	(0x1000)

//C2S: m[0] = nDDS
//S2C: m[1] = ftw
#define C2S_GET_DDS_FREQ 	(0x1001)

//C2S: m[0] = nDDS, m[1] = phase
//S2C: nothing
#define C2S_SET_DDS_PHASE 	(0x1002)

//C2S: m[0] = nDDS
//S2C: m[1] = phase
#define C2S_GET_DDS_PHASE 	(0x1003) //m[0] = nDDS

//C2S: m[0] = nDDS
//S2C: nothing
#define C2S_RESET_DDS 	    (0x1004) //m[0] = nDDS

//C2S: m[0] = nTests
//S2C: nothing
#define C2S_TEST_DDS 	    (0x1005) //m[0] = nDDS


/********* TTL messages *************/
//TTL output are produced by combining the pulse output word with the AND/OR registers
//C2S: m[0] = HIGH register, m[1] = LOW register
//S2C: nothing
#define C2S_SET_TTL 		(0x1010)

//C2S: nothing
//S2C: m[0] = AND register, m[1] = OR register
#define C2S_GET_TTL 		(0x1011)

/********* MOTOR messages *************/
#define C2S_SET_MOTOR_ANGLE	(0x1014)
#define C2S_GET_MOTOR_ANGLE (0x1015)


/******** OTHER messages ************/
#define C2S_SET_DEBUG_1ST       (0x1020)
//#define C2S_SET_VOLTAGES        (0x1022) available
//#define C2S_SET_VOLTAGE         (0x1023) available
#define C2S_SET_DAC_CALIBRATION (0x1024)
#define C2S_RAMP_VOLTAGES       (0x1025)
#define C2S_GET_AL_STATE	(0x1026)
#define CS2_SET_ION_XTAL        (0x1027)

//server (FPGA) to client (PC) messages
//standard acknowledgement
#define S2C_OK	 			(0x2000)

//acknowledgement that says another acknowledgement will follow
//the message payload of the subsequent message should be appended to this one
#define S2C_ACK_CONTD 		(0x2001)
#define S2C_ERROR 		(0x2002)
#define S2C_DEBUG_MESSAGE	(0x2003)
#define S2C_UPDATE_PARAM        (0x2004) //FPGA has new param values ... please update GUI
#define S2C_INFO_MESSAGE        (0x2005)


//remote parameter types
#define RP_UNSIGNED			(0)
#define RP_INT				(1)
#define RP_DOUBLE			(2)
#define RP_BOOL		   		(3)
#define RP_DDS_PULSE		(4)
#define RP_TTL_PULSE		(5)
#define RP_MATRIX  			(6)
#define RP_LCD  			(7)
#define RP_STRING  			(8)

//remote parameter flags
#define RP_FLAG_UPDATE_IMMEDIATE   (0x00000008)
#define RP_FLAG_NOPACK             (0x00000010)
#define RP_FLAG_READ_ONLY			(0x00000020)
#define RP_FLAG_CAN_HIDE			(0x00000040)
#define RP_FLAG_3_COLUMN		(0x00000080)
#define RP_FLAG_4_COLUMN		(0x00000100)
#define RP_FLAG_5_COLUMN		(0x00000200)
#define RP_FLAG_6_COLUMN		(0x00000400)
#define RP_FLAG_PLACE_EAST		(0x00000800)
#define RP_FLAG_HIDDEN			(0x00001000)
#define RP_FLAG_NOLINK			(0x00002000)
#define RP_FLAG_FPGA_UPDATES	(0x00004000) //The FPGA will update this parameter. PC/GUI updates are ignored.  Not yet fully implemented.
#define RP_FLAG_BINARY_XFER    (0x00008000)
#define RP_FLAG_DEBUG          (0x00010000)

//info-message types
#define IM_VOICE				(0x00)

#define EXP_FLAG_DEBUG (0x00000001) //debug the first exp. of the sequence

#endif // _MESSAGING_H_
