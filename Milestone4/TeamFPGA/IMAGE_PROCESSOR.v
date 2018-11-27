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
localparam DIA_THRESHOLD = 12'd18;
localparam DIA_THRESHOLD_2 = 12'd12;
localparam TRI_THRESHOLD = 12'd18;
reg [`SCREEN_WIDTH-1:0] currPixB;
reg [`SCREEN_WIDTH-1:0] currPixR;
reg [`SCREEN_WIDTH-1:0] prevPixB;
reg [`SCREEN_WIDTH-1:0] prevPixR;
reg [`SCREEN_WIDTH-1:0] prevPixB2;
reg [`SCREEN_WIDTH-1:0] prevPixR2;
reg [9:0]               prev_VGA_Y;
reg [8:0]               blue_recent;
reg [8:0]               red_recent;
reg [11:0]              num_diag_u_r;
reg [11:0]              num_diag_d_r;
reg [11:0]              num_diag_u_r_p;
reg [11:0]              num_diag_d_r_p;
reg [11:0]              num_diag_u_b;
reg [11:0]              num_diag_d_b;
reg [11:0]              num_diag_u_b_p;
reg [11:0]              num_diag_d_b_p;

// color counting
reg [23:0] blue_cnt;
reg [23:0] red_cnt;
//reg [23:0] null_cnt;
reg [23:0] blue_cnt_p;
reg [23:0] red_cnt_p;
//reg [23:0] null_cnt_p;
localparam R_CNT_THRESHOLD = 24'd6000;
localparam B_CNT_THRESHOLD = 24'd6000;

reg [23:0]              total_brightness;
reg [23:0]              total_brightness_p;
wire 							is_bright;
localparam BRIGHT_THRESHOLD = 24'd240000;

assign is_bright = time_ctr[25] && (total_brightness_p > BRIGHT_THRESHOLD);

output reg [2:0] RESULT;
output reg [7:0] PIXEL_OUT;

reg [7:0] PIXEL_OUT_E;
reg [7:0] PIXEL_OUT_C;

reg [25:0] time_ctr;
reg [1:0]  curr_output_sel;

