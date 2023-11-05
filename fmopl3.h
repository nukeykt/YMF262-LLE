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
} fmopl3_t;

