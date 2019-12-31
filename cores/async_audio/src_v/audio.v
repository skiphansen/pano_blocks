// 96 Khz async audio output block

`define CHIP_SCOPE = 1
module audio(
    input wire  clk12,  // MCLK and BCLK
    input wire  reset12,

    output reg  codec_dacdat,
    output reg  codec_daclrc,
    input  wire codec_adcdat,
    output reg  codec_adclrc,
    input  wire [15:0] audio_right_sample,
    input  wire [15:0] audio_left_sample
);

`ifdef CHIP_SCOPE
    (* mark_debug = "TRUE" *) wire  debug_clk12;
    (* mark_debug = "TRUE" *) wire  debug_reset12;
    (* mark_debug = "TRUE" *) wire debug_codec_dacdat;
    (* mark_debug = "TRUE" *) wire debug_codec_daclrc;
    (* mark_debug = "TRUE" *) wire [15:0] debug_audio_right_sample;

    assign debug_clk12 = clk12;
    assign debug_reset12 = reset12;
    assign debug_codec_dacdat = codec_dacdat;
    assign debug_codec_daclrc = codec_daclrc;
    assign debug_audio_right_sample = audio_right_sample;
`endif

// We'll use 96 Khz so our sample rate is MCLK / 125

    reg [15:0] l_right;
    reg [15:0] l_left;
    reg [15:0] right;
    reg [15:0] left;
    reg [5:0]  bit_cntr;
    reg last_audio_sample_clk;
    reg start_cycle;
    reg last_bclk;
    reg [6:0] clk_div;

    always @(posedge clk12) 
    if (reset12) begin
        bit_cntr        <= 6'd32;
        codec_adclrc    <= 0;
        last_audio_sample_clk <= 0;
        start_cycle <= 0;
        last_bclk <= 0;
        clk_div <= 0;
    end
    else begin
        l_right <= audio_right_sample;
        l_left <= audio_left_sample;
        if(clk_div == 7'd124) begin
            clk_div <= 0;
            start_cycle <= 1;
            right <= l_right;
            left <= l_left;
        end
        else begin
            clk_div <= clk_div + 1;
        end

        if(start_cycle) begin
            start_cycle <= 0;
            bit_cntr     <= 1;
            codec_daclrc <= 1;
            codec_dacdat <= left[15];
        end
        else begin
            codec_daclrc    <= 0;
            if (bit_cntr < 16) begin
                bit_cntr <= bit_cntr + 1;
                codec_dacdat <= left[~bit_cntr[3:0]];
            end
            else if (bit_cntr < 32) begin
                bit_cntr <= bit_cntr + 1;
                codec_dacdat <= right[~bit_cntr[3:0]];
            end
        end
    end

endmodule

