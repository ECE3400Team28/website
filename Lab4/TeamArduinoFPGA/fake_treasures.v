module fake_treasures(
	input CLK,
	output reg OUTPUT
);


///// turn this into an input
reg [1:0]  treasure_color;		// 0x for none, 11 for blue, 10 for red
reg [1:0]  treasure_shape;    // 00 for none, 01 for square, 10 for triangle, 11 for diamond

reg [15:0] cnt1;
reg [31:0] cnt2;
reg [15:0] cnt3;

reg CLK9600;

wire DONE;
wire LOAD;
wire LOADSHIFT;

always @ (posedge CLK) begin
	cnt1 = cnt1 + 1;
	if (cnt1 == 16'd2604) begin
		CLK9600 = ~CLK9600;
	end
	if (cnt1 == 16'd5208) begin
		///// Currently for 50MHz, 9600 baud rate
		cnt2 = cnt2 + 1;
		CLK9600 = ~CLK9600;
		cnt1 = 16'b0;
		if (cnt2 == 32'd9600) begin
			///// 1 second loop
			cnt2 = 32'b0;
			cnt3 = cnt3 + 1;
			if (cnt3 == 16'd10) begin
				// 10 second total loop
				cnt3 = 16'd0;
			end
		end
	end
end

always @(*) begin
	case (cnt3)
		16'd0:
			treasure_color = 2'b00;
			treasure_shape = 2'b01;
		16'd1:
			treasure_color = 2'b11;
			treasure_shape = 2'b00;
		16'd2:
			treasure_color = 2'b11;
			treasure_shape = 2'b01;
		16'd3:
			treasure_color = 2'b11;
			treasure_shape = 2'b10;
		16'd4:
			treasure_color = 2'b11;
			treasure_shape = 2'b11;
		16'd6:
			treasure_color = 2'b10;
			treasure_shape = 2'b01;
		16'd7:
			treasure_color = 2'b10;
			treasure_shape = 2'b10;
		16'd8:
			treasure_color = 2'b10;
			treasure_shape = 2'b11;
		16'd9:
			treasure_color = 2'b01;
			treasure_shape = 2'b11;
		default:
			treasure_color = 2'b00;
			treasure_shape = 2'b00;
	endcase
end

// assume CLK9600 exists
// either generate new PLL or ???

///// FSM states
// We might want to only send this once? 
// In which case we enable regs for previous color/shape in count
// and check that they are still the same in Send

reg Scurr;
reg Snext;

parameter 	Send  = 1'b0;
			Count = 1'b1;
			
reg [11:0] ShiftData;
reg  [3:0] ShiftCount;
reg [11:0] Nessage;

wire ShiftDone;
wire SHIFTOUT;

always @ (*) begin
	case (Scurr)
		Send:
			// if treasure color and shape signify that they are valid
			Snext <= (treasure_color[1] && (|treasure_shape) && ShiftDone) ? Count : Init;
		Count:
			Snext <= DONE ? Send : Init;
	endcase
end

always @ (Scurr) begin
	case (scurr)
		Send:
			LOAD = 1'b1;
			LOADSHIFT = 1'b0;
			OUTPUT = SHIFTOUT;
		Count:
			LOAD = 1'b0;
			LOADSHIFT = 1'b1;
			OUTPUT = 1'b0;
		default:
			LOAD = 1'b1;
			LOADSHIFT = 1'b0;
			OUTPUT = 1'b1;
	endcase
end
			
always @ (posedge CLK9600) begin
	Scurr <= Snext;
end

///// WHAT DO WE WANT TO SEND?

always @ (*) begin
	case (treasure_color)
		2'b00:
			// No color
			Message <= 12'b1111_1111_1111;
		2'b01:
			// No color
			Message <= 12'b1111_1111_1111;
		2'b10:
			// Red
			case (treasure_shape)
				2'b00:
					// No shape
					Message <= 12'b1111_1111_1111;
				2'b01:
					// Square
					Message <= 12'b101_0011001_01;
				2'b01:
					// Triangle
					Message <= 12'b101_1011010_01;
				2'b01:
					// Diamond
					Message <= 12'b101_0110011_01;
				default:
					// No shape
					Message <= 12'b1111_1111_1111;
			endcase
		2'b11:
			// Blue
			case (treasure_shape)
				2'b00:
					// No shape
					Message <= 12'b1111_1111_1111;
				2'b01:
					// Square
					Message <= 12'b101_1010101_01;
				2'b01:
					// Triangle
					Message <= 12'b101_0010110_01;
				2'b01:
					// Diamond
					Message <= 12'b101_1111111_01;
				default:
					// No shape
					Message <= 12'b1111_1111_1111;
			endcase
	endcase
end

///// SHIFTER

always @ (posedge CLK9600) begin
	if (LOADSHIFT)
		ShiftCount <= 4'd12;
		ShiftData <= Message;
		SHIFTOUT <= 1'b1;
	else
		ShiftCount <= ShiftCount - 4'b1;
		SHIFTOUT <= ShiftData[11];
		ShiftData <= {ShiftData[10:0], 1'b1};
end

assign ShiftDone = (ShiftCount == 4'b0);

///// COUNTDOWN

countdown cd (
	.CLK(CLK9600),
	.RESET(1'b0),
	.LOAD(LOAD),
	.DATA(10'd960),
	.DONE(DONE)
);

endmodule				