#include "fmopl3.h"


static const int ch_map[32] = {
    0, 1, 2, -1,
    3, 4, 5, -1,
    6, 7, 8, -1,
    -1, -1, -1, -1,
    9, 10, 11, -1,
    12, 13, 14, -1,
    15, 16, 17, -1,
    -1, -1, -1, -1
};

static const int op_map[64] = {
    0, 1, 2, 3, 4, 5, -1, -1,
    6, 7, 8, 9, 10, 11, -1, -1,
    12, 13, 14, 15, 16, 17, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    18, 19, 20, 21, 22, 23, -1, -1,
    24, 25, 26, 27, 28, 29, -1, -1,
    30, 31, 32, 33, 34, 35, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1
};

void FMOPL3_Clock(fmopl3_t *chip)
{
    chip->mclk1 = !chip->input.mclk;
    chip->mclk2 = chip->input.mclk;

    chip->io_rd = !chip->input.rd;
    chip->io_wr = !chip->input.wr;
    chip->io_cs = !chip->input.cs;
    chip->io_a0 = chip->input.address & 1;
    chip->io_a1 = (chip->input.address & 2) != 0;

    if (chip->mclk1)
    {
        chip->ic_latch[0] = (chip->ic_latch[1] << 1) | (!chip->input.ic);
    }
    if (chip->mclk2)
    {
        chip->ic_latch[1] = chip->ic_latch[0];

        chip->reset0 = (chip->ic_latch[1] & 2) != 0;
    }

    chip->io_read = !chip->reset0 && chip->io_cs && chip->io_rd && !chip->io_a0 && !chip->io_a1;
    chip->io_write = !chip->reset0 && chip->io_cs && chip->io_wr;
    chip->io_write0 = !chip->reset0 && chip->io_cs && chip->io_wr && !chip->io_a0;
    chip->io_write1 = !chip->reset0 && chip->io_cs && chip->io_wr && chip->io_a0;

    if (chip->reset0)
        chip->data_latch = 0;
    else if (chip->io_write)
        chip->data_latch = chip->input.data_i & 255;
    if (chip->reset0)
        chip->bank_latch = 0;
    else if (chip->io_write0)
        chip->bank_latch = chip->io_a1;

    if (chip->mclk2)
    {
        chip->prescaler1_reset[1] = chip->prescaler1_reset[0];
        chip->prescaler1_cnt[1] = chip->prescaler1_cnt[0];
    }

    int prescaler1_clk = (chip->reg_test1 & 0x40) != 0 ? chip->input.mclk : !(chip->prescaler1_cnt[1] & 2);

    chip->aclk1 = !prescaler1_clk;
    chip->aclk2 = prescaler1_clk;

    if (chip->aclk2)
    {
        chip->prescaler2_reset_l[1] = chip->prescaler2_reset_l[0];
        chip->prescaler2_cnt[1] = chip->prescaler2_cnt[0];
        chip->prescaler2_l1[1] = chip->prescaler2_l1[0];
        chip->prescaler2_l3[1] = chip->prescaler2_l3[0];
        chip->prescaler2_l5[1] = chip->prescaler2_l5[0];
        chip->prescaler2_l6[1] = chip->prescaler2_l6[0];
    }


    chip->clk1 = chip->prescaler2_l1[1] && !chip->prescaler2_l2;
    chip->clk2 = chip->prescaler2_l3[1] && !chip->prescaler2_l4;

    chip->rclk1 = chip->prescaler2_l6[1];
    chip->rclk2 = chip->prescaler2_l5[1];

    if (chip->clk2)
    {
        chip->write0_l[1] = chip->write0_l[0];
        chip->write0_l[3] = chip->write0_l[2];

        chip->write1_l[1] = chip->write1_l[0];
        chip->write1_l[3] = chip->write1_l[2];

        chip->write0 = chip->write0_l[3] && !chip->write0_l[1];
        chip->write1 = chip->write1_l[3] && !chip->write1_l[1];
    }

    if (chip->io_write0)
        chip->write0_sr = 1;
    else if (chip->reset0 || chip->write0_l[1])
        chip->write0_sr = 0;

    if (chip->io_write1)
        chip->write1_sr = 1;
    else if (chip->reset0 || chip->write1_l[1])
        chip->write1_sr = 0;

    if (chip->clk1)
    {
        chip->write0_l[0] = chip->write0_l;
        chip->write0_l[2] = chip->write0_l[1];

        chip->write1_l[0] = chip->write1_l;
        chip->write1_l[2] = chip->write1_l[1];
    }

    //////////////////////

    if (chip->reset0)
    {
        chip->reg_sel1 = 0;
        chip->reg_sel2 = 0;
        chip->reg_sel3 = 0;
        chip->reg_sel4 = 0;
        chip->reg_sel5 = 0;
        chip->reg_sel8 = 0;
        chip->reg_selbd = 0;
    }
    else if (chip->write0)
    {
        chip->reg_sel1 = chip->data_latch == 1;
        chip->reg_sel2 = chip->data_latch == 2;
        chip->reg_sel3 = chip->data_latch == 3;
        chip->reg_sel4 = chip->data_latch == 4;
        chip->reg_sel5 = chip->data_latch == 5;
        chip->reg_sel8 = chip->data_latch == 8;
        chip->reg_selbd = chip->data_latch == 0xbd;
    }

    if (chip->reset0)
        chip->reg_new = 0;
    else if (chip->write1 && chip->bank_latch && chip->reg_sel5)
        chip->reg_new = chip->data_latch & 1;

    int bank_masked = chip->reg_new && chip->bank_latch;

    if (chip->reset0)
    {
        chip->reg_test0 = 0;
        chip->reg_test1 = 0;
        chip->reg_timer1 = 0;
        chip->reg_timer2 = 0;
        chip->reg_notesel = 0;
        chip->rhythm = 0;
        chip->reg_rh_kon = 0;
        chip->reg_da = 0;
        chip->reg_dv = 0;
    }
    else if (chip->write1)
    {
        if (chip->reg_sel1 && !bank_masked)
            chip->reg_test0 = chip->data_latch & 255;
        if (chip->reg_sel2 && !bank_masked)
            chip->reg_timer1 = chip->data_latch & 255;
        if (chip->reg_sel3 && !bank_masked)
            chip->reg_timer2 = chip->data_latch & 255;
        if (chip->reg_sel8 && !bank_masked)
        {
            chip->reg_notesel = (chip->data_latch & 64) != 0;
        }
        if (chip->reg_selbd && !bank_masked)
        {
            chip->reg_rh_kon = chip->data_latch & 31;
            chip->rhythm = (chip->data_latch & 32) != 0;
            chip->reg_dv = (chip->data_latch & 64) != 0;
            chip->reg_da = (chip->data_latch & 128) != 0;
        }
        if (chip->reg_sel1 && bank_masked)
            chip->reg_test1 = chip->data_latch & 255;
        if (chip->reg_sel1 && bank_masked)
            chip->reg_4op = chip->data_latch & 63;
    }
    int reg_sel4_wr = chip->write1 && chip->reg_sel4 && !bank_masked && (chip->data_latch & 128) == 0;
    int reg_sel4_rst = (chip->write1 && chip->reg_sel4 && !bank_masked && (chip->data_latch & 128) != 0) || chip->reset0;

    if (chip->reset0)
    {
        chip->reg_t1_mask = 0;
        chip->reg_t2_mask = 0;
        chip->reg_t1_start = 0;
        chip->reg_t2_start = 0;
    }
    else if (reg_sel4_wr)
    {
        chip->reg_t1_mask = (chip->data_latch & 64) != 0;
        chip->reg_t2_mask = (chip->data_latch & 32) != 0;
        chip->reg_t1_start = (chip->data_latch & 1) != 0;
        chip->reg_t2_start = (chip->data_latch & 2) != 0;
    }

    chip->reset1 = chip->reset0 || (chip->reg_test1 & 0xc0) == 0xc0;

    {
        //int bclk = !prescaler2_reset && chip->prescaler2_l7 && (chip->prescaler2_cnt[1] & 1) == 0;

        int ga = (chip->data_latch & 0xe0) != 0;

        if (chip->reset1)
            chip->ra_address_latch = 0;
        else if (chip->write0 && ga)
            chip->ra_address_latch = (bank_masked << 8) | chip->data_latch;
        if (chip->reset1)
            chip->ra_address_good = 0;
        else if (chip->write0)
            chip->ra_address_good = ga;
        if (chip->reset1)
            chip->ra_data_latch = 0;
        else if (chip->write1 && chip->ra_address_good)
            chip->ra_data_latch = chip->data_latch;

        int write0 = ga && chip->write0 && (chip->reg_test1 & 16) != 0;
        int write = chip->write1 || write0;

        if (chip->aclk1)
            chip->ra_w1_l1 = write;
        chip->ra_write = (write && !chip->ra_w1_l1) || (chip->reset1 && chip->clk2);
        if (chip->clk1)
            chip->ra_w1_l2 = write;
        chip->ra_write_a = write && !chip->ra_w1_l2;

        if (chip->clk1)
        {
            chip->ra_rst_l[0] = chip->reset1;
            int rst = (chip->reset1 && !chip->ra_rst_l[1]) || chip->fsm_out[5];

            int of1 = (chip->ra_cnt1[1] & 5) == 5;
            int of2 = (chip->ra_cnt2[1] & 2) == 2 && of1;
            int of4 = (chip->ra_cnt4[1] & 2) == 2;
            if (rst || of1)
                chip->ra_cnt1[0] = 0;
            else
                chip->ra_cnt1[0] = (chip->ra_cnt1[1] + 1) & 7;
            if (rst || of2)
                chip->ra_cnt2[0] = 0;
            else
                chip->ra_cnt2[0] = (chip->ra_cnt2[1] + of1) & 3;
            if (rst)
                chip->ra_cnt3[0] = 0;
            else
                chip->ra_cnt3[0] = (chip->ra_cnt3[1] + of2) & 1;

            if (rst || of4 || of1)
                chip->ra_cnt4[0] = 0;
            else
                chip->ra_cnt4[0] = (chip->ra_cnt1[4] + 1) & 3;

        }
        if (chip->clk2)
        {
            chip->ra_rst_l[1] = chip->ra_rst_l[0];
            chip->ra_cnt1[1] = chip->ra_cnt1[0];
            chip->ra_cnt2[1] = chip->ra_cnt2[0];
            chip->ra_cnt3[1] = chip->ra_cnt3[0];
            chip->ra_cnt4[1] = chip->ra_cnt4[0];
            chip->ra_cnt = (chip->ra_cnt3[1] << 5) | (chip->ra_cnt2[1] << 3) | chip->ra_cnt1[1];
        }

        if (chip->ra_write || chip->clk1)
        {
            int bank = (chip->ra_address_latch & 0x100) != 0;
            int op_address = chip->ra_write_a ? ((chip->ra_address_latch & 0x1f) | (bank << 5)) : chip->ra_cnt;
            int idx = op_map[op_address];
            if (chip->ra_write && idx != -1)
            {
                if ((chip->ra_address_latch & 0xe0) == 0x20 || write0 || chip->reset1)
                {
                    chip->ra_multi[idx] = chip->ra_data_latch & 15;
                    chip->ra_ksr[idx] = (chip->ra_data_latch >> 4) & 1;
                    chip->ra_egt[idx] = (chip->ra_data_latch >> 5) & 1;
                    chip->ra_vib[idx] = (chip->ra_data_latch >> 6) & 1;
                    chip->ra_am[idx] = (chip->ra_data_latch >> 7) & 1;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x40 || write0 || chip->reset1)
                {
                    chip->ra_tl[idx] = chip->ra_data_latch & 63;
                    chip->ra_ksl[idx] = (chip->ra_data_latch >> 6) & 3;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x60 || write0 || chip->reset1)
                {
                    chip->ra_dr[idx] = chip->ra_data_latch & 15;
                    chip->ra_ar[idx] = (chip->ra_data_latch >> 4) & 15;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x80 || write0 || chip->reset1)
                {
                    chip->ra_rr[idx] = chip->ra_data_latch & 15;
                    chip->ra_sl[idx] = (chip->ra_data_latch >> 4) & 15;
                }
                if ((chip->ra_address_latch & 0xe0) == 0xe0 || write0 || chip->reset1)
                {
                    int data = chip->ra_data_latch & 3;
                    if (chip->reg_new)
                        data |= chip->ra_data_latch & 4;
                    chip->ra_wf[idx] = data;
                }
            }
            int ch_address_write = chip->ra_address_latch & 15;
            int add = 0;
            if (ch_address_write == 3 || ch_address_write == 4 || ch_address_write == 5)
                add |= 1;
            if (ch_address_write == 6 || ch_address_write == 7 || ch_address_write == 8)
                add |= 2;
            int ch_address_mapped = (ch_address_write & 1) + (add & 1);
            ch_address_mapped |= add & 2;
            ch_address_mapped += ch_address_write & 14;
            ch_address_mapped |= bank << 4;
            int ch_address_mapped2 = ch_address_mapped & 3;
            if ((ch_address_mapped & 12) == 8)
                ch_address_mapped2 |= 4;
            if ((ch_address_mapped & 12) == 0)
                ch_address_mapped2 |= 8;
            if ((ch_address_mapped & 28) == 0 || (ch_address_mapped & 28) == 20 || (ch_address_mapped & 28) == 24)
                ch_address_mapped2 |= 16;


            int ch_address_read = (chip->ra_cnt4[1] & 3) | (chip->ra_cnt2[1] << 2) | (chip->ra_cnt3[1] << 4);
            int ch_address = chip->ra_write_a ? ch_address_mapped : ch_address_read;
            int ch_address_read_4op = ch_address_read;
            if ((chip->ra_cnt2[1] & 2) == 0)
            {
                switch (chip->ra_cnt3[1] * 4 + chip->ra_cnt4[1])
                {
                    case 0: // 0, 3, 6, 9
                        if (chip->reg_4op & 1)
                            ch_address_read_4op &= ~4;
                        break;
                    case 1: // 1, 4, 7, 10
                        if (chip->reg_4op & 2)
                            ch_address_read_4op &= ~4;
                        break;
                    case 2: // 2, 5, 8, 11
                        if (chip->reg_4op & 4)
                            ch_address_read_4op &= ~4;
                        break;
                    case 4: // 0, 3, 6, 9
                        if (chip->reg_4op & 8)
                            ch_address_read_4op &= ~4;
                        break;
                    case 5: // 1, 4, 7, 10
                        if (chip->reg_4op & 16)
                            ch_address_read_4op &= ~4;
                        break;
                    case 6: // 2, 5, 8, 11
                        if (chip->reg_4op & 32)
                            ch_address_read_4op &= ~4;
                        break;
                }
            }
            int ch_address_4op = chip->ra_write_a ? ch_address_mapped : ch_address_read_4op;
            int ch_address_fb = chip->ra_write_a ? ch_address_mapped2 : ch_address_read;
            
            int idx1 = ch_map[ch_address];
            int idx2 = ch_map[ch_address_4op];
            int idx3 = ch_map[ch_address_fb];
            if (chip->ra_write && idx1 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xc0 || write0 || chip->reset1)
                {
                    chip->ra_connect[idx1] = chip->ra_data_latch & 1;
                    int pan_data = 0;
                    if (!chip->reg_new || chip->reset1)
                        pan_data |= 3;
                    if (chip->reg_new)
                        pan_data |= (chip->ra_data_latch >> 4) & 15;
                    chip->ra_pan[idx1] = pan_data;
                }
            }
            if (chip->ra_write && idx2 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xa0 || write0 || chip->reset1)
                {
                    chip->ra_fnum[idx2] &= 0x300;
                    chip->ra_fnum[idx2] |= chip->ra_data_latch;
                }
                if ((chip->ra_address_latch & 0xf0) == 0xb0 || write0 || chip->reset1)
                {
                    chip->ra_fnum[idx2] &= 0xff;
                    chip->ra_fnum[idx2] |= (chip->ra_data_latch & 3) << 8;
                    chip->ra_block[idx2] = (chip->ra_data_latch >> 2) & 7;
                    chip->ra_keyon[idx2] = (chip->ra_data_latch >> 5) & 1;
                }
            }
            if (chip->ra_write && idx3 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xc0 || write0 || chip->reset1)
                {
                    chip->ra_connect_pair[idx3] = chip->ra_data_latch & 1;
                    chip->ra_fb[idx3] = (chip->ra_data_latch >> 1) & 7;
                }
            }

            if (chip->clk1)
            {
                if (idx != -1)
                {
                    chip->multi[0] = chip->ra_multi[idx];
                    chip->ksr[0] = chip->ra_ksr[idx];
                    chip->egt[0] = chip->ra_egt[idx];
                    chip->am[0] = chip->ra_am[idx];
                    chip->vib[0] = chip->ra_vib[idx];
                    chip->tl[0] = chip->ra_tl[idx];
                    chip->ksl[0] = chip->ra_ksl[idx];
                    chip->ar[0] = chip->ra_ar[idx];
                    chip->dr[0] = chip->ra_dr[idx];
                    chip->sl[0] = chip->ra_sl[idx];
                    chip->rr[0] = chip->ra_rr[idx];
                    chip->wf[0] = chip->ra_wf[idx];
                }
                if (idx1 != 0)
                {
                    chip->connect[0] = chip->ra_connect[idx1];
                    chip->pan[0] = chip->ra_pan[idx1];
                }
                if (idx2 != 0)
                {
                    chip->fnum[0] = chip->ra_fnum[idx2];
                    chip->block[0] = chip->ra_block[idx2];
                    chip->keyon[0] = chip->ra_keyon[idx2];
                }
                if (idx3 != 0)
                {
                    chip->connect_pair[0] = chip->ra_connect_pair[idx3];
                    chip->fb[0] = chip->ra_fb[idx3];
                }
            }
        }
        if (chip->clk2)
        {
            chip->multi[1] = chip->multi[0];
            chip->ksr[1] = chip->ksr[0];
            chip->egt[1] = chip->egt[0];
            chip->am[1] = chip->am[0];
            chip->vib[1] = chip->vib[0];
            chip->tl[1] = chip->tl[0];
            chip->ksl[1] = chip->ksl[0];
            chip->ar[1] = chip->ar[0];
            chip->dr[1] = chip->dr[0];
            chip->sl[1] = chip->sl[0];
            chip->rr[1] = chip->rr[0];
            chip->wf[1] = chip->wf[0];

            chip->connect[1] = chip->connect[0];
            chip->pan[1] = chip->pan[0];

            chip->fnum[1] = chip->fnum[0];
            chip->block[1] = chip->block[0];
            chip->keyon[1] = chip->keyon[0];

            chip->connect_pair[1] = chip->connect_pair[0];
            chip->fb[1] = chip->fb[0];
        }
    }
    
    {
        int fsm_4op = 0;
        switch (chip->fsm_cnt)
        {
        case 5: // 5
            fsm_4op = (chip->reg_4op & 1) != 0;
            break;
        case 8: // 6
            fsm_4op = (chip->reg_4op & 2) != 0;
            break;
        case 9: // 7
            fsm_4op = (chip->reg_4op & 4) != 0;
            break;
        case 37: // 23
            fsm_4op = (chip->reg_4op & 8) != 0;
            break;
        case 40: // 24
            fsm_4op = (chip->reg_4op & 16) != 0;
            break;
        case 41: // 25
            fsm_4op = (chip->reg_4op & 32) != 0;
            break;
        }
        int con_4op = fsm_4op && (chip->fsm_l10[1] & 4) != 0; // 01 connect

        if (chip->clk1)
        {
            int fsm_reset = (chip->fsm_reset_l[1] & 2) == 0 && chip->reset1;
            chip->fsm_reset_l[0] = (chip->fsm_reset_l[1] << 1) | chip->reset1;

            int fsm_of1 = (chip->fsm_cnt1[1] & 5) == 5;
            int fsm_of2 = (chip->fsm_cnt2[1] & 2) == 2 && fsm_of1;

            if (fsm_reset || fsm_of1)
                chip->fsm_cnt1[0] = 0;
            else
                chip->fsm_cnt1[0] = (chip->fsm_cnt1[1] + 1) & 7;

            if (fsm_reset || fsm_of2)
                chip->fsm_cnt2[0] = 0;
            else
                chip->fsm_cnt2[0] = (chip->fsm_cnt2[1] + fsm_of1) & 3;

            if (fsm_reset)
                chip->fsm_cnt3[0] = 0;
            else
                chip->fsm_cnt3[0] = (chip->fsm_cnt3[1] + fsm_of2) & 1;

            chip->fsm_l1[0] = chip->fsm_cnt == 53;
            chip->fsm_l2[0] = chip->fsm_cnt == 16;
            chip->fsm_l3[0] = chip->fsm_cnt == 20;
            chip->fsm_l4[0] = chip->fsm_cnt == 52;
            chip->fsm_l5[0] = (chip->fsm_l5[1] << 1) | ((chip->fsm_cnt & 56) == 0);
            chip->fsm_l6[0] = (chip->fsm_l6[1] << 1) | ((chip->fsm_cnt & 56) == 8 || (chip->fsm_cnt & 62) == 16);
            chip->fsm_l7[0] = (chip->fsm_l7[1] << 1) | ((chip->fsm_cnt & 56) == 40 || (chip->fsm_cnt & 62) == 48);
            chip->fsm_l8[0] = (chip->fsm_l8[1] << 1) | ((chip->fsm_cnt & 48) == 16);
            chip->fsm_l9[0] = (chip->fsm_l9[1] << 1) | con_4op;
            chip->fsm_l10[0] = (chip->fsm_l10[1] << 1) | ((chip->connect_l[1] & 2) == 0 && (chip->connect_pair_l[1] & 2) != 0);
        }
        if (chip->clk2)
        {
            chip->fsm_reset_l[1] = chip->fsm_reset_l[0];
            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];
            chip->fsm_cnt3[1] = chip->fsm_cnt3[0];

            chip->fsm_cnt = (chip->fsm_cnt3[1] << 5) | (chip->fsm_cnt2[1] << 3) | chip->fsm_cnt1[1];

            chip->fsm_l1[1] = chip->fsm_l1[0];
            chip->fsm_l2[1] = chip->fsm_l2[0];
            chip->fsm_l3[1] = chip->fsm_l3[0];
            chip->fsm_l4[1] = chip->fsm_l4[0];
            chip->fsm_l5[1] = chip->fsm_l5[0];
            chip->fsm_l6[1] = chip->fsm_l6[0];
            chip->fsm_l7[1] = chip->fsm_l7[0];
            chip->fsm_l8[1] = chip->fsm_l8[0];
            chip->fsm_l9[1] = chip->fsm_l9[0];
        }
        {
            chip->fsm_out[0] = chip->fsm_l1[1]; // 0
            chip->fsm_out[1] = chip->fsm_cnt == 16; // 12
            chip->fsm_out[2] = chip->fsm_l2[1]; // 13
            chip->fsm_out[3] = chip->fsm_cnt == 20; // 16
            chip->fsm_out[4] = chip->fsm_l3[1]; // 17
            chip->fsm_out[5] = chip->fsm_cnt == 52; // 34
            chip->fsm_out[6] = chip->fsm_l4[1]; // 35
            chip->fsm_out[7] = (chip->fsm_l5[1] & 4) != 0 || ((chip->fsm_cnt & 56) == 0); // 0-8
            chip->fsm_out[8] = (chip->fsm_cnt & 32) == 0;
            chip->fsm_out[9] = (chip->fsm_l6[1] & 2) != 0;
            chip->fsm_out[10] = (chip->fsm_l7[1] & 2) != 0;
            chip->fsm_out[11] = chip->rhythm && (chip->fsm_l8[1] & 2) != 0; // r 14, 15, 16, 17, 18, 19

            int fsm_mc = !((chip->fsm_cnt & 5) == 4 || (chip->fsm_cnt & 2) != 0);
            int fsm_mc_4op = fsm_mc && !fsm_4op;
            int rhy_19_20 = chip->rhythm && (chip->fsm_cnt == 19 || chip->fsm_cnt == 20);

            chip->fsm_out[12] = fsm_mc_4op && !(chip->rhythm && (chip->fsm_cnt == 16 || chip->fsm_cnt == 17)); // feedback
            chip->fsm_out[14] = con_4op || (!fsm_4op && !(chip->fsm_l9[1] & 4) && (chip->connect_l[1] & 2) != 0); // connect
            chip->fsm_out[13] = !(chip->rhythm && chip->fsm_cnt == 18) && (fsm_mc_4op || rhy_19_20 || chip->fsm_out[14]); // output
            chip->fsm_out[15] = !fsm_mc && !rhy_19_20; // load fb
            chip->fsm_out[16] = !fsm_mc_4op && !rhy_19_20; // modulate
        }
    }

    if (chip->clk1)
        chip->timer_st_load_l = chip->fsm_out[6];
    chip->timer_st_load = chip->fsm_out[6] && !chip->timer_st_load_l;

    if (chip->timer_st_load)
    {
        chip->t1_start = chip->reg_t1_start;
        chip->t2_start = chip->reg_t2_start;
    }

    if (chip->clk1)
    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[6];
        int reset = (chip->reg_test0 & 2) != 0 || chip->reset1;

        chip->lfo_cnt[0] = reset ? 0 : (lfo + add) & 1023;
        chip->vib_cnt[0] = reset ? 0 : (chip->vib_cnt[1] + chip->vib_step) & 7;
    }
    if (chip->clk2)
    {
        chip->lfo_cnt[1] = chip->lfo_cnt[0];
        chip->vib_cnt[1] = chip->vib_cnt[0];
    }

    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[6];

        chip->t1_step = (((lfo & 3) + add) & 4) != 0;
        chip->t2_step = (((lfo & 15) + add) & 16) != 0;
        chip->am_step = (((lfo & 63) + add) & 64) != 0;
        chip->vib_step = (((lfo & 1023) + add) & 1024) != 0;
        chip->vib_step |= (chip->reg_test0 & 16) != 0 && add;
    }

    if (chip->clk1)
    {
        int value = (chip->t1_of[1] || (chip->t1_start_l[1] & 3) == 1) ? chip->reg_timer1 : chip->t1_cnt[1];
        value += ((chip->t1_start_l[1] & 1) != 0 && chip->t1_step) || (chip->reg_test1 & 8) != 0;
        chip->t1_of[0] = (value & 256) != 0;
        chip->t1_cnt[0] = (chip->t1_start_l[1] & 1) == 0 ? 0 : (value & 255);
        
        value = (chip->t2_of[1] || (chip->t2_start_l[1] & 3) == 1) ? chip->reg_timer2 : chip->t2_cnt[1];
        value += ((chip->t2_start_l[1] & 1) != 0 && chip->t2_step) || (chip->reg_test1 & 8) != 0;
        chip->t2_of[0] = (value & 256) != 0;
        chip->t2_cnt[0] = (chip->t2_start_l[1] & 1) == 0 ? 0 : (value & 255);

        chip->t1_start_l[0] = (chip->t1_start_l[1] << 1) | chip->t1_start;
        chip->t2_start_l[0] = (chip->t2_start_l[1] << 1) | chip->t2_start;
    }
    if (chip->clk2)
    {
        chip->t1_cnt[1] = chip->t1_cnt[0];
        chip->t1_of[1] = chip->t1_of[0];
        chip->t2_cnt[1] = chip->t2_cnt[0];
        chip->t2_of[1] = chip->t2_of[0];

        chip->t1_start_l[1] = chip->t1_start_l[0];
        chip->t2_start_l[1] = chip->t2_start_l[0];
    }

    if (reg_sel4_rst || chip->reg_t1_mask)
        chip->t1_status = 0;
    else if (chip->t1_of[1])
        chip->t1_status = 1;

    if (reg_sel4_rst || chip->reg_t2_mask)
        chip->t2_status = 0;
    else if (chip->t2_of[1])
        chip->t2_status = 1;

    chip->rh_sel0 = chip->rhythm && chip->fsm_out[1];

    if (chip->clk1)
    {
        chip->rh_sel[0] = (chip->rh_sel[1] << 1) | chip->rh_sel0;
    }
    if (chip->clk2)
    {
        chip->rh_sel[1] = chip->rh_sel[0];
    }
    
    chip->keyon_comb = chip->keyon[1]
        || (chip->rh_sel0 && (chip->reg_rh_kon & 16) != 0) // bd0
        || ((chip->rh_sel[1] & 1) != 0 && (chip->reg_rh_kon & 1) != 0) // hh
        || ((chip->rh_sel[1] & 2) != 0 && (chip->reg_rh_kon & 4) != 0) // tom
        || ((chip->rh_sel[1] & 4) != 0 && (chip->reg_rh_kon & 16) != 0) // bd1
        || ((chip->rh_sel[1] & 8) != 0 && (chip->reg_rh_kon & 8) != 0) // sd
        || ((chip->rh_sel[1] & 16) != 0 && (chip->reg_rh_kon & 2) != 0); // tc

    if (chip->clk1)
    {
        static const int pg_multi[16] = {
            1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
        };
        int fnum = chip->fnum[1];
        int freq;
        int pg_add;
        int vib_sel1 = (chip->vib_cnt[1] & 3) == 2;
        int vib_sel2 = (chip->vib_cnt[1] & 1) == 1;
        int vib_sh0 = chip->reg_dv && chip->vib[1] && vib_sel1;
        int vib_sh1 = (chip->reg_dv && chip->vib[1] && vib_sel2)
            || (!chip->reg_dv && chip->vib[1] && vib_sel1);
        int vib_sh2 = !chip->reg_dv && chip->vib[1] && vib_sel2;
        int vib_sign = (chip->vib_cnt[1] & 4) != 0 && chip->vib[1];
        int vib_add = 0;
        int phase;
        if (vib_sh0)
            vib_add |= (chip->fnum[1] >> 7) & 7;
        if (vib_sh1)
            vib_add |= (chip->fnum[1] >> 8) & 3;
        if (vib_sh2)
            vib_add |= (chip->fnum[1] >> 9) & 1;
        if (vib_sign)
        {
            vib_add ^= 1023;
        }
        fnum += vib_add;
        fnum += vib_sign;
        if (vib_sign)
            fnum &= 1023;

        freq = (fnum << chip->block[1]) >> 1;

        pg_add = (freq * pg_multi[chip->multi]) >> 1;

        phase = ((chip->eg_dokon || (chip->reg_test0 & 4) != 0 || chip->reset1) ? 0 : chip->pg_phase_o[3]) + pg_add;

        if (chip->fsm_out[4] || chip->fsm_out[6])
            chip->pg_index[0] = 0;
        else
            chip->pg_index[0] = chip->pg_index[1] + 1;

        if (chip->pg_index[1] < 18)
        {
            int index1 = chip->pg_index[1];
            int index2 = (index1 + 17) % 18;
            chip->pg_cells[index2] = phase;
            chip->pg_cells[index2 + 18] = chip->pg_cells[index1];
            chip->pg_phase_o[0] = chip->pg_cells[18 + index1];
        }
        chip->pg_phase_o[2] = chip->pg_phase_o[1];
    }
    if (chip->clk2)
    {
        chip->pg_index[1] = chip->pg_index[0];
        chip->pg_phase_o[1] = chip->pg_phase_o[0];
        chip->pg_phase_o[3] = chip->pg_phase_o[2];
    }

    if (chip->rclk1)
    {
        int noise_bit;

        noise_bit = ((chip->noise_lfsr[1] >> 22) ^ (chip->noise_lfsr[1] >> 8)) & 1;

        if ((chip->noise_lfsr[1] & 0x7fffff) == 0)
            noise_bit |= 1;

        noise_bit |= (chip->reg_test0 & 2) != 0;

        if (chip->reset1)
            noise_bit = 0;

        chip->noise_lfsr[0] = (chip->noise_lfsr[1] << 1) | noise_bit;
    }
    if (chip->rclk2)
    {
        chip->noise_lfsr[1] = chip->noise_lfsr[0];
    }

    {
        int pg_out = chip->pg_phase_o[3] >> 9;
        int hh = chip->fsm_out[2] && chip->rhythm;
        int sd = chip->fsm_out[3] && chip->rhythm;
        int tc = chip->fsm_out[4] && chip->rhythm;
        int rhy = (chip->fsm_out[2] || chip->fsm_out[3] || chip->fsm_out[4]) && chip->rhythm;
        if (chip->clk1)
            chip->hh_load = chip->fsm_out[2];
        if (!chip->hh_load && chip->fsm_out[2])
        {
            chip->hh_bit2 = (pg_out >> 2) & 1;
            chip->hh_bit3 = (pg_out >> 3) & 1;
            chip->hh_bit7 = (pg_out >> 7) & 1;
            chip->hh_bit8 = (pg_out >> 8) & 1;
        }
        if (chip->clk1)
            chip->tc_load = tc;
        if (!chip->tc_load && tc)
        {
            chip->tc_bit3 = (pg_out >> 3) & 1;
            chip->tc_bit5 = (pg_out >> 5) & 1;
        }

        if (chip->clk1) // opt
        {
            int rm_bit;
            int noise = (chip->noise_lfsr[1] >> 22) & 1;

            rm_bit = (chip->hh_bit2 ^ chip->hh_bit7)
                | (chip->tc_bit5 ^ chip->hh_bit3)
                | (chip->tc_bit5 ^ chip->tc_bit3);

            chip->pg_out_rhy = 0;
            if (!rhy)
                chip->pg_out_rhy |= chip->pg_out;
            if (hh)
            {
                chip->pg_out_rhy |= rm_bit << 9;
                if (noise ^ rm_bit)
                    chip->pg_out_rhy |= 0xd0;
                else
                    chip->pg_out_rhy |= 0x34;
            }
            if (sd)
                chip->pg_out_rhy |= (chip->hh_bit8 << 9) | ((noise ^ chip->hh_bit8) << 8);
            if (tc)
                chip->pg_out_rhy |= (rm_bit << 9) | 0x80;
        }

        if (chip->clk1)
        {
            chip->pg_dbg[0] = chip->pg_dbg[1] >> 1;

            chip->pg_dbg_load_l[0] = (chip->reg_test0 & 8) != 0;

            if ((chip->reg_test0 & 8) != 0 && !chip->pg_dbg_load_l[1])
            {
                chip->pg_dbg[0] |= chip->pg_phase_o[3] & 0x1ff;
                chip->pg_dbg[0] |= (chip->pg_out_rhy & 0x3ff) << 9;
            }
        }
        if (chip->clk2)
        {
            chip->pg_dbg_load_l[1] = chip->pg_dbg_load_l[0];

            chip->pg_dbg[1] = chip->pg_dbg[0];
        }
    }

