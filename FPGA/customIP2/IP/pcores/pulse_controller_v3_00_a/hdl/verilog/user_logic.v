//----------------------------------------------------------------------------
// user_logic.vhd - module
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
// Filename:          user_logic.vhd
// Version:           3.00.a
// Description:       User logic module.
// Date:              Sun Aug 23 16:11:49 2009 (by Create and Import Peripheral Wizard)
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
  Bus2IP_Data,                    // Bus to IP data bus
  Bus2IP_BE,                      // Bus to IP byte enables
  Bus2IP_RdCE,                    // Bus to IP read chip enable
  Bus2IP_WrCE,                    // Bus to IP write chip enable
  Bus2IP_Burst,                   // Bus to IP burst-mode qualifier
  Bus2IP_BurstLength,             // Bus to IP burst length
  Bus2IP_RdReq,                   // Bus to IP read request
  Bus2IP_WrReq,                   // Bus to IP write request
  IP2Bus_AddrAck,                 // IP to Bus address acknowledgement
  IP2Bus_Data,                    // IP to Bus data bus
  IP2Bus_RdAck,                   // IP to Bus read transfer acknowledgement
  IP2Bus_WrAck,                   // IP to Bus write transfer acknowledgement
  IP2Bus_Error,                   // IP to Bus error response
  IP2RFIFO_WrReq,                 // IP to RFIFO : IP write request
  IP2RFIFO_Data,                  // IP to RFIFO : IP write data bus
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
parameter C_SLV_DWIDTH                   = 32;
parameter C_NUM_REG                      = 8;
parameter C_RDFIFO_DEPTH                 = 16;
parameter C_WRFIFO_DEPTH                 = 1024;
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
input      [0 : C_SLV_DWIDTH-1]           Bus2IP_Data;
input      [0 : C_SLV_DWIDTH/8-1]         Bus2IP_BE;
input      [0 : C_NUM_REG-1]              Bus2IP_RdCE;
input      [0 : C_NUM_REG-1]              Bus2IP_WrCE;
input                                     Bus2IP_Burst;
input      [0 : 8]                        Bus2IP_BurstLength;
input                                     Bus2IP_RdReq;
input                                     Bus2IP_WrReq;
output                                    IP2Bus_AddrAck;
output     [0 : C_SLV_DWIDTH-1]           IP2Bus_Data;
output                                    IP2Bus_RdAck;
output                                    IP2Bus_WrAck;
output                                    IP2Bus_Error;
output                                    IP2RFIFO_WrReq;
output     [0 : C_SLV_DWIDTH-1]           IP2RFIFO_Data;
input                                     RFIFO2IP_WrAck;
input                                     RFIFO2IP_AlmostFull;
input                                     RFIFO2IP_Full;
input      [0 : 4]                        RFIFO2IP_Vacancy;
output                                    IP2WFIFO_RdReq;
input      [0 : C_SLV_DWIDTH-1]           WFIFO2IP_Data;
input                                     WFIFO2IP_RdAck;
input                                     WFIFO2IP_AlmostEmpty;
input                                     WFIFO2IP_Empty;
input      [0 : 10]                       WFIFO2IP_Occupancy;
// -- DO NOT EDIT ABOVE THIS LINE --------------------

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------

  // --USER nets declarations added here, as needed for user logic

  // Nets for user logic slave model s/w accessible register example
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg0;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg1;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg2;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg3;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg4;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg5;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg6;
  reg        [0 : C_SLV_DWIDTH-1]           slv_reg7;
  wire       [0 : 7]                        slv_reg_write_sel;
  wire       [0 : 7]                        slv_reg_read_sel;
  reg        [0 : C_SLV_DWIDTH-1]           slv_ip2bus_data;
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
  // if you have four 32 bit software accessible registers in the user logic,
  // you are basically operating on the following memory mapped registers:
  // 
  //    Bus2IP_WrCE/Bus2IP_RdCE   Memory Mapped Register
  //                     "1000"   C_BASEADDR + 0x0
  //                     "0100"   C_BASEADDR + 0x4
  //                     "0010"   C_BASEADDR + 0x8
  //                     "0001"   C_BASEADDR + 0xC
  // 
  // ------------------------------------------------------

  assign
    slv_reg_write_sel = Bus2IP_WrCE[0:7],
    slv_reg_read_sel  = Bus2IP_RdCE[0:7],
    slv_write_ack     = Bus2IP_WrCE[0] || Bus2IP_WrCE[1] || Bus2IP_WrCE[2] || Bus2IP_WrCE[3] || Bus2IP_WrCE[4] || Bus2IP_WrCE[5] || Bus2IP_WrCE[6] || Bus2IP_WrCE[7],
    slv_read_ack      = Bus2IP_RdCE[0] || Bus2IP_RdCE[1] || Bus2IP_RdCE[2] || Bus2IP_RdCE[3] || Bus2IP_RdCE[4] || Bus2IP_RdCE[5] || Bus2IP_RdCE[6] || Bus2IP_RdCE[7];

  // implement slave model register(s)
  always @( posedge Bus2IP_Clk )
    begin: SLAVE_REG_WRITE_PROC

      if ( Bus2IP_Reset == 1 )
        begin
          slv_reg0 <= 0;
          slv_reg1 <= 0;
          slv_reg2 <= 0;
          slv_reg3 <= 0;
          slv_reg4 <= 0;
          slv_reg5 <= 0;
          slv_reg6 <= 0;
          slv_reg7 <= 0;
        end
      else begin
        slv_reg3[0] <= underflow_out;
        case ( slv_reg_write_sel )
          8'b10000000 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg0[bit_index] <= Bus2IP_Data[bit_index];
          8'b01000000 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg1[bit_index] <= Bus2IP_Data[bit_index];
          8'b00100000 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg2[bit_index] <= Bus2IP_Data[bit_index];
          8'b00010000 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg3[bit_index] <= Bus2IP_Data[bit_index];
          8'b00001000 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg4[bit_index] <= Bus2IP_Data[bit_index];
          8'b00000100 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg5[bit_index] <= Bus2IP_Data[bit_index];
          8'b00000010 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg6[bit_index] <= Bus2IP_Data[bit_index];
          8'b00000001 :
            for ( byte_index = 0; byte_index <= (C_SLV_DWIDTH/8)-1; byte_index = byte_index+1 )
              if ( Bus2IP_BE[byte_index] == 1 )
                for ( bit_index = byte_index*8; bit_index <= byte_index*8+7; bit_index = bit_index+1 )
                  slv_reg7[bit_index] <= Bus2IP_Data[bit_index];
          default : ;
        endcase
      end
    end // SLAVE_REG_WRITE_PROC

  // implement slave model register read mux
  always @( slv_reg_read_sel or slv_reg0 or slv_reg1 or slv_reg2 or slv_reg3 or slv_reg4 or slv_reg5 or slv_reg6 or slv_reg7 )
    begin: SLAVE_REG_READ_PROC

      case ( slv_reg_read_sel )
        8'b10000000 : slv_ip2bus_data <= slv_reg0;
        8'b01000000 : slv_ip2bus_data <= slv_reg1;
        8'b00100000 : slv_ip2bus_data <= slv_reg2;
        8'b00010000 : slv_ip2bus_data <= slv_reg3;
        8'b00001000 : slv_ip2bus_data <= slv_reg4;
        8'b00000100 : slv_ip2bus_data <= slv_reg5;
        8'b00000010 : slv_ip2bus_data <= slv_reg6;
        8'b00000001 : slv_ip2bus_data <= slv_reg7;
        default : slv_ip2bus_data <= 0;
      endcase

    end // SLAVE_REG_READ_PROC

  // ------------------------------------------------------------
  // Example code to drive IP to Bus signals
  // ------------------------------------------------------------

  assign IP2Bus_AddrAck = slv_write_ack || slv_read_ack;
  assign IP2Bus_Data    = slv_ip2bus_data;
  assign IP2Bus_WrAck   = slv_write_ack;
  assign IP2Bus_RdAck   = slv_read_ack;
  assign IP2Bus_Error   = 0;

wire [0:(U_PULSE_WIDTH-1)] ttl_out;

//assume 64-bit slave register width, and 32-bit pulse width
assign pulse_io = (ttl_out | slv_reg0[0:(U_PULSE_WIDTH-1)]) & (~slv_reg0[U_PULSE_WIDTH:(C_SLV_DWIDTH-1)]);

//use our names instead of the generated ones
timing_controller #(.N_DDS(N_DDS), .N_COUNTER(N_COUNTER), .iFIFO_WIDTH(C_SLV_DWIDTH), .rFIFO_WIDTH(C_SLV_DWIDTH))
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
