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

wire DONE;
wire LOAD;

always @ (posedge CLK) begin
	cnt1 = cnt1 + 1;
	if (cnt1 == 16'd5208) begin
		///// Currently for 50MHz, 9600 baud rate
		cnt2 = cnt2 + 1;
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

always @ (cnt2) begin
	OUTPUT = 1'b1;
end

always @ (treasure_color, treasure_shape) begin
	LOAD = 1'b1;
end

countdown cd (
	.CLK(cnt2),
	.RESET(1'b0),
	.LOAD(LOAD),
	.DATA(10'd960),
	.DONE(DONE)
);

endmodule				