end:

    if (chip->mclk1)
    {
        chip->prescaler1_reset[0] = (chip->prescaler1_reset[1] << 1) | chip->reset1;

        if (!(chip->prescaler1_reset[1] & 2) && chip->reset1)
            chip->prescaler1_cnt[0] = 0;
        else
            chip->prescaler1_cnt[0] = (chip->prescaler1_cnt[1] + 1) & 3;
    }

    if (chip->aclk1)
    {
        int prescaler2_reset = !(chip->prescaler2_reset_l[1] & 2) && chip->reset1;
        chip->prescaler2_reset_l[0] = (chip->prescaler2_reset_l[1] << 1) | chip->reset1;

        if (prescaler2_reset)
            chip->prescaler2_cnt[0] = 0;
        else
            chip->prescaler2_cnt[0] = (chip->prescaler2_cnt[1] + 1) & 3;

        chip->prescaler2_l1[0] = !prescaler2_reset && (chip->prescaler2_cnt[1] & 1) == 0;
        chip->prescaler2_l2 = chip->prescaler2_l1[1];

        chip->prescaler2_l3[0] = !prescaler2_reset && (chip->prescaler2_cnt[1] & 1) != 0;
        chip->prescaler2_l4 = chip->prescaler2_l3[1];

        chip->prescaler2_l5[0] = !prescaler2_reset && chip->prescaler2_cnt[1] == 3;

        chip->prescaler2_l6[0] = !prescaler2_reset && chip->prescaler2_cnt[1] == 1;

        chip->prescaler2_l7 = (chip->prescaler2_cnt[1] & 1) == 0;
    }
}
