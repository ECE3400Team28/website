`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144


module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);


//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		    [1:0]		KEY;
//////////// GPIO_0, GPIO_2 connect to c0 //////////
//output          GPIO_02;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

//wire			C0_to_GPIO_02;
wire			PLL_24;
wire			PLL_50;
wire			PLL_25;

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

assign GPIO_0_D[33] = PLL_24;

// assign to the GPIO pin the value of the 24 MHz clock
//assign C0_to_GPIO_02 = c0;

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN;

fake_treasures fkt(
	.CLK ( PLL_50),
	.OUTPUT ( GPIO_0_D[30] )
);

thePLL	thePLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( PLL_24 ),
	.c1 ( PLL_25 ),
	.c2 ( PLL_50 )
);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(PLL_24),
	.clk_R(PLL_25), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(PLL_25),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(PLL_25),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


/////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
	READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
	if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
			VGA_READ_MEM_EN = 1'b0;
	end
	else begin
			VGA_READ_MEM_EN = 1'b1;
	end
end

always @ (posedge PLL_25) begin
	W_EN = 1'b0;
	X_ADDR <= X_ADDR + 15'd1;
	if ( X_ADDR == `SCREEN_WIDTH ) begin 
		Y_ADDR <= Y_ADDR + 15'd1;
		end
	if ( Y_ADDR == `SCREEN_HEIGHT ) begin
		X_ADDR <= 15'd0;
		end
	if ( X_ADDR == `SCREEN_WIDTH /2 ) begin
		pixel_data_RGB332 <= GREEN;
		end
	

	W_EN = 1'b1;
end

	
endmodule 