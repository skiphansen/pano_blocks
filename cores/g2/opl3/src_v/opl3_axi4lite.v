module opl3_axi4lite
//-----------------------------------------------------------------
// Params
//-----------------------------------------------------------------
#(
     parameter NUM_CORES = 1
)
//-----------------------------------------------------------------
// Ports
//-----------------------------------------------------------------
(
    // Inputs
     input          clk_i
    ,input          rst_i
    ,input          cfg_awvalid_i
    ,input  [31:0]  cfg_awaddr_i
    ,input          cfg_wvalid_i
    ,input  [31:0]  cfg_wdata_i
    ,input  [3:0]   cfg_wstrb_i
    ,input          cfg_bready_i
    ,input          cfg_arvalid_i
    ,input  [31:0]  cfg_araddr_i
    ,input          cfg_rready_i
    ,input          clk_opl3   // 25 MHz OPL2 clock

    // Outputs
    ,output         cfg_awready_o
    ,output         cfg_wready_o
    ,output         cfg_bvalid_o
    ,output [1:0]   cfg_bresp_o
    ,output         cfg_arready_o
    ,output         cfg_rvalid_o
    ,output [31:0]  cfg_rdata_o
    ,output [1:0]   cfg_rresp_o

    ,output signed [15:0] channel_a
    ,output signed [15:0] channel_b
    ,output signed [15:0] channel_c
    ,output signed [15:0] channel_d
    ,output sample_clk
    ,output sample_clk_128
);

//-----------------------------------------------------------------
// Retime write data
//-----------------------------------------------------------------
reg [31:0] wr_data_q;

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    wr_data_q <= 32'b0;
else
    wr_data_q <= cfg_wdata_i;

//-----------------------------------------------------------------
// Request Logic
//-----------------------------------------------------------------
wire read_en_w  = cfg_arvalid_i & cfg_arready_o;
wire write_en_w = cfg_awvalid_i & cfg_awready_o;

//-----------------------------------------------------------------
// Accept Logic
//-----------------------------------------------------------------
assign cfg_arready_o = ~cfg_rvalid_o;
assign cfg_awready_o = ~cfg_bvalid_o && ~cfg_arvalid_i; 
assign cfg_wready_o  = cfg_awready_o;


//-----------------------------------------------------------------
// BVALID
//-----------------------------------------------------------------
reg bvalid_q;

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    bvalid_q <= 1'b0;
else if (write_en_w)
    bvalid_q <= 1'b1;
else if (cfg_bready_i)
    bvalid_q <= 1'b0;

assign cfg_bvalid_o = bvalid_q;
assign cfg_bresp_o  = 2'b0;

// Opl3 interface

opl3 opl3_u (
    .clk(clk_i),  // system clock
    .clk_opl3(clk_opl3),  // 25 MHz OPL2 clock
    .opl3_we(write_en_w),   // register write
    .opl3_data(cfg_wdata_i[7:0]), // register data
    .opl3_adr(cfg_awaddr_i[10:2]),  // register address
    .channel_a(channel_a),
    .channel_b(channel_b),
    .channel_c(channel_c),
    .channel_d(channel_d),
    .sample_clk(sample_clk),
    .sample_clk_128(sample_clk_128)   // 128 X sample rate clock
);


endmodule
