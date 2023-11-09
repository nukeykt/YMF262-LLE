#pragma once

typedef struct
{
    int mclk;
    int address;
    int data_i;
    int ic;
    int cs;
    int rd;
    int wr;
} fmopl3_input_t;

typedef struct
{
    fmopl3_input_t input;

    int mclk1;
    int mclk2;
    int aclk1;
    int aclk2;
    int clk1;
    int clk2;
    int rclk1;
    int rclk2;

    int prescaler1_reset[2];
    int prescaler1_cnt[2];

    int prescaler2_reset_l[2];
    int prescaler2_cnt[2];
    int prescaler2_reset;
    int prescaler2_l1[2];
    int prescaler2_l2;
    int prescaler2_l3[2];
    int prescaler2_l4;
    int prescaler2_l5[2];
    int prescaler2_l6[2];
    int prescaler2_l7;

    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_cnt3[2];
    int fsm_cnt;

    int fsm_reset_l[2];
    int fsm_out[17];
    int fsm_l1[2];
    int fsm_l2[2];
    int fsm_l3[2];
    int fsm_l4[2];
    int fsm_l5[2];
    int fsm_l6[2];
    int fsm_l7[2];
    int fsm_l8[2];
    int fsm_l9[2];
    int fsm_l10[2];

    int ic_latch[2];

    int io_rd;
    int io_wr;
    int io_cs;
    int io_a0;
    int io_a1;

    int io_read;
    int io_write;
    int io_write0;
    int io_write1;
    int io_bank;

    int data_latch;
    int bank_latch;
    int bank_masked;

    int reg_sel1;
    int reg_sel2;
    int reg_sel3;
    int reg_sel4;
    int reg_sel5;
    int reg_sel8;
    int reg_selbd;

    int reg_test0;
    int reg_timer1;
    int reg_timer2;
    int reg_notesel;
    int rhythm;
    int reg_rh_kon;
    int reg_da;
    int reg_dv;

    int reg_test1;
    int reg_new;
    int reg_4op;

    int reg_t1_mask;
    int reg_t2_mask;
    int reg_t1_start;
    int reg_t2_start;

    int lfo_cnt[2];
    int vib_cnt[2];
    int t1_step;
    int t2_step;
    int am_step;
    int vib_step;

    int rh_sel0;
    int rh_sel[2];

    int keyon_comb;

    int ra_address_latch;
    int ra_address_good;
    int ra_data_latch;
    int ra_cnt1[2];
    int ra_cnt2[2];
    int ra_cnt3[2];
    int ra_cnt4[2];
    int ra_cnt;
    int ra_rst_l[2];
    int ra_w1_l1;
    int ra_w1_l2;
    int ra_write;
    int ra_write_a;

    int ra_multi[36];
    int ra_ksr[36];
    int ra_egt[36];
    int ra_am[36];
    int ra_vib[36];
    int ra_tl[36];
    int ra_ksl[36];
    int ra_ar[36];
    int ra_dr[36];
    int ra_sl[36];
    int ra_rr[36];
    int ra_wf[36];
    int ra_fnum[18];
    int ra_block[18];
    int ra_keyon[18];
    int ra_connect[18];
    int ra_fb[18];
    int ra_pan[18];
    int ra_connect_pair[18];
    int multi[2];
    int ksr[2];
    int egt[2];
    int am[2];
    int vib[2];
    int tl[2];
    int ksl[2];
    int ar[2];
    int dr[2];
    int sl[2];
    int rr[2];
    int wf[2];
    int fnum[2];
    int block[2];
    int keyon[2];
    int connect[2];
    int fb[2];
    int pan[2];
    int connect_pair[2];

    int write0_sr;
    int write0_l[4];
    int write0;

    int write1_sr;
    int write1_l[4];
    int write1;

    int connect_l[2];
    int connect_pair_l[2];

    int t1_cnt[2];
    int t2_cnt[2];
    int t1_of[2];
    int t2_of[2];
    int t1_status;
    int t2_status;
    int timer_st_load_l;
    int timer_st_load;
    int t1_start;
    int t2_start;
    int t1_start_l[2];
    int t2_start_l[2];

    int reset0;
    int reset1;

    int pg_phase_o[3];
    int pg_dbg[2];
    int pg_dbg_load_l[2];
    int noise_lfsr[2];
    int pg_index[2];
    int pg_cells[36];
    int pg_out_rhy;

    int hh_load;
    int tc_load;
    int hh_bit2;
    int hh_bit3;
    int hh_bit7;
    int hh_bit8;
    int tc_bit3;
    int tc_bit5;

    int tm_w1;
    int tm_w2;
    int tm_w3;
    int tm_w4;
} fmopl3_t;