// change what output comes out to VGA every few seconds
always @(posedge CLK) begin
	time_ctr = time_ctr + 26'b1;
	if (time_ctr == 26'd0) begin
		curr_output_sel = curr_output_sel + 2'b1;
	end
	if (curr_output_sel[1] == 1'b1) begin
		PIXEL_OUT = PIXEL_IN;
	end else if (curr_output_sel == 2'b1) begin
		PIXEL_OUT = PIXEL_OUT_C;
	end else begin
		PIXEL_OUT = PIXEL_OUT_E;
	end
end

//assign PIXEL_OUT = PIXEL_OUT_E; // or PIXEL_OUT_C, PIXEL_IN

always @(posedge CLK) begin
	PIXEL_OUT_C = 8'b000_000_00;
	PIXEL_OUT_E = 8'b000_000_00;
	if (prev_VGA_Y != VGA_PIXEL_Y) begin
		prevPixB2 = prevPixB;
		prevPixR2 = prevPixR;
		prevPixB = currPixB;
		prevPixR = currPixR;
		currPixB = {`SCREEN_WIDTH{1'b0}};
		currPixR = {`SCREEN_WIDTH{1'b0}};
	end
//	if (VGA_PIXEL_X < `SCREEN_WIDTH && VGA_PIXEL_Y < `SCREEN_HEIGHT) begin
	if (VGA_PIXEL_X > `SCREEN_WIDTH/5 && VGA_PIXEL_X < `SCREEN_WIDTH && VGA_PIXEL_Y < `SCREEN_HEIGHT) begin
		total_brightness = total_brightness + PIXEL_IN[7:5] + PIXEL_IN[4:2] + PIXEL_IN[1:0];
	
//		if (PIXEL_IN[1:0] < 2'b10 && PIXEL_IN[4:2] < 3'b101 && PIXEL_IN[7:5] < 3'b101) begin
//		if (PIXEL_IN[1:0] <= 2'b01 && PIXEL_IN[4:2] < time_ctr[25:23] && PIXEL_IN[7:5] < time_ctr[25:23]) begin
//		if (PIXEL_IN[1:0] <= 2'b10 && PIXEL_IN[4:2] < 3'b010 && PIXEL_IN[7:5] < 3'b010) begin
		if (PIXEL_IN[1:0] <= 2'b10+is_bright && PIXEL_IN[4:2] < 3'b010+is_bright && PIXEL_IN[7:5] < 3'b010+is_bright) begin
			// all pixels dark - what tends to be blue
			blue_cnt = blue_cnt + 24'b1;
			currPixB[VGA_PIXEL_X] = 1'b1;
			PIXEL_OUT_C = 8'b000_000_11;
		end
//		if (PIXEL_IN[7:5] > 3'b010 && PIXEL_IN[4:2] < 3'b011 && PIXEL_IN[1:0] <= 2'b10) begin
//		if (PIXEL_IN[7:5] >= 3'b011 && PIXEL_IN[4:2] <= 3'b100 && PIXEL_IN[1:0] <= 2'b10) begin
//		if (PIXEL_IN[7:5] >= time_ctr[25:23] && PIXEL_IN[4:2] <= (3'd7 - time_ctr[25:23]) && PIXEL_IN[1:0] <= 2'b10) begin
//		if (PIXEL_IN[7:5] >= 3'b011 && PIXEL_IN[4:2] <= (3'd7 - time_ctr[25:23]) && PIXEL_IN[1:0] <= 2'b10) begin
//		if (PIXEL_IN[7:5] >= 3'b011 && PIXEL_IN[4:2] <= 3'b010 && PIXEL_IN[1:0] <= 2'b10) begin
		if (PIXEL_IN[7:5] >= 3'b011 && PIXEL_IN[4:2] <= 3'b010+is_bright && PIXEL_IN[1:0] <= 2'b10+is_bright) begin
			// anything somewhat red
			red_cnt = red_cnt + 24'b1;
			currPixR[VGA_PIXEL_X] = 1'b1;
			PIXEL_OUT_C = 8'b111_000_00;
		end else if (PIXEL_IN[7:5] > PIXEL_IN[4:2] && PIXEL_IN[7:6] >= PIXEL_IN[1:0]) begin
			red_cnt = red_cnt + 24'b1;
			currPixR[VGA_PIXEL_X] = 1'b1;
			PIXEL_OUT_C = 8'b011_000_00;
		end
//		if (PIXEL_IN[1:0] >= 2'b10 && PIXEL_IN[4:2] > 3'b101 && PIXEL_IN[7:5] > 3'b101) begin
//			PIXEL_OUT_C = 8'b111_111_11;
//		end
//		if (PIXEL_IN[1:0] < 2'b10) null_cnt = null_cnt + 24'b1;
		
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
					PIXEL_OUT_E = 8'b001_001_11;
					num_diag_u_b = num_diag_u_b + 12'd1;
				end
				9'b111_011_001: begin
					// upper right
					PIXEL_OUT_E = 8'b001_001_11;
					num_diag_u_b = num_diag_u_b + 12'd1;
				end
				9'b001_011_111: begin
					// bottom right
					PIXEL_OUT_E = 8'b001_001_11;
					num_diag_d_b = num_diag_d_b + 12'd1;
				end
				9'b100_110_111: begin
					// bottom left
					PIXEL_OUT_E = 8'b001_001_11;
					num_diag_d_b = num_diag_d_b + 12'd1;
				end
				default: PIXEL_OUT_E = PIXEL_OUT_E;
			endcase
			
			case (red_recent)
				9'b111_110_100: begin
					// upper left
					PIXEL_OUT_E = 8'b111_000_00;
					num_diag_u_r = num_diag_u_r + 12'd1;
				end
				9'b111_011_001: begin
					// upper right
					PIXEL_OUT_E = 8'b111_000_00;
					num_diag_u_r = num_diag_u_r + 12'd1;
				end
				9'b001_011_111: begin
					// bottom right
					PIXEL_OUT_E = 8'b111_000_00;
					num_diag_d_r = num_diag_d_r + 12'd1;
				end
				9'b100_110_111: begin
					// bottom left
					PIXEL_OUT_E = 8'b111_000_00;
					num_diag_d_r = num_diag_d_r + 12'd1;
				end
				default: PIXEL_OUT_E = PIXEL_OUT_E;
			endcase
		end
	end
	
	if (~VGA_VSYNC_NEG & VGA_VSYNC_PREV) begin
		// negedge VGA VSYNC
		blue_cnt_p = (blue_cnt_p - (blue_cnt_p >> 4) ) + (blue_cnt >> 4);
		red_cnt_p = (red_cnt_p - (red_cnt_p >> 4) ) + (red_cnt >> 4);
		
//		// first order low pass
//		num_diag_u_p = (num_diag_u_p - (num_diag_u_p >> 4) ) + (num_diag_u >> 4);
//		num_diag_d_p = (num_diag_d_p - (num_diag_d_p >> 4) ) + (num_diag_d >> 4);

		num_diag_u_r_p = (num_diag_u_r_p - (num_diag_u_r_p >> 4) ) + (num_diag_u_r >> 4);
		num_diag_u_b_p = (num_diag_u_b_p - (num_diag_u_b_p >> 4) ) + (num_diag_u_b >> 4);
		num_diag_d_r_p = (num_diag_d_r_p - (num_diag_d_r_p >> 4) ) + (num_diag_d_r >> 4);
		num_diag_d_b_p = (num_diag_d_b_p - (num_diag_d_b_p >> 4) ) + (num_diag_d_b >> 4);
		
		total_brightness_p = total_brightness_p - (total_brightness_p >> 4) + (total_brightness >> 4);
		
		RESULT = 3'b000;
		if (blue_cnt_p > B_CNT_THRESHOLD) RESULT = 3'b001;
		if (num_diag_d_b_p > TRI_THRESHOLD && blue_cnt_p > B_CNT_THRESHOLD/2) RESULT = 3'b101;
		if (num_diag_u_b_p > DIA_THRESHOLD && blue_cnt_p > B_CNT_THRESHOLD/2) RESULT = 3'b011;
		if (blue_cnt_p > B_CNT_THRESHOLD/2) begin
			if (num_diag_d_b_p > TRI_THRESHOLD) RESULT = 3'b101;
			if (num_diag_u_b_p > DIA_THRESHOLD) RESULT = 3'b011;
//			if (num_diag_u_b_p > DIA_THRESHOLD_2 && num_diag_d_b_p > DIA_THRESHOLD_2) RESULT = 3'b011;
		end
		
		if (red_cnt_p > R_CNT_THRESHOLD) RESULT = 3'b010;
		if (red_cnt_p > R_CNT_THRESHOLD/2) begin
			if (num_diag_d_r_p > TRI_THRESHOLD) RESULT = 3'b110;
			if (num_diag_u_r_p > DIA_THRESHOLD) RESULT = 3'b100;
//			if (num_diag_u_r_p > DIA_THRESHOLD_2 && num_diag_d_r_p > DIA_THRESHOLD_2) RESULT = 3'b100;
		end
		
//		if (blue_cnt_p > B_CNT_THRESHOLD) begin
//			if (num_diag_d_p > DIA_THRESHOLD) RESULT = 3'b011;
//			else if (num_diag_u_p > TRI_THRESHOLD) RESULT = 3'b101;
//			else RESULT = 3'b001;
//		end
//		if (red_cnt_p > R_CNT_THRESHOLD) begin
//			if (num_diag_d_p > DIA_THRESHOLD) RESULT = 3'b100;
//			else if (num_diag_u_p > TRI_THRESHOLD) RESULT = 3'b110;
//			else RESULT = 3'b010;
//		end
		
//		RESULT = 3'b000;
//		// currently blue is more sensitive so we prioritize red
//		if (red_cnt_p > R_CNT_THRESHOLD) RESULT[1] = 1'b1;
//		else if (blue_cnt_p > B_CNT_THRESHOLD) RESULT[2] = 1'b1;
//		if (null_cnt > B_CNT_THRESHOLD) RESULT[0] = 1'b1;
//		if (num_diag_p > DIA_THRESHOLD) RESULT[2:1] = 2'b10;
//		else if (num_diag_p > TRI_THRESHOLD) RESULT[2:1] = 2'b01;
//		else if (red_cnt_p > R_CNT_THRESHOLD) RESULT[2:1] = 2'b01;
//		else RESULT[2:1] = 2'b00;
	end else if (VGA_VSYNC_NEG & ~VGA_VSYNC_PREV) begin
		// posedge VGA VSYNC - reset things
		blue_cnt = 0;
		red_cnt = 0;
//		null_cnt = 0;
		
		num_diag_u_b = 0;
		num_diag_d_b = 0;
		num_diag_u_r = 0;
		num_diag_d_r = 0;
		
		
	end
	
	VGA_VSYNC_PREV = VGA_VSYNC_NEG;
	prev_VGA_Y = VGA_PIXEL_Y;
end

endmodule