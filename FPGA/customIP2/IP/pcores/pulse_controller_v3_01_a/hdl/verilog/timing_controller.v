`timescale 1ns / 1ps

/* fifo2: precise timing fifo to control experiments
	
	This is a finite state machine (FSM) connected to a 64 bit wide FIFO,
	and a DDS controller module.
	
	The FSM pulls instructions from the FIFO, and executes them for a precise number 
	of clock cycles.  Instructions consist of one 8-byte FIFO word.
	
	bits 0...3, instruction
	
		Currently there are three instructions:
		0 TIMED_OUTPUT (6 cycles minimum, 4000000 maximum)
				bits 10...31 = duration (in clock cycles, must be at least 6)
				bits 31...63 = output word 
				
		1 DDS_CONTROL (50 cycles)
				bits 15...31 = DDS opcode
				bits 32...63 = DDS operand
				
		2 PMT_READ (read PMT data into rFIFO, 10 cycles)
		
		3 CLEAR_UNDERFLOW (5 cycles)
		
	bit 4, disable underflow flag.  Prevents underflow from going high.
	
	bit 5, count pulses flag.  Enables the PMT counter during this pulse.
*/

module timing_controller(clock, reset, 
								 iFIFO_data, iFIFO_RdReq, iFIFO_RdAck, iFIFO_empty,
								 rFIFO_data, rFIFO_WrReq, rFIFO_WrAck, rFIFO_full,
								 dds_addr, dds_data_I, dds_data_O, dds_data_T, dds_control, dds_cs,
								 ttl_out, underflow_out, counter_in, sync_in,
								 correlation_config_in, correlation_data_out, correlation_data_ready);

parameter MAX_VAL 					= 32'hFFFFFFFF;
parameter iFIFO_WIDTH 				= 64;
parameter rFIFO_WIDTH 				= 64;
parameter TTL_WIDTH					= 32;
parameter TIMER_WIDTH 				= 24;				 
parameter N_DDS 						= 8;
parameter N_COUNTER 					= 1;
parameter DDS_OPCODE_WIDTH 		= 16;
parameter DDS_OPERAND_WIDTH 		= 32;

parameter INSTRUCTION_WIDTH 		= 4;
parameter PMT_ENABLE_BIT			= 5;
parameter ENABLE_TIMING_CHECK_BIT	= 4;
parameter TIMER_BITA					= 8;
parameter TIMER_BITB					= TIMER_BITA + TIMER_WIDTH - 1;
parameter TTL_BITA					= 32;
parameter TTL_BITB					= TTL_BITA + TTL_WIDTH - 1;
 
input  clock;
input  reset;

input  [0:(iFIFO_WIDTH-1)]	iFIFO_data;
output iFIFO_RdReq;
input  iFIFO_RdAck;
input  iFIFO_empty;

output [0:(rFIFO_WIDTH-1)] rFIFO_data;
output rFIFO_WrReq;
input rFIFO_WrAck;
input rFIFO_full;

output [0:5] dds_addr;

//tri-state for dds_data to allow read & write
output [0:7] dds_data_O;
input  [0:7] dds_data_I;
output dds_data_T;

output [0:3] dds_control;
output [0:(N_DDS-1)] dds_cs;

input [0:(N_COUNTER-1)] counter_in;

input sync_in;
input [0:5] correlation_config_in;
output [0:255] correlation_data_out;
output correlation_data_ready;

output [0:(TTL_WIDTH-1)] 	ttl_out;
output underflow_out;

reg   [0:(TTL_WIDTH-1)]   ttl_out_reg;
reg	[0:(TIMER_WIDTH-1)] timer;
reg	[0:(TIMER_WIDTH-1)] iFIFO_timer;

reg 	[2:0] state;

reg	timing_check;
reg	underflow;
reg overflow;

reg   iFIFO_RdReq;

assign ttl_out = ttl_out_reg;
assign underflow_out = underflow;


reg  dds_we;

reg [0:(DDS_OPCODE_WIDTH-1)]  dds_opcode;
reg [0:(DDS_OPERAND_WIDTH-1)] dds_operand;

wire dds_WrReq;
wire [0:31] dds_result;

dds_controller #(.N_DDS(N_DDS))
					dds_controller_inst(.clock(clock), .reset(reset), 
											  .write_enable(dds_we), .opcode(dds_opcode), .operand(dds_operand),
											  .dds_addr(dds_addr), 
											  .dds_data_I(dds_data_I), .dds_data_O(dds_data_O), 
											  .dds_data_T(dds_data_T), .dds_control(dds_control), .dds_cs(dds_cs),
											  .result_data(dds_result), .result_WrReq(dds_WrReq), .result_WrAck(rFIFO_WrAck));


reg PMT_enable; //enable counting on PMT
reg PMT_RdReq;  //raise high to read result into FIFO

wire PMT_WrReq;
wire [0:31] PMT_result;

PMT_counter #(.N_COUNTER(N_COUNTER))
				 PMT_counter_inst(.clock(clock), .reset(reset), .counter_in(counter_in), .count_enable(PMT_enable), 
										.get_result(PMT_RdReq), .result_data(PMT_result), .result_WrReq(PMT_WrReq), .result_WrAck(rFIFO_WrAck));

PMT_correlation PMT_correlation_inst(
	.clk(clock),
	.reset(reset),
	.configdata_in(correlation_config_in[1:5]),
	.configwrite_in(correlation_config_in[0]),
	.gate_in(PMT_enable),
	.data_out(correlation_data_out),
	.dataready_out(correlation_data_ready),
	.pmt_in(counter_in),
	.sync_in(sync_in));

//allow either PMT_counter or DDS_controller to write into the rFIFO
assign rFIFO_WrReq 	= dds_WrReq  | PMT_WrReq;

//seems like we can only read bits 32...63 from the FIFO w/ the PPC
assign rFIFO_data[32:63] = dds_result | PMT_result; 
assign rFIFO_data[0:31] = 0;

always @(posedge clock or posedge reset) begin
	if(reset) begin
		state <= 0;
		ttl_out_reg <= 0;
		timer <= 0;
		iFIFO_RdReq <= 0;
		dds_we <= 0;
		timing_check <= 0;
		underflow <= 0;
		overflow <= 0;
		PMT_enable <= 0;
		PMT_RdReq <= 0;
	end else begin
		overflow <= (rFIFO_WrReq & rFIFO_full) | overflow;
		
		case (state)
			0: state <= 1;
			
			//Set FIFO read enable to load the next word, if the fifo has data.
			//If the fifo is empty and this is not the last pulse, set underflow high.
			1: if(!iFIFO_empty && !overflow) begin
					 state <= 2;
					 iFIFO_RdReq <= 1;
					 PMT_RdReq <= 0; 
				 end else begin
					 underflow <= (underflow || timing_check);
					 PMT_RdReq <= 0; 
				 end
			
			//Wait for iFIFO_RdAck, which means there will be data on the next cycle
			2: begin
					iFIFO_RdReq <= 0;
					
					if(iFIFO_RdAck) 
						state <= 3;
				end
			
			//New data on iFIFO_data
			3: begin
					state <= 4;
					timing_check <= iFIFO_data[ENABLE_TIMING_CHECK_BIT];
					PMT_enable <= iFIFO_data[PMT_ENABLE_BIT];
					
					case(iFIFO_data[0:(INSTRUCTION_WIDTH-1)])
						0 : begin // set digital output
								timer <= iFIFO_data[TIMER_BITA:TIMER_BITB];
								ttl_out_reg <= iFIFO_data[TTL_BITA:TTL_BITB];
							 end
							 
						1 : begin
								dds_opcode <= iFIFO_data[15:31];
								dds_operand <= iFIFO_data[32:63];
								dds_we <= 1; // write to DDS
								timer <= 50;
							 end
							 
						2 : begin
								PMT_RdReq <= 1;
								timer <= 10;
							 end
							 
						3 : begin 
						        underflow <= 0;
						        timer <= 5;
						    end
						        
					   
						default : timer <= 1000;
					endcase
				 end
					
			4 : begin //decrement timer until it equals the minimum pulse time
					dds_we <= 0; 
					PMT_RdReq <= 0;
				
					if(timer == 4) state <= 1;	// minimum pulse time is 4 cycles		
					else timer <= timer + MAX_VAL;
				  end
		endcase
	end
end

endmodule
