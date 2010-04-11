`timescale 1ns / 1ps

//opcode[0:3] 0=set_freq, 1=set_phase, 2=set_byte, 3=get_byte, 4=reset, 5=get_frequency, 6=get_phase
//opcode[4:7] DDS number (0...7)


module dds_controller(clock, reset, write_enable, opcode, operand, 
							 dds_addr, dds_data_I, dds_data_O, dds_data_T, dds_control, dds_cs,
							 result_data, result_WrReq, result_WrAck);

// synthesis attribute iostandard of dds_bus is LVCMOS33;

parameter N_DDS = 8;
parameter DDS_OPCODE_WIDTH = 16;
parameter DDS_OPERAND_WIDTH = 32;
parameter RESULT_WIDTH = 32;

input  clock;
input  reset;
input  write_enable;
input  [0:(DDS_OPCODE_WIDTH-1)]  opcode;
input  [0:(DDS_OPERAND_WIDTH-1)] operand;

output [0:5] dds_addr;

//tri-state for dds_data to allow read & write
output [0:7] dds_data_O;
input  [0:7] dds_data_I;
output dds_data_T; //dds_data_T = 0 means output, dds_data_T = 1 means high-Z

output [0:3] dds_control;
output [0:(N_DDS-1)] dds_cs;

reg [0:5] dds_addr_reg;
reg [0:7] dds_data_reg;
reg dds_data_T_reg;

reg dds_w_strobe_n;
reg dds_r_strobe_n;
reg dds_FUD;
reg dds_reset;
reg [0:(N_DDS-1)] dds_cs_reg;

assign dds_addr = dds_addr_reg;
assign dds_data_O = dds_data_reg;
assign dds_data_T = dds_data_T_reg;

assign dds_control = {dds_w_strobe_n, dds_r_strobe_n, dds_FUD, dds_reset};
assign dds_cs = dds_cs_reg;

output [0:(RESULT_WIDTH-1)] result_data;
output result_WrReq;
input result_WrAck;

reg [0:(DDS_OPERAND_WIDTH-1)] operand_reg;
reg [0:(DDS_OPCODE_WIDTH-1)]  opcode_reg;

reg result_WrReq_reg;
reg [0:(RESULT_WIDTH-1)] result_reg;

assign result_WrReq = result_WrReq_reg;
assign result_data = result_reg;



reg [0:3]  cycle;
reg [0:1]  sub_cycle;


always @(posedge clock or posedge reset)
begin
	if(reset)
	begin
		cycle <= 0;
		sub_cycle <= 0;
		
		operand_reg <= 0;
		opcode_reg <= 0;
	
		dds_w_strobe_n 	<= 1;
		dds_r_strobe_n 	<= 1;
		dds_FUD 				<= 0;
		dds_reset			<= 0;
		
		dds_cs_reg <= 8'hFF;
		
		dds_addr_reg 	<= 0;
		dds_data_reg 	<= 0;
		dds_data_T_reg <= 1;
		
		result_WrReq_reg <= 0;
	end
	else
	begin	
		if(cycle == 0)	begin
			dds_w_strobe_n 	<= 1;
			dds_r_strobe_n 	<= 1;
			dds_FUD 				<= 0;
			dds_reset			<= 0;
			
			dds_addr_reg 	<= 0;
			dds_data_reg 	<= 0;
			dds_data_T_reg <= 0;
			
			result_WrReq_reg <= 0;
			result_reg <= 0;
			
			sub_cycle <= 0;
						
			//wait for write_enable to start
			if(write_enable) begin
				//chip select
				dds_cs_reg <= ~(1 << opcode[4:7]);
				
				//latch in operand and operand
				operand_reg    <= operand;
				opcode_reg		<= opcode;
				
				cycle <= 1;
			end else begin 
				dds_cs_reg <= 8'hFF;
			end
		end else begin
			case (opcode_reg[0:3])
			0 : begin // set frequency for profile 0
					case(cycle)
					1 : begin dds_addr_reg <= 6'h0a; end
					2 : begin dds_data_reg <= operand_reg[24:31]; dds_w_strobe_n <= 0; end
					3 : begin dds_addr_reg <= 6'h0b; dds_w_strobe_n <= 1; end
					4 : begin dds_data_reg <= operand_reg[16:23]; dds_w_strobe_n <= 0; end
					5 : begin dds_addr_reg <= 6'h0c; dds_w_strobe_n <= 1; end
					6 : begin dds_data_reg <= operand_reg[8:15]; dds_w_strobe_n <= 0; end
					7 : begin dds_addr_reg <= 6'h0d; dds_w_strobe_n <= 1; end
					8 : begin dds_data_reg <= operand_reg[0:7]; dds_w_strobe_n <= 0; end
					9 : dds_w_strobe_n <= 1;
					10: dds_FUD <= 1; 
					endcase
				  end
				  
			1 : begin // set phase for profile 0
					case(cycle)
					1 : begin dds_addr_reg <= 6'h0e; end
					2 : begin dds_data_reg <= operand_reg[24:31]; dds_w_strobe_n <= 0; end
					3 : begin dds_addr_reg <= 6'h0f; dds_w_strobe_n <= 1; end
					4 : begin dds_data_reg <= operand_reg[16:23]; dds_w_strobe_n <= 0; end
					5 : dds_w_strobe_n <= 1;
					10: dds_FUD <= 1; 
					endcase
				  end
				  
			2 : begin // set memory byte
					case(cycle)
					1 : begin dds_addr_reg <= operand_reg[8:13]; end
					2 : begin dds_data_reg <= operand_reg[0:7]; dds_w_strobe_n <= 0; end
					3 : dds_w_strobe_n <= 1;
					10: dds_FUD <= 1; 
					endcase
				  end
				  
			3 : begin // get memory byte
					case(cycle)
					1 : begin dds_addr_reg <= operand_reg[8:13]; dds_data_T_reg <= 1; result_reg <= 0; end
					3 : dds_r_strobe_n <= 0;
					5 : result_reg[24:31] <= dds_data_I; 
					7 : dds_r_strobe_n <= 1; 
					11: result_WrReq_reg <= 1;
					endcase
				  end
				  
			4 : begin // DDS reset
					case(cycle)
					1 : dds_reset <= 0;
					5 : dds_reset <= 1;
					endcase
				 end
				 
			5 : begin // get frequency for profile 0
					case(cycle)
					1 : begin dds_addr_reg <= 6'h0a; dds_data_T_reg <= 1; result_reg <= 0; end
					2 : begin dds_r_strobe_n <= 0; end
					3 : begin result_reg[24:31] <= dds_data_I; dds_addr_reg <= 6'h0b; dds_r_strobe_n <= 1; end
					4 : begin dds_r_strobe_n <= 0; end
					5 : begin result_reg[16:23] <= dds_data_I; dds_addr_reg <= 6'h0c; dds_r_strobe_n <= 1; end
					6 : begin dds_r_strobe_n <= 0; end
					7 : begin result_reg[8:15] <= dds_data_I; dds_addr_reg <= 6'h0d; dds_r_strobe_n <= 1; end
					8 : begin dds_r_strobe_n <= 0; end
					9 : begin result_reg[0:7] <= dds_data_I; dds_r_strobe_n <= 1; end
					11: result_WrReq_reg <= 1;
					endcase
				 end
				  
			6 : begin // get phase for profile 0
					case(cycle)
					1 : begin dds_addr_reg <= 6'h0e; dds_data_T_reg <= 1; result_reg <= 0; end
					2 : begin dds_r_strobe_n <= 0; end
					3 : begin result_reg[24:31] <= dds_data_I; dds_addr_reg <= 6'h0f; dds_r_strobe_n <= 1; end
					4 : begin dds_r_strobe_n <= 0; end
					5 : begin result_reg[16:23] <= dds_data_I; dds_r_strobe_n <= 1; end
					11: result_WrReq_reg <= 1;
					endcase
				  end				  
			endcase
			
			if(cycle == 11) cycle <= 0;
			else begin
				sub_cycle <= sub_cycle + 1;
				if(sub_cycle == 2'b11)
					cycle <= cycle + 1;
			end
		end
	end
end
	
		
endmodule
