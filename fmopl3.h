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
    int bclk;

    int prescaler1_reset[2];
    int prescaler1_cnt[2];

    int prescaler1_clk;

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

    int reset1;


    int tm_w1;
    int tm_w2;
    int tm_w3;
    int tm_w4;
} fmopl3_t;

