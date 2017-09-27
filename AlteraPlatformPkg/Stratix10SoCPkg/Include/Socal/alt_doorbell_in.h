/***********************************************************************************
*                                                                                  *
* Copyright 2013-2015 Altera Corporation. All Rights Reserved.                     *
*                                                                                  *
* Redistribution and use in source and binary forms, with or without               *
* modification, are permitted provided that the following conditions are met:      *
*                                                                                  *
* 1. Redistributions of source code must retain the above copyright notice,        *
*    this list of conditions and the following disclaimer.                         *
*                                                                                  *
* 2. Redistributions in binary form must reproduce the above copyright notice,     *
*    this list of conditions and the following disclaimer in the documentation     *
*    and/or other materials provided with the distribution.                        *
*                                                                                  *
* 3. Neither the name of the copyright holder nor the names of its contributors    *
*    may be used to endorse or promote products derived from this software without *
*    specific prior written permission.                                            *
*                                                                                  *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"      *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE        *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE       *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE        *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR              *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF             *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS         *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN          *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)          *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                      *
*                                                                                  *
***********************************************************************************/

/* Altera - ALT_DOORBELL_IN */

#ifndef __ALT_SOCAL_DOORBELL_IN_H__
#define __ALT_SOCAL_DOORBELL_IN_H__

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else   /* __cplusplus */
//#include <stdint.h>
#endif  /* __cplusplus */
#endif  /* __ASSEMBLY__ */

/*
 * Component : DOORBELL_IN
 * Door Bell registers to allow RW by both SDM and external masters
 * 
 */
/*
 * Register : reg - em2sdm
 * 
 * External master to SDM doorbell register
 * 
 * Register Layout
 * 
 *  Bits   | Access | Reset | Description                    
 * :-------|:-------|:------|:--------------------------------
 *  [0]    | RW     | 0x0   | ALT_DOORBELL_IN_EM2SDM_DOORBELL
 *  [31:1] | ???    | 0x0   | *UNDEFINED*                    
 * 
 */
/*
 * Field : doorbell
 * 
 * external master to sdm door bell register.
 * 
 * Field Access Macros:
 * 
 */
/* The Least Significant Bit (LSB) position of the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_LSB        0
/* The Most Significant Bit (MSB) position of the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_MSB        0
/* The width in bits of the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_WIDTH      1
/* The mask used to set the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field value. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_SET_MSK    0x00000001
/* The mask used to clear the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field value. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_CLR_MSK    0xfffffffe
/* The reset value of the ALT_DOORBELL_IN_EM2SDM_DOORBELL register field. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_RESET      0x0
/* Extracts the ALT_DOORBELL_IN_EM2SDM_DOORBELL field value from a register. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_GET(value) (((value) & 0x00000001) >> 0)
/* Produces a ALT_DOORBELL_IN_EM2SDM_DOORBELL register field value suitable for setting the register. */
#define ALT_DOORBELL_IN_EM2SDM_DOORBELL_SET(value) (((value) << 0) & 0x00000001)

#ifndef __ASSEMBLY__
/*
 * WARNING: The C register and register group struct declarations are provided for
 * convenience and illustrative purposes. They should, however, be used with
 * caution as the C language standard provides no guarantees about the alignment or
 * atomicity of device memory accesses. The recommended practice for coding device
 * drivers is to use the SoCAL access macros in conjunction with alt_read_word()
 * and alt_write_word() functions for 32 bit registers and alt_read_dword() and
 * alt_write_dword() functions for 64 bit registers.
 * 
 * The struct declaration for register ALT_DOORBELL_IN_EM2SDM.
 */
struct ALT_DOORBELL_IN_EM2SDM_s
{
    volatile uint32_t  doorbell :  1;  /* ALT_DOORBELL_IN_EM2SDM_DOORBELL */
    uint32_t                    : 31;  /* *UNDEFINED* */
};

/* The typedef declaration for register ALT_DOORBELL_IN_EM2SDM. */
typedef struct ALT_DOORBELL_IN_EM2SDM_s  ALT_DOORBELL_IN_EM2SDM_t;
#endif  /* __ASSEMBLY__ */

/* The reset value of the ALT_DOORBELL_IN_EM2SDM register. */
#define ALT_DOORBELL_IN_EM2SDM_RESET       0x00000000
/* The byte offset of the ALT_DOORBELL_IN_EM2SDM register from the beginning of the component. */
#define ALT_DOORBELL_IN_EM2SDM_OFST        0x0

#ifndef __ASSEMBLY__
/*
 * WARNING: The C register and register group struct declarations are provided for
 * convenience and illustrative purposes. They should, however, be used with
 * caution as the C language standard provides no guarantees about the alignment or
 * atomicity of device memory accesses. The recommended practice for coding device
 * drivers is to use the SoCAL access macros in conjunction with alt_read_word()
 * and alt_write_word() functions for 32 bit registers and alt_read_dword() and
 * alt_write_dword() functions for 64 bit registers.
 * 
 * The struct declaration for register group ALT_DOORBELL_IN.
 */
struct ALT_DOORBELL_IN_s
{
    volatile ALT_DOORBELL_IN_EM2SDM_t  em2sdm;           /* ALT_DOORBELL_IN_EM2SDM */
    volatile uint32_t                  _pad_0x4_0xc[2];  /* *UNDEFINED* */
};

/* The typedef declaration for register group ALT_DOORBELL_IN. */
typedef struct ALT_DOORBELL_IN_s  ALT_DOORBELL_IN_t;
/* The struct declaration for the raw register contents of register group ALT_DOORBELL_IN. */
struct ALT_DOORBELL_IN_raw_s
{
    volatile uint32_t  em2sdm;           /* ALT_DOORBELL_IN_EM2SDM */
    volatile uint32_t  _pad_0x4_0xc[2];  /* *UNDEFINED* */
};

/* The typedef declaration for the raw register contents of register group ALT_DOORBELL_IN. */
typedef struct ALT_DOORBELL_IN_raw_s  ALT_DOORBELL_IN_raw_t;
#endif  /* __ASSEMBLY__ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __ALT_SOCAL_DOORBELL_IN_H__ */

