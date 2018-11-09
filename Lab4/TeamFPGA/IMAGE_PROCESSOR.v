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
    HREF,
    RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input   [7:0]   PIXEL_IN;
input       CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input           VGA_VSYNC_NEG;

output reg [2:0] RESULT;

reg [15:0] blue_cnt;
reg [15:0] red_cnt;
reg [15:0] null_cnt;
reg        lastSync;

localparam R_CNT_THRESHOLD = 16'd10000;
localparam B_CNT_THRESHOLD = 16'd10000;

always @(posedge CLK) begin
    if (HREF) begin
        if (PIXEL_IN[1:0] > 2'b01) blue_cnt = blue_cnt + 1;
        else if (PIXEL_IN[7:5] > 3'b010) red_cnt = red_cnt + 1;
        else null_cnt = null_cnt + 1;
    end
    if (VSYNC & ~lastSync) begin
        // posedge vsync
        if (blue_cnt > B_CNT_THRESHOLD) RESULT = 3'b111;
        else if (red_cnt > R_CNT_THRESHOLD) RESULT = 3'b110;
        else RESULT = 3'b000;
    end
    if (~VSYNC & lastSync) begin
        // negedge vsync
        blue_cnt = 0;
        red_cnt = 0;
        null_cnt = 0;
    end
    
    lastSync = VSYNC;
end

endmodule