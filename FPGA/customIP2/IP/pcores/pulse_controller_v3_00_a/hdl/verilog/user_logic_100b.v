//----------------------------------------------------------------------------
// user_logic.v - module
//----------------------------------------------------------------------------
//
// ***************************************************************************
// ** Copyright (c) 1995-2006 Xilinx, Inc.  All rights reserved.            **
// **                                                                       **
// ** Xilinx, Inc.                                                          **
// ** XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"         **
// ** AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND       **
// ** SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,        **
// ** OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,        **
// ** APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION           **
// ** THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,     **
// ** AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE      **
// ** FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY              **
// ** WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE               **
// ** IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR        **
// ** REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF       **
// ** INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       **
// ** FOR A PARTICULAR PURPOSE.                                             **
// **                                                                       **
// ***************************************************************************
//
//----------------------------------------------------------------------------
// Filename:          user_logic.v
// Version:           1.00.b
// Description:       User logic module.
// Date:              Thu Jun 22 18:07:45 2006 (by Create and Import Peripheral Wizard)
// Verilog Standard:  Verilog-2001
//----------------------------------------------------------------------------
// Naming Conventions:
//   active low signals:                    "*_n"
//   clock signals:                         "clk", "clk_div#", "clk_#x"
//   reset signals:                         "rst", "rst_n"
//   generics:                              "C_*"
//   user defined types:                    "*_TYPE"
//   state machine next state:              "*_ns"
//   state machine current state:           "*_cs"
//   combinatorial signals:                 "*_com"
//   pipelined or register delay signals:   "*_d#"
//   counter signals:                       "*cnt*"
//   clock enable signals:                  "*_ce"
//   internal version of output port:       "*_i"
//   device pins:                           "*_pin"
//   ports:                                 "- Names begin with Uppercase"
//   processes:                             "*_PROCESS"
//   component instantiations:              "<ENTITY_>I_<#|FUNC>"
//----------------------------------------------------------------------------

module user_logic
(
  // -- ADD USER PORTS BELOW THIS LINE ---------------
  // --USER ports added here 
  pulse_io,
  dds_addr, dds_data_I, dds_data_O, dds_data_T, dds_control, dds_cs,
  counter_in, fullness,
  // -- ADD USER PORTS ABOVE THIS LINE ---------------

  // -- DO NOT EDIT BELOW THIS LINE ------------------
  // -- Bus protocol ports, do not add to or delete 
  Bus2IP_Clk,                     // Bus to IP clock
  Bus2IP_Reset,                   // Bus to IP reset
  Bus2IP_Data,                    // Bus to IP data bus for user logic
  Bus2IP_BE,                      // Bus to IP byte enables for user logic
  Bus2IP_Burst,                   // Bus to IP burst-mode qualifier
  Bus2IP_RdCE,                    // Bus to IP read chip enable for user logic
  Bus2IP_WrCE,                    // Bus to IP write chip enable for user logic
  Bus2IP_RdReq,                   // Bus to IP read request
  Bus2IP_WrReq,                   // Bus to IP write request
  IP2Bus_Data,                    // IP to Bus data bus for user logic
  IP2Bus_Retry,                   // IP to Bus retry response
  IP2Bus_Error,                   // IP to Bus error response
  IP2Bus_ToutSup,                 // IP to Bus timeout suppress
  IP2Bus_AddrAck,                 // IP to Bus address acknowledgement
  IP2Bus_Busy,                    // IP to Bus busy response
  IP2Bus_RdAck,                   // IP to Bus read transfer acknowledgement
  IP2Bus_WrAck,                   // IP to Bus write transfer acknowledgement
  IP2RFIFO_WrReq,                 // IP to RFIFO : IP write request
  IP2RFIFO_Data,                  // IP to RFIFO : IP write data
  RFIFO2IP_WrAck,                 // RFIFO to IP : RFIFO write acknowledge
  RFIFO2IP_AlmostFull,            // RFIFO to IP : RFIFO almost full
  RFIFO2IP_Full,                  // RFIFO to IP : RFIFO full
  RFIFO2IP_Vacancy,               // RFIFO to IP : RFIFO vacancy
  IP2WFIFO_RdReq,                 // IP to WFIFO : IP read request
  WFIFO2IP_Data,                  // WFIFO to IP : WFIFO read data
  WFIFO2IP_RdAck,                 // WFIFO to IP : WFIFO read acknowledge
  WFIFO2IP_AlmostEmpty,           // WFIFO to IP : WFIFO almost empty
  WFIFO2IP_Empty,                 // WFIFO to IP : WFIFO empty
  WFIFO2IP_Occupancy              // WFIFO to IP : WFIFO occupancy
  // -- DO NOT EDIT ABOVE THIS LINE ------------------
); // user_logic

