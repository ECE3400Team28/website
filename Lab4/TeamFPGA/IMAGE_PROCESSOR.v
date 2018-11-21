`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
    PIXEL_IN,
    CLK,
    VGA_PIXEL_X,
    VGA_PIXEL_Y,
	 VGA_VSYNC_NEG,
    VSYNC,
	 prevVSYNC,
	 prevVSYNC2,
	 prevVSYNC3,
	 PIXEL_OUT,
    HREF,
    RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input   [7:0]   PIXEL_IN;
input       CLK;

// from reading in
input VSYNC;
input prevVSYNC;
input prevVSYNC2;
input prevVSYNC3;
input HREF;

// from VGA
input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input            VGA_VSYNC_NEG;

reg              VGA_VSYNC_PREV;

// edge detection
localparam DIA_THRESHOLD = 16'd35;
localparam TRI_THRESHOLD = 16'd35;
reg [`SCREEN_WIDTH-1:0] currPixB;
reg [`SCREEN_WIDTH-1:0] currPixR;
reg [`SCREEN_WIDTH-1:0] prevPixB;
reg [`SCREEN_WIDTH-1:0] prevPixR;
reg [`SCREEN_WIDTH-1:0] prevPixB2;
reg [`SCREEN_WIDTH-1:0] prevPixR2;
reg [9:0]               prev_VGA_Y;
reg [8:0]               blue_recent;
reg [8:0]               red_recent;
reg [15:0]              num_diag_u;
reg [15:0]              num_diag_d;
reg [15:0]              num_diag_u_p;
reg [15:0]              num_diag_d_p;
reg [15:0]              num_straight;

// color counting
reg [23:0] blue_cnt;
reg [23:0] red_cnt;
reg [23:0] null_cnt;
reg [23:0] blue_cnt_p;
reg [23:0] red_cnt_p;
reg [23:0] null_cnt_p;
localparam R_CNT_THRESHOLD = 24'd8000;
localparam B_CNT_THRESHOLD = 24'd8000;

output reg [2:0] RESULT;
output reg [7:0] PIXEL_OUT;

always @(posedge CLK) begin
	if (prev_VGA_Y != VGA_PIXEL_Y) begin
		prevPixB2 = prevPixB;
		prevPixR2 = prevPixR;
		prevPixB = currPixB;
		prevPixR = currPixR;
		currPixB = {`SCREEN_WIDTH{1'b0}};
		currPixR = {`SCREEN_WIDTH{1'b0}};
	end
	if (VGA_PIXEL_X < `SCREEN_WIDTH && VGA_PIXEL_Y < `SCREEN_HEIGHT) begin
		PIXEL_OUT = 8'b000_000_00;
//		if (PIXEL_IN[1:0] < 2'b10 && PIXEL_IN[4:2] < 3'b101 && PIXEL_IN[7:5] < 3'b101) begin
		if (PIXEL_IN[1:0] <= 2'b10 && PIXEL_IN[4:2] < 3'b101 && PIXEL_IN[7:5] < 3'b101) begin
			// all pixels dark - what tends to be blue
			blue_cnt = blue_cnt + 24'b1;
			currPixB[VGA_PIXEL_X] = 1'b1;
			PIXEL_OUT = 8'b000_000_11;
		end
//		if (PIXEL_IN[7:5] > 3'b010 && PIXEL_IN[4:2] < 3'b011 && PIXEL_IN[1:0] <= 2'b10) begin
		if (PIXEL_IN[7:5] >= 3'b010 && PIXEL_IN[4:2] < 3'b101 && PIXEL_IN[1:0] <= 2'b10) begin
			// anything somewhat red
			red_cnt = red_cnt + 24'b1;
			currPixR[VGA_PIXEL_X] = 1'b1;
			PIXEL_OUT = 8'b111_000_00;
		end
//		if (PIXEL_IN[1:0] >= 2'b10 && PIXEL_IN[4:2] > 3'b101 && PIXEL_IN[7:5] > 3'b101) begin
//			PIXEL_OUT = 8'b111_111_11;
//		end
		if (PIXEL_IN[1:0] < 2'b10) null_cnt = null_cnt + 24'b1;
		
		// color edges
//		if (currPixB[VGA_PIXEL_X] && ~prevPixB[VGA_PIXEL_X]) begin
//			PIXEL_OUT = 8'b000_000_11;
//		end else if (~currPixB[VGA_PIXEL_X] && prevPixB[VGA_PIXEL_X]) begin
//			PIXEL_OUT = 8'b000_000_10;
//		end
//		if (currPixR[VGA_PIXEL_X] && ~prevPixR[VGA_PIXEL_X]) begin
//			PIXEL_OUT = 8'b111_000_00;
//		end else if (~currPixR[VGA_PIXEL_X] && prevPixR[VGA_PIXEL_X]) begin
//			PIXEL_OUT = 8'b011_000_00;
//		end

		if ((VGA_PIXEL_X > 1) && (VGA_PIXEL_X < `SCREEN_WIDTH-2) && (VGA_PIXEL_Y > 1) && (VGA_PIXEL_Y < `SCREEN_HEIGHT-2)) begin
			// for the pixels in the center
			blue_recent[8] = prevPixB2[VGA_PIXEL_X-2];
			blue_recent[7] = prevPixB2[VGA_PIXEL_X-1];
			blue_recent[6] = prevPixB2[VGA_PIXEL_X];
			blue_recent[5] = prevPixB[VGA_PIXEL_X-2];
			blue_recent[4] = prevPixB[VGA_PIXEL_X-1];
			blue_recent[3] = prevPixB[VGA_PIXEL_X];
			blue_recent[2] = currPixB[VGA_PIXEL_X-2];
			blue_recent[1] = currPixB[VGA_PIXEL_X-1];
			blue_recent[0] = currPixB[VGA_PIXEL_X];
			
			red_recent[8] = prevPixR2[VGA_PIXEL_X-2];
			red_recent[7] = prevPixR2[VGA_PIXEL_X-1];
			red_recent[6] = prevPixR2[VGA_PIXEL_X];
			red_recent[5] = prevPixR[VGA_PIXEL_X-2];
			red_recent[4] = prevPixR[VGA_PIXEL_X-1];
			red_recent[3] = prevPixR[VGA_PIXEL_X];
			red_recent[2] = currPixR[VGA_PIXEL_X-2];
			red_recent[1] = currPixR[VGA_PIXEL_X-1];
			red_recent[0] = currPixR[VGA_PIXEL_X];
			
			case (blue_recent)
				9'b111_110_100: begin
					// upper left
					PIXEL_OUT = 8'b000_000_11;
					num_diag_u = num_diag_u + 1;
				end
				9'b111_011_001: begin
					// upper right
					PIXEL_OUT = 8'b000_000_11;
					num_diag_u = num_diag_u + 1;
				end
				9'b001_011_111: begin
					// bottom right
					PIXEL_OUT = 8'b000_000_11;
					num_diag_d = num_diag_d + 1;
				end
				9'b100_110_111: begin
					// bottom left
					PIXEL_OUT = 8'b000_000_11;
					num_diag_d = num_diag_d + 1;
				end
//				9'b000_111_111: begin
//					// bottom
//					PIXEL_OUT = 8'b000_000_10;
//					num_straight = num_straight + 1;
//				end
//				9'b111_111_000: begin
//					// top
//					PIXEL_OUT = 8'b000_000_10;
//					num_straight = num_straight + 1;
//				end
//				9'b011_011_011: begin
//					// right
//					PIXEL_OUT = 8'b000_000_10;
//					num_straight = num_straight + 1;
//				end
//				9'b110_110_110: begin
//					// left
//					PIXEL_OUT = 8'b000_000_10;
//					num_straight = num_straight + 1;
//				end
				default: PIXEL_OUT = PIXEL_OUT;
			endcase
			
			case (red_recent)
				9'b111_110_100: begin
					// upper left
					PIXEL_OUT = 8'b111_000_00;
					num_diag_u = num_diag_u + 1;
				end
				9'b111_011_001: begin
					// upper right
					PIXEL_OUT = 8'b111_000_00;
					num_diag_u = num_diag_u + 1;
				end
				9'b001_011_111: begin
					// bottom right
					PIXEL_OUT = 8'b111_000_00;
					num_diag_d = num_diag_d + 1;
				end
				9'b100_110_111: begin
					// bottom left
					PIXEL_OUT = 8'b111_000_00;
					num_diag_d = num_diag_d + 1;
				end
//				9'b000_111_111: begin
//					// bottom
//					PIXEL_OUT = 8'b011_000_00;
//					num_straight = num_straight + 1;
//				end
//				9'b111_111_000: begin
//					// top
//					PIXEL_OUT = 8'b011_000_00;
//					num_straight = num_straight + 1;
//				end
//				9'b011_011_011: begin
//					// right
//					PIXEL_OUT = 8'b011_000_00;
//					num_straight = num_straight + 1;
//				end
//				9'b110_110_110: begin
//					// left
//					PIXEL_OUT = 8'b011_000_00;
//					num_straight = num_straight + 1;
//				end
				default: PIXEL_OUT = PIXEL_OUT;
			endcase
		end
	end
	
	if (~VGA_VSYNC_NEG & VGA_VSYNC_PREV) begin
		// negedge VGA VSYNC
		blue_cnt_p = (blue_cnt_p - (blue_cnt_p >> 4) ) + (blue_cnt >> 4);
		red_cnt_p = (red_cnt_p - (red_cnt_p >> 4) ) + (red_cnt >> 4);
		
		// first order low pass
		num_diag_u_p = (num_diag_u_p - (num_diag_u_p >> 4) ) + (num_diag_u >> 4);
		num_diag_d_p = (num_diag_d_p - (num_diag_d_p >> 4) ) + (num_diag_d >> 4);
		
		RESULT = 3'b000;
		if (blue_cnt_p > B_CNT_THRESHOLD) begin
			if (num_diag_d_p > DIA_THRESHOLD) RESULT = 3'b011;
			else if (num_diag_u_p > TRI_THRESHOLD) RESULT = 3'b101;
			else RESULT = 3'b001;
		end
		if (red_cnt_p > R_CNT_THRESHOLD) begin
			if (num_diag_d_p > DIA_THRESHOLD) RESULT = 3'b100;
			else if (num_diag_u_p > TRI_THRESHOLD) RESULT = 3'b110;
			else RESULT = 3'b010;
		end
		
//		RESULT = 3'b000;
//		// currently blue is more sensitive so we prioritize red
//		if (red_cnt_p > R_CNT_THRESHOLD) RESULT[1] = 1'b1;
//		else if (blue_cnt_p > B_CNT_THRESHOLD) RESULT[2] = 1'b1;
//		if (null_cnt > B_CNT_THRESHOLD) RESULT[0] = 1'b1;
//		if (num_diag_p > DIA_THRESHOLD) RESULT[2:1] = 2'b10;
//		else if (num_diag_p > TRI_THRESHOLD) RESULT[2:1] = 2'b01;
//		else if (red_cnt_p > R_CNT_THRESHOLD) RESULT[2:1] = 2'b01;
//		else RESULT[2:1] = 2'b00;
		
//		RESULT[0] = (red_cnt_p > R_CNT_THRESHOLD);
//		RESULT[2] = (num_diag > DIA_THRESHOLD);
//		RESULT[1] = (num_straight > STR_THRESHOLD);
//		RESULT[0] = (num_straight > num_diag);
	end else if (VGA_VSYNC_NEG & ~VGA_VSYNC_PREV) begin
		// posedge VGA VSYNC - reset things
		blue_cnt = 0;
		red_cnt = 0;
		null_cnt = 0;
		
		num_diag_u = 0;
		num_diag_d = 0;
		num_straight = 0;
	end
	
	VGA_VSYNC_PREV = VGA_VSYNC_NEG;
	prev_VGA_Y = VGA_PIXEL_Y;
//	PIXEL_OUT = PIXEL_IN;
end

endmodule