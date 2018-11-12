`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
    PIXEL_IN,
    CLK,
    VGA_PIXEL_X,
    VGA_PIXEL_Y,
    VSYNC,
	 prevVSYNC,
	 prevVSYNC2,
	 prevVSYNC3,
    HREF,
    RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input   [7:0]   PIXEL_IN;
input       CLK;

input VSYNC;
input prevVSYNC;
input prevVSYNC2;
input prevVSYNC3;
input HREF;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
//input           VGA_VSYNC_NEG;

output reg [2:0] RESULT;

reg [23:0] blue_cnt;
reg [23:0] red_cnt;
reg [23:0] null_cnt;

reg [23:0] blue_cnt_p;
reg [23:0] red_cnt_p;
reg [23:0] null_cnt_p;

localparam R_CNT_THRESHOLD = 24'd25000;
localparam B_CNT_THRESHOLD = 24'd20000;

always @(posedge CLK) begin
    if (HREF) begin
        if (PIXEL_IN[1:0] > 2'b10 && PIXEL_IN[4:2] < 3'b011 && PIXEL_IN[7:5] < 3'b011) blue_cnt = blue_cnt + 24'b1;
		  //if (PIXEL_IN[1:0] < 2'b10) blue_cnt = blue_cnt + 24'b1;
//        else if (PIXEL_IN[7:5] > 3'b010) red_cnt = red_cnt + 24'b1;
		  else if (PIXEL_IN[7:5] > 3'b010 && PIXEL_IN[4:2] < 3'b011) red_cnt = red_cnt + 24'b1;
		  if (PIXEL_IN[1:0] < 2'b10) null_cnt = null_cnt + 24'b1;
        //else null_cnt = null_cnt + 24'b1;
    end
	 if (VSYNC & prevVSYNC & ~prevVSYNC2 & ~prevVSYNC3) begin
//    if (VSYNC & ~lastSync) begin
        // posedge vsync
		  blue_cnt_p = (blue_cnt_p - (blue_cnt_p >> 3) ) + (blue_cnt >> 3);
		  red_cnt_p = (red_cnt_p - (red_cnt_p >> 3) ) + (red_cnt >> 3);
		  
		  RESULT = 3'b000;
        if (blue_cnt_p > B_CNT_THRESHOLD) RESULT[2] = 1'b1;
        if (red_cnt_p > R_CNT_THRESHOLD) RESULT[1] = 1'b1;
		  if (null_cnt > B_CNT_THRESHOLD) RESULT[0] = 1'b1;
    end
	 if (~VSYNC & ~prevVSYNC & prevVSYNC2 & prevVSYNC3) begin
    //if (~VSYNC & lastSync) begin
        // negedge vsync
        blue_cnt = 0;
        red_cnt = 0;
        null_cnt = 0;
    end
end

endmodule