// -- ADD USER PARAMETERS BELOW THIS LINE ------------
// --USER parameters added here 
parameter WFIFO_OCC_WIDTH = 10;
parameter U_PULSE_WIDTH	= 32;
parameter N_DDS = 8;
parameter N_COUNTER = 1;
// -- ADD USER PARAMETERS ABOVE THIS LINE ------------

// -- DO NOT EDIT BELOW THIS LINE --------------------
// -- Bus protocol parameters, do not add to or delete
parameter C_DWIDTH                       = 32;
parameter C_NUM_CE                       = 4;
parameter C_RDFIFO_DWIDTH                = 32;
parameter C_RDFIFO_DEPTH                 = 16;
parameter C_WRFIFO_DWIDTH                = 64;
parameter C_WRFIFO_DEPTH                 = 512;
// -- DO NOT EDIT ABOVE THIS LINE --------------------

// -- ADD USER PORTS BELOW THIS LINE -----------------
// --USER ports added here 
output [0:(U_PULSE_WIDTH-1)] pulse_io;

//DDS ports
output [0:5] dds_addr;

//tri-state for dds_data to allow read & write
output [0:7] dds_data_O;
input  [0:7] dds_data_I;
output dds_data_T;

output [0:3] dds_control;
output [0:(N_DDS-1)] dds_cs;

input [0:(N_COUNTER-1)] counter_in;

output [0:3] fullness;

wire underflow_out;
wire [0:31] counter_out;

// -- ADD USER PORTS ABOVE THIS LINE -----------------

