`timescale 1ns / 1ps

module PMT_counter(clock, reset, counter_in, count_enable, get_result, counter_in,
						 result_data, result_WrReq, result_WrAck);

parameter N_COUNTER = 1;
parameter RESULT_WIDTH = 32;

input  clock;
input  reset;
input  [0:(N_COUNTER-1)] counter_in;
input  count_enable;
input  get_result;

output [0:(RESULT_WIDTH-1)] result_data;
output result_WrReq;
input result_WrAck;


reg result_WrReq_reg;
reg [0:(RESULT_WIDTH-1)] counter;
reg [0:(RESULT_WIDTH-1)] counter_out_reg;

assign result_WrReq = result_WrReq_reg;
assign result_data = counter_out_reg;

reg [0:1] fifo_write_state;

wire gated_counter_in;
wire counter_gate;

assign counter_gate = count_enable & (fifo_write_state == 0);
assign gated_counter_in = counter_in & counter_gate;

//only count while data is not being pushed onto FIFO
always @(posedge gated_counter_in)
begin
	counter = counter + 1;
end

always @(posedge clock or posedge reset)
begin
	if(reset) begin
		result_WrReq_reg <= 0;
		fifo_write_state <= 0;
		counter_out_reg <=0;
	end else begin		
		case(fifo_write_state)
			0: begin 
					counter_out_reg <= 0;
					
					if(get_result == 1) begin //clock data into result FIFO on rising edges of get_result
						fifo_write_state <= 1;
					end
				end
				
		   //add single cycle delay, to allow counter to settle down
			1: begin 
					result_WrReq_reg <= 1;
					fifo_write_state <= 2;
					counter_out_reg <= counter;
				end
				
			2: if(result_WrAck == 1) begin //wait for WrAck
					result_WrReq_reg <= 0;
					fifo_write_state <= 3;
				end
				
			3: if(get_result == 0) begin //wait for get_result to go low
					fifo_write_state <= 0;
				end
		endcase
	end
end
	
		
endmodule
