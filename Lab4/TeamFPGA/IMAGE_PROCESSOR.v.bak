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
	RESULT
);


//=======================================================
//  PARAMETER declarations
//=======================================================
localparam square_thresh = 1056;
localparam y_bound_0 = 47;
localparam y_bound_1 = 95;
localparam y_bound_2 = 143;

//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;

reg [13:0] RGB_pixel_count [`NUM_BARS-1:0][2:0]; // [ROW][R | G | B ]
//reg [13:0] pixel_counter;
//reg [1:0]  bar_counter;

reg [2:0] triangle_detect;
reg [2:0] square_detect;
reg [2:0] circle_detect;

assign RESULT = {triangle_detect[2:0], square_detect[2:0], circle_detect[2:0]};

wire[2:0] r_val,
			 g_val,
			 b_val;
			 
assign r_val = PIXEL_IN[7:5];
assign g_val = PIXEL_IN[4:2];
assign b_val = {1'b0, PIXEL_IN[1:0]};

//=======================================================
//  UPDATE PIXEL COUNT
//=======================================================
always @ (posedge CLK)begin
		if(VGA_VSYNC_NEG == 1'b0)begin
				RGB_pixel_count [2][2] <= 14'd0;
				RGB_pixel_count [2][1] <= 14'd0;
				RGB_pixel_count [2][0] <= 14'd0;
				RGB_pixel_count [1][2] <= 14'd0;
				RGB_pixel_count [1][1] <= 14'd0;
				RGB_pixel_count [1][0] <= 14'd0;
				RGB_pixel_count [0][2] <= 14'd0;
				RGB_pixel_count [0][1] <= 14'd0;
				RGB_pixel_count [0][0] <= 14'd0;
		end
		
		else if(VGA_PIXEL_X < `SCREEN_WIDTH && VGA_PIXEL_Y < `SCREEN_HEIGHT)begin
				if( VGA_PIXEL_Y < y_bound_0 )begin
						if(r_val > g_val && r_val > b_val) RGB_pixel_count [2][2] <= RGB_pixel_count[2][2] + 14'd1;			//TOP  ROW
						else if(g_val > r_val && g_val > b_val) RGB_pixel_count [2][1] <= RGB_pixel_count [2][1] + 14'd1;
						else if(b_val > r_val && b_val > g_val) RGB_pixel_count [2][0] <= RGB_pixel_count [2][0] + 14'd1;
				end 
				else if( VGA_PIXEL_Y < y_bound_1 )begin
						if(r_val > g_val && r_val > b_val) RGB_pixel_count [1][2] <= RGB_pixel_count[1][2] + 14'd1;			
						else if(g_val > r_val && g_val > b_val) RGB_pixel_count [1][1] <= RGB_pixel_count [1][1] + 14'd1;
						else if(b_val > r_val && b_val > g_val) RGB_pixel_count [1][0] <= RGB_pixel_count [1][0] + 14'd1;
				end
				else begin
						if(r_val > g_val && r_val > b_val) RGB_pixel_count [0][2] <= RGB_pixel_count[0][2] + 14'd1;			//BOTTOM ROW
						else if(g_val > r_val && g_val > b_val) RGB_pixel_count [0][1] <= RGB_pixel_count [0][1] + 14'd1;
						else if(b_val > r_val && b_val > g_val) RGB_pixel_count [0][0] <= RGB_pixel_count [0][0] + 14'd1;
				end
		end
		
		// RED TRIANGLE
		if (RGB_pixel_count[2][2] > RGB_pixel_count [1][2] && RGB_pixel_count [1][2] > RGB_pixel_count[0][2])begin
			triangle_detect <= 3'b100; 
		end
		// BLUE TRIANGLE
		else if (RGB_pixel_count[2][1] > RGB_pixel_count [1][1] && RGB_pixel_count [1][1] > RGB_pixel_count[0][1])begin
			triangle_detect <= 3'b010; 
		end
		// GREEN TRIANGLE
		else if (RGB_pixel_count[2][0] > RGB_pixel_count [1][0] && RGB_pixel_count [1][0] > RGB_pixel_count[0][0])begin
			triangle_detect <= 3'b001; 
		end
end
		
//		if(pixel_counter == (`SCREEN_WIDTH * `BAR_HEIGHT - 14'd1))begin
//				(bar_counter == NUM_BARS-2'd1) ? bar_counter <= 2'd0 : bar_counter <= bar_counter + 2'd1;
//				pixel_counter <= 14'd0;
//		end
//		else pixel_counter<= pixel_counter + 14'd1;


endmodule