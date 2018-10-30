module fake_treasures(
	input CLK,
	output reg OUTPUT
);

reg [15:0] cnt1;
reg [31:0] cnt2;

always @ (posedge CLK) begin
	cnt1 = cnt1 + 1;
	OUTPUT = 1'b1;
	if (cnt1 == 16'd5208) begin
		///// Currently for 50MHz, 9600 baud rate
		cnt2 = cnt2 + 1;
		cnt1 = 16'b0;
		if (cnt2 == 32'd48000) begin
			///// 5 second loop timer
			cnt2 = 32'b0;
		end
	end
	if (cnt2 <= 32'd960) begin
		OUTPUT = 1'b0;
	end else if (cnt2 == 32'd962) begin
		OUTPUT = 1'b0;
	end else if (cnt2 == 32'd963) begin
		OUTPUT = 1'b0;
	end else if (cnt2 == 32'd967) begin
		OUTPUT = 1'b0;
	end else if (cnt2 >= 32'd19200 && cnt2 <= 32'd20160) begin
		OUTPUT = 1'b0;
	end else if (cnt2 == 32'd20165) begin
		OUTPUT = 1'b0;
	end
end

endmodule				