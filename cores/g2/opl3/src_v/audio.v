// `define CHIP_SCOPE = 1

module audio(
    input wire  clk25,
    input wire  reset25,

    input wire codec_bclk_i,
    input  wire [15:0] audio_right_sample,
    input  wire [15:0] audio_left_sample,
    input  wire audio_sample_clk
    );

reg  dacdat;
reg  daclrc;
reg  adclrc;

`ifdef CHIP_SCOPE
(* mark_debug = "TRUE" *) wire  debug_clk25;
(* mark_debug = "TRUE" *) wire  debug_reset25;
(* mark_debug = "TRUE" *) wire debug_codec_bclk_i;
(* mark_debug = "TRUE" *) wire debug_dacdat;
(* mark_debug = "TRUE" *) wire debug_daclrc;
(* mark_debug = "TRUE" *) wire [15:0] debug_audio_right_sample;
(* mark_debug = "TRUE" *) wire debug_audio_sample_clk;

assign debug_clk25 = clk25;
assign debug_reset25 = reset25;
assign debug_codec_bclk_i = codec_bclk_i;
assign debug_dacdat = dacdat;
assign debug_daclrc = daclrc;
assign debug_audio_right_sample = audio_right_sample;
assign debug_audio_sample_clk = audio_sample_clk;
`endif

// The sampling rate for real opl3 is 14.31818MHz/288 = 49715.2777.
// The Wolfson codec supports sample rates of 8kHz, 11.025kHz, 12kHz,
// 16kHz, 22.05kHz, 24kHz, 32kHz, 44.1kHz, 48kHz, 88.2kHz and 96kHz.
// The codec will be configured for the closest available sampling rate 
// of 48 Khz.
// 
// The codec requires a MCLK that is a multiple of the sampling rate(fs).  
// When using a 48Khz sampling rate the choices are 250fs, 256fs or 384fs.
// 
// Considering the above and after much experimentation it was decided to
// use a sampling rate of 50 Khz (25 MHz/500) which is 0.57% faster than ideal.
// MCLK and BCLK are 250fs (25 Mhz / 2).

    reg [5:0]  bit_cntr;
    reg last_audio_sample_clk;
    reg start_cycle;
    reg last_bclk;

    always @(posedge clk25) 
    begin
        last_bclk <= codec_bclk_i;
        if (last_audio_sample_clk != audio_sample_clk) begin
            last_audio_sample_clk <= audio_sample_clk;
        // rising edge of sample clock, start new cycle
            if (audio_sample_clk) begin
                start_cycle <= 1;
            end
        end

        if(codec_bclk_i && !last_bclk) begin
        end
        else if(!codec_bclk_i && last_bclk) begin
            daclrc    <= 0;
            if(start_cycle) begin
                start_cycle <= 0;
                bit_cntr     <= 1;
                daclrc <= 1;
                dacdat <= audio_left_sample[15];
            end
            else if (bit_cntr < 16) begin
                bit_cntr <= bit_cntr + 1;
                dacdat <= audio_left_sample[~bit_cntr[3:0]];
            end
            else if (bit_cntr < 32) begin
                bit_cntr <= bit_cntr + 1;
                dacdat <= audio_right_sample[~bit_cntr[3:0]];
            end
        end

        if (reset25) begin
            bit_cntr        <= 6'd32;
            adclrc    <= 0;
            last_audio_sample_clk <= 0;
            start_cycle <= 0;
            last_bclk <= 0;
        end
    end

    (* s = "true" *) wire codec_dacdat;
    (* keep = "true" *) (* s = "true" *) OBUF dacdat_obuf (
        .I(dacdat),
        .O(codec_dacdat)
    );

    (* s = "true" *) wire codec_daclrc;
    (* keep = "true" *) (* s = "true" *) OBUF daclrc_obuf (
        .I(daclrc),
        .O(codec_daclrc)
    );

    (* s = "true" *) wire codec_adclrc;
    (* keep = "true" *) (* s = "true" *) OBUF adclrc_obuf (
        .I(adclrc),
        .O(codec_adclrc)
    );

    wire bclk_ddr2;
    (* keep = "true" *) (* s = "true" *) ODDR2 bclk_oddr2 (
        .S(1'b0),
        .R(1'b0),
        .D0(1'b1),
        .D1(1'b0),
        .C0(codec_bclk_i),
        .C1(!codec_bclk_i),
        .CE(1'b1),
        .Q(bclk_ddr2)
    );

    (* s = "true" *) wire codec_bclk;
    (* keep = "true" *) (* s = "true" *) OBUF bclk_obuf(
        .I(bclk_ddr2),
        .O(codec_bclk)
    );

    wire mclk_ddr2;
    (* keep = "true" *) (* s = "true" *) ODDR2 mclk_oddr2 (
        .S(1'b0),
        .R(1'b0),
        .D0(1'b1),
        .D1(1'b0),
        .C0(codec_bclk_i),
        .C1(!codec_bclk_i),
        .CE(1'b1),
        .Q(mclk_ddr2)
    );

    (* s = "true" *) wire codec_mclk;
    (* keep = "true" *) (* s = "true" *) OBUF mclk_obuf(
        .I(mclk_ddr2),
        .O(codec_mclk)
    );

endmodule

