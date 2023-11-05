#include "fmopl3.h"


void FMOPL3_Clock(fmopl3_t *chip)
{
    chip->mclk1 = !chip->input.mclk;
    chip->mclk2 = chip->input.mclk;

    if (chip->mclk1)
    {
        chip->prescaler1_reset[0] = (chip->prescaler1_reset[1] << 1) | chip->reset1;

        if (!chip->prescaler1_reset[1] && chip->reset1)
            chip->prescaler1_cnt[0] = 0;
        else
            chip->prescaler1_cnt[0] = (chip->prescaler1_cnt[1] + 1) & 3;
    }
    if (chip->mclk2)
    {
        chip->prescaler1_reset[1] = chip->prescaler1_reset[0];
        chip->prescaler1_cnt[1] = chip->prescaler1_cnt[0];
    }

    chip->prescaler1_clk = chip->tm_w1 ? chip->input.mclk : !(chip->prescaler1_cnt[1] & 2);

    chip->aclk1 = !chip->prescaler1_clk;
    chip->aclk2 = chip->prescaler1_clk;

    if (chip->aclk1)
    {
        chip->prescaler2_reset_l[0] = (chip->prescaler2_reset_l[1] << 1) | chip->reset1;

        if (prescaler2_reset)
            chip->prescaler2_cnt[0] = 0;
        else
            chip->prescaler2_cnt[0] = (chip->prescaler2_cnt[1] + 1) & 3;

        chip->prescaler2_l1[0] = !chip->prescaler2_reset && (chip->prescaler2_cnt[1] & 1) == 0;
        chip->prescaler2_l2 = chip->prescaler2_l1[1];

        chip->prescaler2_l3[0] = !chip->prescaler2_reset && (chip->prescaler2_cnt[1] & 1) != 0;
        chip->prescaler2_l4 = chip->prescaler2_l3[1];

        chip->prescaler2_l5[0] = !chip->prescaler2_reset && chip->prescaler2_cnt[1] == 3;

        chip->prescaler2_l6[0] = !chip->prescaler2_reset && chip->prescaler2_cnt[1] == 1;

        chip->prescaler2_l7 = (chip->prescaler2_cnt[1] & 1) == 0;
    }
    if (chip->aclk2)
    {
        chip->prescaler2_reset_l[1] = chip->prescaler2_reset_l[0];
        chip->prescaler2_cnt[1] = chip->prescaler2_cnt[0];
        chip->prescaler2_l1[1] = chip->prescaler2_l1[0];
        chip->prescaler2_l3[1] = chip->prescaler2_l3[0];
        chip->prescaler2_l5[1] = chip->prescaler2_l5[0];
        chip->prescaler2_l6[1] = chip->prescaler2_l6[0];
    }

    chip->prescaler2_reset = !chip->prescaler2_reset_l[1] && chip->reset1;

    chip->clk1 = chip->prescaler2_l1[1] && !chip->prescaler2_l2;
    chip->clk2 = chip->prescaler2_l3[1] && !chip->prescaler2_l4;

    chip->rclk1 = chip->prescaler2_l6[1];
    chip->rclk2 = chip->prescaler2_l5[1];

    chip->bclk = !chip->prescaler2_reset && chip->prescaler2_l7 && (chip->prescaler2_cnt[1] & 1) == 0;
}
