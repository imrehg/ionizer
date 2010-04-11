`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// PMT_correlation
// 
// Dave Leibrandt
// 11/18/09
//
// This is heavily based on code written by Jarek Labaziewicz at MIT.
// 
// Count photons and measure the distribution with respect to some sync signal.
// The top level structure is as follows:
// - photon pulses move through a series of N_BINS latches clocked at clk
// - on sync signal, the data on the latches is saved
// - there are N_BINS counters which increment whenever there is a photon in the corresponding latch
//
// In many ways, this is a ridiculous design, but it's very fast.
////////////////////////////////////////////////////////////////////////////////

module PMT_correlation(
   input  wire 		  clk,
   input  wire 		  reset,
   
   input  wire [0:4]	  configdata_in,
   input  wire		     configwrite_in,
	
	input  wire			  gate_in,
   
   output wire [0:255] data_out,
   output wire         dataready_out,
	
   input  wire		     pmt_in,
   input  wire		     sync_in);

// the following two parameters must multiply to get 4*64 = 256 bits
// N_BITS must be 4, 8, 16, or 32
parameter N_BINS = 16; // number of bins 
parameter N_BITS = 16; // number of bits per counter

///////////////////////////////////////////////////////////////////////////////
// Get the configuration

reg removefromqueue_f; // removing photons from queue is optional
reg [0:3] clockdivider_f; // clock the latches at a different speed than the main clock

always @(posedge clk)
	if (reset) begin
	    removefromqueue_f <= 1'b1;
		 clockdivider_f <= 4'h1;
	end
	else if (configwrite_in) begin
	    removefromqueue_f <= configdata_in[0];
		 clockdivider_f <= configdata_in[1:4];
	end

///////////////////////////////////////////////////////////////////////////////
// Generate clk2 at the clock speed divided by clockdivider_f

wire clk2;

reg [0:3] clockcounter;
reg dividedclk;

always @(posedge clk)
	if (reset) begin
	    clockcounter <= 4'h0;
	    dividedclk <= 1'b0;
	end
	else if (clockcounter != clockdivider_f - 1) begin
	    clockcounter <= clockcounter + 1;
	    dividedclk <= 1'b0;
	end
	else begin
	    clockcounter <= 4'h0;
	    dividedclk <= 1'b1;
	end

assign clk2 = (clockdivider_f > 1) ? dividedclk : clk;

///////////////////////////////////////////////////////////////////////////////
// Generate a counting_f, reset_counters, and dataready_out based on gate_in

reg [0:2] counting_buf;
reg reset_counters;
reg [0:(N_BINS-1)] counting2_buf;
wire counting2;

always @(posedge clk) begin
	counting_buf <= {gate_in, counting_buf[0:1]};
	reset_counters <= counting_buf[1] & (~counting_buf[2]);
end

always @(posedge clk2) begin
	counting2_buf <= {gate_in, counting2_buf[0:(N_BINS-2)]};
end
assign counting2 = counting2_buf[1];
assign dataready_out = ~counting2_buf[(N_BINS-1)];

///////////////////////////////////////////////////////////////////////////////
// Generate a 1 clk long sync signal and a 1 clk2 long notsync signal
reg [0:2] sync_buf, sync2_buf;
reg sync_det, sync, sync2_det, sync2, notsync2_det, notsync2;

always @(posedge clk)
	sync_buf <= {sync_in, sync_buf[0:1]};

always @(posedge clk) begin
	sync_det <= sync_buf[1] & (~sync_buf[2]);
	sync <= sync_det;
end

always @(posedge clk2)
	sync2_buf <= {sync_in, sync2_buf[0:1]};

always @(posedge clk2) begin
	sync2_det <= sync2_buf[1] & (~sync2_buf[2]);
	sync2 <= sync2_det;
	notsync2_det <= (~sync2_buf[1]) | sync2_buf[2];
	notsync2 <= notsync2_det;
end

///////////////////////////////////////////////////////////////////////////
// Parallelize pmt_in by sending it down a series of N_BINS latches
reg [0:(N_BINS-1)] photons, photons_sync;
reg [0:2] pmt_buf;
reg pmt_det, pmt, notremovefromqueue2_f;

always @(posedge clk2)
	pmt_buf <= {pmt_in, pmt_buf[0:1]};
	
always @(posedge clk2) begin
	pmt_det <= pmt_buf[1] & (~pmt_buf[2]);
	pmt <= pmt_det;
end

always @(posedge clk2)
	notremovefromqueue2_f <= ~removefromqueue_f;

always @(posedge clk2)
	photons <= {(pmt & counting2), photons[0:(N_BINS-2)] & {(N_BINS-1){notsync2 | notremovefromqueue2_f}}};

always @(posedge clk2)
	if (sync2) photons_sync <= photons;

////////////////////////////////////////////////////////////////////////////////					  
// Count photons on sync

counter #(.N_BITS(N_BITS)) counter1[0:(N_BINS-1)] (
	.count_o(data_out),
	.clk_i(clk),
	.reset_i(reset | reset_counters),
	.photon_i(photons_sync),
	.sync_i(sync));

endmodule

//////////////////////////////////////////////////////////////////////////////////////////////////////
// counter
//
// Internals of a single photon counter. A counter and an output register.
//////////////////////////////////////////////////////////////////////////////////////////////////////
module counter(count_o, clk_i, reset_i, photon_i, sync_i);

parameter N_BITS = 16; // number of bits per counter

output reg  [0:(N_BITS-1)] count_o;
input  wire 					clk_i;
input  wire 					reset_i;
input  wire						photon_i;
input  wire 					sync_i;

	// Latch for speed (if you do not latch sync, it will fail - photon signal sometimes misses by a clock at 333MHz)

	reg photon_f, sync_f, syncdly_f;

	always @(posedge clk_i) begin
		photon_f <= photon_i;
		syncdly_f <= sync_i;
		sync_f <= syncdly_f;
	end

	// Count the photons
	
	always @(posedge clk_i)
		if (reset_i) count_o <= {(N_BITS){1'b0}};
		else if (sync_f & photon_f) count_o <= count_o + 1'b1;

endmodule