// -- DO NOT EDIT BELOW THIS LINE --------------------
// -- Bus protocol ports, do not add to or delete
input                                     Bus2IP_Clk;
input                                     Bus2IP_Reset;
input      [0 : C_DWIDTH-1]               Bus2IP_Data;
input      [0 : C_DWIDTH/8-1]             Bus2IP_BE;
input                                     Bus2IP_Burst;
input      [0 : C_NUM_CE-1]               Bus2IP_RdCE;
input      [0 : C_NUM_CE-1]               Bus2IP_WrCE;
input                                     Bus2IP_RdReq;
input                                     Bus2IP_WrReq;
output     [0 : C_DWIDTH-1]               IP2Bus_Data;
output                                    IP2Bus_Retry;
output                                    IP2Bus_Error;
output                                    IP2Bus_ToutSup;
output                                    IP2Bus_AddrAck;
output                                    IP2Bus_Busy;
output                                    IP2Bus_RdAck;
output                                    IP2Bus_WrAck;
output                                    IP2RFIFO_WrReq;
output     [0 : C_RDFIFO_DWIDTH-1]        IP2RFIFO_Data;
input                                     RFIFO2IP_WrAck;
input                                     RFIFO2IP_AlmostFull;
input                                     RFIFO2IP_Full;
input      [0 : 4]                        RFIFO2IP_Vacancy;
output                                    IP2WFIFO_RdReq;
input      [0 : C_WRFIFO_DWIDTH-1]        WFIFO2IP_Data;
input                                     WFIFO2IP_RdAck;
input                                     WFIFO2IP_AlmostEmpty;
input                                     WFIFO2IP_Empty;
input      [0 : WFIFO_OCC_WIDTH-1]        WFIFO2IP_Occupancy;
// -- DO NOT EDIT ABOVE THIS LINE --------------------

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------

  // --USER nets declarations added here, as needed for user logic

  // Nets for user logic slave model s/w accessible register example
  reg        [0 : C_DWIDTH-1]               slv_reg0;
  reg        [0 : C_DWIDTH-1]               slv_reg1;
  reg        [0 : C_DWIDTH-1]               slv_reg2;
  reg        [0 : C_DWIDTH-1]               slv_reg3;
  wire       [0 : 3]                        slv_reg_write_select;
  wire       [0 : 3]                        slv_reg_read_select;
  reg        [0 : C_DWIDTH-1]               slv_ip2bus_data;
  wire                                      slv_read_ack;
  wire                                      slv_write_ack;
  integer                                   byte_index, bit_index;

  // --USER logic implementation added here

  // ------------------------------------------------------
  // Example code to read/write user logic slave model s/w accessible registers
  // 
  // Note:
  // The example code presented here is to show you one way of reading/writing
  // software accessible registers implemented in the user logic slave model.
  // Each bit of the Bus2IP_WrCE/Bus2IP_RdCE signals is configured to correspond
  // to one software accessible register by the top level template. For example,
  // if you have four 32 bit software accessible registers in the user logic, you
  // are basically operating on the following memory mapped registers:
  // 
  //    Bus2IP_WrCE or   Memory Mapped
  //       Bus2IP_RdCE   Register
  //            "1000"   C_BASEADDR + 0x0
  //            "0100"   C_BASEADDR + 0x4
  //            "0010"   C_BASEADDR + 0x8
  //            "0001"   C_BASEADDR + 0xC
  // 
  // ------------------------------------------------------
  
  assign
    slv_reg_write_select = Bus2IP_WrCE[0:3],
    slv_reg_read_select  = Bus2IP_RdCE[0:3],
    slv_write_ack        = Bus2IP_WrCE[0] || Bus2IP_WrCE[1] || Bus2IP_WrCE[2] || Bus2IP_WrCE[3],
    slv_read_ack         = Bus2IP_RdCE[0] || Bus2IP_RdCE[1] || Bus2IP_RdCE[2] || Bus2IP_RdCE[3];

  // implement slave model register(s)
  always @( posedge Bus2IP_Clk )
    begin: SLAVE_REG_WRITE_PROC

      if ( Bus2IP_Reset == 1 ) begin
          slv_reg0 <= 0;
          slv_reg1 <= 0;
          slv_reg2 <= 0;
          slv_reg3 <= 0;
      end else begin
        slv_reg3[0] <= underflow_out;

        case ( slv_reg_write_select )
	  4'b1000 :
            for ( byte_index = 0; byte_index <= (C_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg0[bit_index] <= Bus2IP_Data[bit_index];

	  4'b0100 :
            for ( byte_index = 0; byte_index <= (C_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg1[bit_index] <= Bus2IP_Data[bit_index];

          4'b0010 :
            for ( byte_index = 0; byte_index <= (C_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg2[bit_index] <= Bus2IP_Data[bit_index];
 
          default : ;
        endcase
		 end
    end // SLAVE_REG_WRITE_PROC

  // implement slave model register read mux
  always @( slv_reg_read_select or slv_reg0 or slv_reg1 or slv_reg2 or slv_reg3 )
    begin: SLAVE_REG_READ_PROC

      case ( slv_reg_read_select )
        4'b1000 : slv_ip2bus_data <= slv_reg0;
        4'b0100 : slv_ip2bus_data <= slv_reg1;
        4'b0010 : slv_ip2bus_data <= slv_reg2;
        4'b0001 : slv_ip2bus_data <= slv_reg3;
        default : slv_ip2bus_data <= 0;
      endcase

    end // SLAVE_REG_READ_PROC

  // ------------------------------------------------------------
  // Example code to drive IP to Bus signals
  // ------------------------------------------------------------

  assign IP2Bus_Data        = slv_ip2bus_data;
  assign IP2Bus_WrAck       = slv_write_ack;
  assign IP2Bus_RdAck       = slv_read_ack;
  assign IP2Bus_AddrAck     = slv_write_ack || slv_read_ack;
  assign IP2Bus_Busy        = 0;
  assign IP2Bus_Error       = 0;
  assign IP2Bus_Retry       = 0;
  assign IP2Bus_ToutSup     = 0;

wire [0:U_PULSE_WIDTH] ttl_out;

assign pulse_io = (ttl_out | slv_reg0) & (~slv_reg1);

//use our names instead of the generated ones
timing_controller #(.N_DDS(N_DDS), .N_COUNTER(N_COUNTER), .iFIFO_WIDTH(C_WRFIFO_DWIDTH), .rFIFO_WIDTH(C_RDFIFO_DWIDTH))
						tc(.clock(Bus2IP_Clk), .reset(Bus2IP_Reset), 
							.iFIFO_data(WFIFO2IP_Data), .iFIFO_RdReq(IP2WFIFO_RdReq), 
							.iFIFO_RdAck(WFIFO2IP_RdAck), .iFIFO_empty(WFIFO2IP_Empty),
							.rFIFO_data(IP2RFIFO_Data), .rFIFO_WrReq(IP2RFIFO_WrReq),
							.rFIFO_WrAck(RFIFO2IP_WrAck), .rFIFO_full(RFIFO2IP_Full),
							.dds_addr(dds_addr), 
						    .dds_data_I(dds_data_I), .dds_data_O(dds_data_O), 
						    .dds_data_T(dds_data_T), .dds_control(dds_control), .dds_cs(dds_cs),
							.ttl_out(ttl_out), .underflow_out(underflow_out),
							.counter_in(counter_in));
							
assign fullness[0:2] = ~WFIFO2IP_Occupancy[1:3];
assign fullness[3] = ~underflow_out;

endmodule
