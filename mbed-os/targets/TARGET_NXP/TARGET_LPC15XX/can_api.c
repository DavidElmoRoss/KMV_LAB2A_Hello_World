/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "can_api.h"

#include "cmsis.h"
#include "mbed_error.h"

#include <math.h>
#include <string.h>

/* Handy defines */
#define RX_MSG_OBJ_COUNT 31
#define TX_MSG_OBJ_COUNT 1
#define DLC_MAX          8

#define ID_STD_MASK      0x07FF
#define ID_EXT_MASK      0x1FFFFFFF
#define DLC_MASK         0x0F

#define CANIFn_ARB2_DIR         (1UL << 13)
#define CANIFn_ARB2_XTD         (1UL << 14)
#define CANIFn_ARB2_MSGVAL      (1UL << 15)
#define CANIFn_MSK2_MXTD        (1UL << 15)
#define CANIFn_MSK2_MDIR        (1UL << 14)
#define CANIFn_MCTRL_EOB        (1UL << 7)
#define CANIFn_MCTRL_TXRQST     (1UL << 8)
#define CANIFn_MCTRL_RMTEN      (1UL << 9)
#define CANIFn_MCTRL_RXIE       (1UL << 10)
#define CANIFn_MCTRL_TXIE       (1UL << 11)
#define CANIFn_MCTRL_UMASK      (1UL << 12)
#define CANIFn_MCTRL_INTPND     (1UL << 13)
#define CANIFn_MCTRL_MSGLST     (1UL << 14)
#define CANIFn_MCTRL_NEWDAT     (1UL << 15)
#define CANIFn_CMDMSK_DATA_B    (1UL << 0)
#define CANIFn_CMDMSK_DATA_A    (1UL << 1)
#define CANIFn_CMDMSK_TXRQST    (1UL << 2)
#define CANIFn_CMDMSK_NEWDAT    (1UL << 2)
#define CANIFn_CMDMSK_CLRINTPND (1UL << 3)
#define CANIFn_CMDMSK_CTRL      (1UL << 4)
#define CANIFn_CMDMSK_ARB       (1UL << 5)
#define CANIFn_CMDMSK_MASK      (1UL << 6)
#define CANIFn_CMDMSK_WR        (1UL << 7)
#define CANIFn_CMDMSK_RD        (0UL << 7)
#define CANIFn_CMDREQ_BUSY      (1UL << 15)

#define CANSTAT_TXOK                   (1 << 3)           // Transmitted a message successfully This bit must be reset by the CPU. It is never reset by the CAN controller.
#define CANSTAT_RXOK                   (1 << 4)           // Received a message successfully This bit must be reset by the CPU. It is never reset by the CAN controller.
#define CANSTAT_EPASS                  (1 << 5)           // Error passive
#define CANSTAT_EWARN                  (1 << 6)           // Warning status
#define CANSTAT_BOFF                   (1 << 7)           // Busoff status

#define CANCNTL_INIT                   (1 << 0)           // Initialization
#define CANCNTL_IE                     (1 << 1)           // Module interrupt enable
#define CANCNTL_SIE                    (1 << 2)           // Status change interrupt enable
#define CANCNTL_EIE                    (1 << 3)           // Error interrupt enable
#define CANCNTL_DAR                    (1 << 5)           // Disable automatic retransmission
#define CANCNTL_CCE                    (1 << 6)           // Configuration change enable
#define CANCNTL_TEST                   (1 << 7)           // Test mode enable

#define CANTEST_BASIC                  (1 << 2)           // Basic mode
#define CANTEST_SILENT                 (1 << 3)           // Silent mode
#define CANTEST_LBACK                  (1 << 4)           // Loop back mode
#define CANTEST_TX_MASK                0x0060             // Control of CAN_TXD pins
#define CANTEST_TX_SHIFT               5                 
#define CANTEST_RX                     (1 << 7)           // Monitors the actual value of the CAN_RXD pin.

static uint32_t can_irq_id = 0;
static can_irq_handler irq_handler;

#define IRQ_ENABLE_TX (1 << 0)
#define IRQ_ENABLE_RX (1 << 1)
#define IRQ_ENABLE_EW (1 << 2)
#define IRQ_ENABLE_EP (1 << 3)
#define IRQ_ENABLE_BE (1 << 4)
#define IRQ_ENABLE_STATUS (IRQ_ENABLE_TX | IRQ_ENABLE_RX)
#define IRQ_ENABLE_ERROR (IRQ_ENABLE_EW | IRQ_ENABLE_EP | IRQ_ENABLE_BE)
#define IRQ_ENABLE_ANY (IRQ_ENABLE_STATUS | IRQ_ENABLE_ERROR)
static uint32_t enabled_irqs = 0;

// Pinmap used for testing only
static const PinMap PinMap_CAN_testing[] = {
    {P0_0,  0, 0},
    {P0_1,  0, 0},
    {P0_2,  0, 0},
    {P0_3,  0, 0},
    {P0_4,  0, 0},
    {P0_5,  0, 0},
    {P0_6,  0, 0},
    {P0_7,  0, 0},
    {P0_8,  0, 0},
    {P0_9,  0, 0},
    {P0_10, 0, 0},
    {P0_11, 0, 0},
    {P0_12, 0, 0},
    {P0_13, 0, 0},
    {P0_14, 0, 0},
    {P0_15, 0, 0},
    {P0_16, 0, 0},
    {P0_17, 0, 0},
    {P0_18, 0, 0},
    {P0_19, 0, 0},
    {P0_20, 0, 0},
    {P0_21, 0, 0},
    {P0_22, 0, 0},
    {P0_23, 0, 0},
    {P0_24, 0, 0},
    {P0_25, 0, 0},
    {P0_26, 0, 0},
    {P0_27, 0, 0},
    {P0_28, 0, 0},
    {P0_29, 0, 0},
    {P0_30, 0, 0},
    {P0_31, 0, 0},
    {P1_0,  0, 0},
    {P1_1,  0, 0},
    {P1_2,  0, 0},
    {P1_3,  0, 0},
    {P1_4,  0, 0},
    {P1_5,  0, 0},
    {P1_6,  0, 0},
    {P1_7,  0, 0},
    {P1_8,  0, 0},
    {P1_9,  0, 0},
    {P1_10, 0, 0},
    {P1_11, 0, 0},
    {P1_12, 0, 0},
    {P1_13, 0, 0},
    {P1_14, 0, 0},
    {P1_15, 0, 0},
    {P1_16, 0, 0},
    {P1_17, 0, 0},
    {P1_18, 0, 0},
    {P1_19, 0, 0},
    {P1_20, 0, 0},
    {P1_21, 0, 0},
    {P1_22, 0, 0},
    {P1_23, 0, 0},
    {P1_24, 0, 0},
    {P1_25, 0, 0},
    {P1_26, 0, 0},
    {P1_27, 0, 0},
    {P1_28, 0, 0},
    {P1_29, 0, 0},
    {P1_30, 0, 0},
    {P1_31, 0, 0},
    {P2_0,  0, 0},
    {P2_1,  0, 0},
    {P2_2,  0, 0},
    {P2_3,  0, 0},
    {P2_4,  0, 0},
    {P2_5,  0, 0},
    {P2_6,  0, 0},
    {P2_7,  0, 0},
    {P2_8,  0, 0},
    {P2_9,  0, 0},
    {P2_10, 0, 0},
    {P2_11, 0, 0},
    {P2_12, 0, 0},

    {NC, NC, 0}
};

static inline void can_disable(can_t *obj) {
    LPC_C_CAN0->CANCNTL |= 0x1;
}

static inline void can_enable(can_t *obj) {
    if (LPC_C_CAN0->CANCNTL & 0x1) {
        LPC_C_CAN0->CANCNTL &= ~(0x1);
    }
}

int can_mode(can_t *obj, CanMode mode) {
    int success = 0;
    switch (mode) {
        case MODE_RESET:
            LPC_C_CAN0->CANCNTL &=~CANCNTL_TEST;
            can_disable(obj);
            success = 1;
            break;
        case MODE_NORMAL:
            LPC_C_CAN0->CANCNTL &=~CANCNTL_TEST;
            can_enable(obj);
            success = 1;
            break;
        case MODE_SILENT:
            LPC_C_CAN0->CANCNTL |= CANCNTL_TEST;
            LPC_C_CAN0->CANTEST |= CANTEST_SILENT;
            LPC_C_CAN0->CANTEST &=~ CANTEST_LBACK;
            success = 1;
            break;
        case MODE_TEST_LOCAL:
            LPC_C_CAN0->CANCNTL |= CANCNTL_TEST;
            LPC_C_CAN0->CANTEST &=~CANTEST_SILENT;
            LPC_C_CAN0->CANTEST |= CANTEST_LBACK;
            success = 1;
            break;
        case MODE_TEST_SILENT:
            LPC_C_CAN0->CANCNTL |= CANCNTL_TEST;
            LPC_C_CAN0->CANTEST |= (CANTEST_LBACK | CANTEST_SILENT);
            success = 1;
            break;
        case MODE_TEST_GLOBAL:
        default:
            success = 0;
            break;
    }
    
    return success;
}

int can_filter(can_t *obj, uint32_t id, uint32_t mask, CANFormat format, int32_t handle) {
    uint16_t i;

    // Find first free message object
    if (handle == 0) {
        uint32_t msgval = LPC_C_CAN0->CANMSGV1 | (LPC_C_CAN0->CANMSGV2 << 16);

        // Find first free messagebox
        for (i = 0; i < 32; i++) {
            if ((msgval & (1 << i)) == 0) {
                handle = i+1;
                break;
            }
        }
    }

    if (handle > 0 && handle <= 32) {
        if (format == CANExtended) {
            // Mark message valid, Direction = TX, Extended Frame, Set Identifier and mask everything
            LPC_C_CAN0->CANIF1_ARB1 = (id & 0xFFFF);
            LPC_C_CAN0->CANIF1_ARB2 = CANIFn_ARB2_MSGVAL | CANIFn_ARB2_XTD | ((id >> 16) & 0x1FFF);
            LPC_C_CAN0->CANIF1_MSK1 = (mask & 0xFFFF);
            LPC_C_CAN0->CANIF1_MSK2 = CANIFn_MSK2_MXTD /*| CANIFn_MSK2_MDIR*/ | ((mask >> 16) & 0x1FFF);
        } else {
            // Mark message valid, Direction = TX, Set Identifier and mask everything
            LPC_C_CAN0->CANIF1_ARB2 = CANIFn_ARB2_MSGVAL | ((id << 2) & 0x1FFF);
            LPC_C_CAN0->CANIF1_MSK2 = /*CANIFn_MSK2_MDIR |*/ ((mask << 2) & 0x1FFF);
        }

        // Use mask, single message object and set DLC
        LPC_C_CAN0->CANIF1_MCTRL = CANIFn_MCTRL_UMASK | CANIFn_MCTRL_EOB | (DLC_MAX & 0xF);

        // Transfer all fields to message object
        LPC_C_CAN0->CANIF1_CMDMSK_W = CANIFn_CMDMSK_WR | CANIFn_CMDMSK_MASK | CANIFn_CMDMSK_ARB | CANIFn_CMDMSK_CTRL;

        // Start Transfer to given message number
        LPC_C_CAN0->CANIF1_CMDREQ = (handle & 0x3F);

        // Wait until transfer to message ram complete - TODO: maybe not block??
        while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );
    }

    return handle;
}

static inline void can_irq() {
    uint32_t intid = LPC_C_CAN0->CANINT & 0xFFFF;

    if (intid == 0x8000) {
        uint32_t status = LPC_C_CAN0->CANSTAT;
        // Note that since it's impossible to tell which specific status caused
        // the interrupt to fire, this just fires them all.
        // In particular, EWARN is not mutually exclusive with the others and
        // may fire multiple times with other status transitions, including
        // transmit and receive completion (if enabled). Ignoring EWARN with a
        // priority system (i.e. blocking EWARN interrupts if EPASS or BOFF is
        // set) may discard some EWARN interrupts.
        if (status & CANSTAT_BOFF) {
            if (enabled_irqs & IRQ_ENABLE_BE) {
                irq_handler(can_irq_id, IRQ_BUS);
            }
        }
        if (status & CANSTAT_EPASS) {
            if (enabled_irqs & IRQ_ENABLE_EP) {
                irq_handler(can_irq_id, IRQ_PASSIVE);
            }
        }
        if (status & CANSTAT_EWARN) {
            if (enabled_irqs & IRQ_ENABLE_EW) {
                irq_handler(can_irq_id, IRQ_ERROR);
            }
        }
        if ((status & CANSTAT_RXOK) != 0) {
            LPC_C_CAN0->CANSTAT &= ~CANSTAT_RXOK;
            irq_handler(can_irq_id, IRQ_RX);
        }
        if ((status & CANSTAT_TXOK) != 0) {
            LPC_C_CAN0->CANSTAT &= ~CANSTAT_TXOK;
            irq_handler(can_irq_id, IRQ_TX);
        }
    }
}

// Register CAN object's irq handler
void can_irq_init(can_t *obj, can_irq_handler handler, uint32_t id) {
    irq_handler = handler;
    can_irq_id = id;
}

// Unregister CAN object's irq handler
void can_irq_free(can_t *obj) {
    LPC_C_CAN0->CANCNTL &= ~(1UL << 1); // Disable Interrupts :)
    can_irq_id = 0;
    NVIC_DisableIRQ(C_CAN0_IRQn);
}

// Clear or set a irq
void can_irq_set(can_t *obj, CanIrqType type, uint32_t enable) {
    uint32_t mask_enable;
    switch (type) {
       case IRQ_RX:
           mask_enable = IRQ_ENABLE_RX;
           break;
       case IRQ_TX:
           mask_enable = IRQ_ENABLE_TX;
           break;
       case IRQ_BUS:
           mask_enable = IRQ_ENABLE_BE;
           break;
       case IRQ_PASSIVE:
           mask_enable = IRQ_ENABLE_EP;
           break;
       case IRQ_ERROR:
           mask_enable = IRQ_ENABLE_EW;
           break;
       default:
           return;
    }
    
    if (enable) {
        enabled_irqs = enabled_irqs | mask_enable;
    } else {
        enabled_irqs = enabled_irqs & ~mask_enable;
    }

    // Put CAN in Reset Mode and enable interrupt
    can_disable(obj);
    if (!(enabled_irqs & IRQ_ENABLE_ANY)) {
        LPC_C_CAN0->CANCNTL &= ~(1UL << 1 | 1UL << 2 | 1UL << 3);
    } else {
        LPC_C_CAN0->CANCNTL |= 1UL << 1;
        // Use status interrupts instead of message interrupts to avoid
        // stomping over potential filter configurations.
        if (enabled_irqs & IRQ_ENABLE_STATUS) {
            LPC_C_CAN0->CANCNTL |= 1UL << 2;
        } else {
            LPC_C_CAN0->CANCNTL &= ~(1UL << 2);
        }
        if (enabled_irqs & IRQ_ENABLE_ERROR) {
            LPC_C_CAN0->CANCNTL |= 1UL << 3;
        } else {
            LPC_C_CAN0->CANCNTL &= ~(1UL << 3);
        } 
    }
    
    // Take it out of reset...
    can_enable(obj);

    // Enable NVIC if at least 1 interrupt is active
    NVIC_SetVector(C_CAN0_IRQn, (uint32_t) &can_irq);
    NVIC_EnableIRQ(C_CAN0_IRQn);
}

// This table has the sampling points as close to 75% as possible. The first
// value is TSEG1, the second TSEG2.
static const int timing_pts[23][2] = {
    {0x0, 0x0},      // 2,  50%
    {0x1, 0x0},      // 3,  67%
    {0x2, 0x0},      // 4,  75%
    {0x3, 0x0},      // 5,  80%
    {0x3, 0x1},      // 6,  67%
    {0x4, 0x1},      // 7,  71%
    {0x5, 0x1},      // 8,  75%
    {0x6, 0x1},      // 9,  78%
    {0x6, 0x2},      // 10, 70%
    {0x7, 0x2},      // 11, 73%
    {0x8, 0x2},      // 12, 75%
    {0x9, 0x2},      // 13, 77%
    {0x9, 0x3},      // 14, 71%
    {0xA, 0x3},      // 15, 73%
    {0xB, 0x3},      // 16, 75%
    {0xC, 0x3},      // 17, 76%
    {0xD, 0x3},      // 18, 78%
    {0xD, 0x4},      // 19, 74%
    {0xE, 0x4},      // 20, 75%
    {0xF, 0x4},      // 21, 76%
    {0xF, 0x5},      // 22, 73%
    {0xF, 0x6},      // 23, 70%
    {0xF, 0x7},      // 24, 67%
};

static unsigned int can_speed(unsigned int sclk, unsigned int cclk, unsigned char psjw) {
    uint32_t    btr;
    uint32_t    clkdiv = 1;
    uint16_t    brp = 0;
    uint32_t    calcbit;
    uint32_t    bitwidth;
    int         hit = 0;
    int         bits = 0;

    bitwidth = sclk / cclk;

    brp = bitwidth / 0x18;
    while ((!hit) && (brp < bitwidth / 4)) {
        brp++;
        for (bits = 22; bits > 0; bits--) {
            calcbit = (bits + 3) * (brp + 1);
            if (calcbit == bitwidth) {
                hit = 1;
                break;
            }
        }
    }

    clkdiv = clkdiv - 1;

    if (hit) {
        btr = (timing_pts[bits][1] & 0x7) << 12
            | (timing_pts[bits][0] & 0xf) << 8
            | (psjw & 0x3) << 6
            | (brp & 0x3F);
        btr = btr | (clkdiv << 16);
    } else {
        btr = 0;
    }

    return btr;
}


int can_config_rxmsgobj(can_t *obj) {
    uint16_t i = 0;

    // Make sure the interface is available
    while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );

    // Mark message valid, Direction = RX, Don't care about anything else
    LPC_C_CAN0->CANIF1_ARB1 = 0;
    LPC_C_CAN0->CANIF1_ARB2 = 0;
    LPC_C_CAN0->CANIF1_MCTRL = 0;

    for ( i = 1; i <= RX_MSG_OBJ_COUNT; i++ ) {
        // Transfer arb and control fields to message object
        LPC_C_CAN0->CANIF1_CMDMSK_W = CANIFn_CMDMSK_WR | CANIFn_CMDMSK_ARB | CANIFn_CMDMSK_CTRL;

        // Start Transfer to given message number
        LPC_C_CAN0->CANIF1_CMDREQ = (i & 0x3F);

        // Wait until transfer to message ram complete - TODO: maybe not block??
        while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );
    }

    // Accept all messages
    can_filter(obj, 0, 0, CANStandard, 1);

    return 1;
}

int can_config_txmsgobj(can_t *obj) {
    uint16_t i = 0;

    // Make sure the interface is available
    while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );

    // Mark message valid, Direction = TX, Don't care about anything else
    LPC_C_CAN0->CANIF1_ARB1 = 0;
    LPC_C_CAN0->CANIF1_ARB2 = CANIFn_ARB2_DIR;
    LPC_C_CAN0->CANIF1_MCTRL = 0;

    for ( i = RX_MSG_OBJ_COUNT + 1; i <= (TX_MSG_OBJ_COUNT + RX_MSG_OBJ_COUNT); i++ )
    {
        // Transfer arb and control fields to message object
        LPC_C_CAN0->CANIF1_CMDMSK_W = CANIFn_CMDMSK_WR | CANIFn_CMDMSK_ARB | CANIFn_CMDMSK_CTRL;
        // In a union with CANIF1_CMDMSK_R

        // Start Transfer to given message number
        LPC_C_CAN0->CANIF1_CMDREQ = i & 0x3F;

        // Wait until transfer to message ram complete - TODO: maybe not block??
        while( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );
    }

    return 1;
}

void can_init_freq(can_t *obj, PinName rd, PinName td, int hz) {
    // Enable power and clock
    LPC_SYSCON->SYSAHBCLKCTRL1 |= (1UL << 7);
    LPC_SYSCON->PRESETCTRL1    |= (1UL << 7);
    LPC_SYSCON->PRESETCTRL1    &= ~(1UL << 7);

    // Enable Initialization mode
    if (!(LPC_C_CAN0->CANCNTL & (1UL << 0))) {
        LPC_C_CAN0->CANCNTL |= (1UL << 0);
    }

    LPC_SWM->PINASSIGN[6] &= ~(0x00FFFF00L);
    LPC_SWM->PINASSIGN[6] |= (rd << 16) | (td << 8);

    can_frequency(obj, hz);

    // Resume operation
    LPC_C_CAN0->CANCNTL &= ~(1UL << 0);
    while ( LPC_C_CAN0->CANCNTL & (1UL << 0) );

    // Initialize RX message object
    can_config_rxmsgobj(obj);
    // Initialize TX message object
    can_config_txmsgobj(obj);
}

void can_init(can_t *obj, PinName rd, PinName td) {
    can_init_freq(obj, rd, td, 100000);
}

void can_free(can_t *obj) {
    LPC_SYSCON->SYSAHBCLKCTRL1 &= ~(1UL << 7);
    LPC_SYSCON->PRESETCTRL1    &= ~(1UL << 7);
}

int can_frequency(can_t *obj, int f) {
    int btr = can_speed(SystemCoreClock, (unsigned int)f, 1);
    int clkdiv = (btr >> 16) & 0x0F;
    btr = btr & 0xFFFF;

    if (btr > 0) {
        // Set the bit clock
        LPC_C_CAN0->CANCNTL |= (1UL << 6 | 1UL << 0); // set CCE and INIT
        LPC_C_CAN0->CANCLKDIV = clkdiv;
        LPC_C_CAN0->CANBT = btr;
        LPC_C_CAN0->CANBRPE = 0x0000;
        LPC_C_CAN0->CANCNTL &= ~(1UL << 6 | 1UL << 0); // clear CCE and INIT
        return 1;
    }
    return 0;
}

int can_write(can_t *obj, CAN_Message msg, int cc) {

    // Make sure controller is enabled
    can_enable(obj);

    // Find first message object that isn't pending to send
    uint16_t msgnum = 0;
    uint32_t txPending = (LPC_C_CAN0->CANTXREQ1 & 0xFF) | (LPC_C_CAN0->CANTXREQ2 << 16);
    uint16_t i = 0;
    for(i = RX_MSG_OBJ_COUNT; i < 32; i++) {
        if ((txPending & (1 << i)) == 0) {
            msgnum = i+1;
            break;
        }
    }

    // If no messageboxes are available, stop and return failure
    if (msgnum == 0) {
        return 0;
    }

    // Make sure the interface is available
    while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY );

    // Set the direction bit based on the message type
    uint32_t direction = 0;
    if (msg.type == CANData) {
        direction = CANIFn_ARB2_DIR;
    }

    if (msg.format == CANExtended)    {
        // Mark message valid, Extended Frame, Set Identifier and mask everything
        LPC_C_CAN0->CANIF1_ARB1 = (msg.id & 0xFFFF);
        LPC_C_CAN0->CANIF1_ARB2 = CANIFn_ARB2_MSGVAL | CANIFn_ARB2_XTD | direction | ((msg.id >> 16) & 0x1FFFF);
        LPC_C_CAN0->CANIF1_MSK1 = (ID_EXT_MASK & 0xFFFF);
        LPC_C_CAN0->CANIF1_MSK2 = CANIFn_MSK2_MXTD | CANIFn_MSK2_MDIR | ((ID_EXT_MASK >> 16) & 0x1FFF);
    } else {
        // Mark message valid, Set Identifier and mask everything
        LPC_C_CAN0->CANIF1_ARB2 = CANIFn_ARB2_MSGVAL | direction | ((msg.id << 2) & 0x1FFF);
        LPC_C_CAN0->CANIF1_MSK2 = CANIFn_MSK2_MDIR | ((ID_STD_MASK << 2) & 0x1FFF);
    }

    // Use mask, request transmission, single message object and set DLC
    LPC_C_CAN0->CANIF1_MCTRL = CANIFn_MCTRL_UMASK | CANIFn_MCTRL_TXRQST | CANIFn_MCTRL_EOB | (msg.len & 0xF);

    LPC_C_CAN0->CANIF1_DA1 = ((msg.data[1] & 0xFF) << 8) | (msg.data[0] & 0xFF);
    LPC_C_CAN0->CANIF1_DA2 = ((msg.data[3] & 0xFF) << 8) | (msg.data[2] & 0xFF);
    LPC_C_CAN0->CANIF1_DB1 = ((msg.data[5] & 0xFF) << 8) | (msg.data[4] & 0xFF);
    LPC_C_CAN0->CANIF1_DB2 = ((msg.data[7] & 0xFF) << 8) | (msg.data[6] & 0xFF);

    // Transfer all fields to message object
    LPC_C_CAN0->CANIF1_CMDMSK_W = CANIFn_CMDMSK_WR | CANIFn_CMDMSK_MASK | CANIFn_CMDMSK_ARB | CANIFn_CMDMSK_CTRL | CANIFn_CMDMSK_TXRQST | CANIFn_CMDMSK_DATA_A | CANIFn_CMDMSK_DATA_B;

    // Start Transfer to given message number
    LPC_C_CAN0->CANIF1_CMDREQ = (msgnum & 0x3F);

    // Wait until transfer to message ram complete - TODO: maybe not block??
    while ( LPC_C_CAN0->CANIF1_CMDREQ & CANIFn_CMDREQ_BUSY);

    // Wait until TXOK is set, then clear it - TODO: maybe not block
    //while ( !(LPC_C_CAN0->STAT & CANSTAT_TXOK) );
    LPC_C_CAN0->CANSTAT &= ~(1UL << 3);

    return 1;
}

int can_read(can_t *obj, CAN_Message *msg, int handle) {
    uint16_t i;

    // Make sure controller is enabled
    can_enable(obj);

    // Find first message object with new data
    if (handle == 0) {
        uint32_t newdata = LPC_C_CAN0->CANND1 | (LPC_C_CAN0->CANND2 << 16);
        // Find first free messagebox
        for (i = 0; i < RX_MSG_OBJ_COUNT; i++) {
            if (newdata & (1 << i)) {
                handle = i+1;
                break;
            }
        }
    }

    if (handle > 0 && handle <= 32) {
        // Wait until message interface is free
        while ( LPC_C_CAN0->CANIF2_CMDREQ & CANIFn_CMDREQ_BUSY );

        // Transfer all fields to message object
        LPC_C_CAN0->CANIF2_CMDMSK_W = CANIFn_CMDMSK_RD | CANIFn_CMDMSK_MASK | CANIFn_CMDMSK_ARB | CANIFn_CMDMSK_CTRL | CANIFn_CMDMSK_CLRINTPND | CANIFn_CMDMSK_TXRQST | CANIFn_CMDMSK_DATA_A | CANIFn_CMDMSK_DATA_B;

        // Start Transfer from given message number
        LPC_C_CAN0->CANIF2_CMDREQ = (handle & 0x3F);

        // Wait until transfer to message ram complete
        while ( LPC_C_CAN0->CANIF2_CMDREQ & CANIFn_CMDREQ_BUSY );

        if (LPC_C_CAN0->CANIF2_ARB2 & CANIFn_ARB2_XTD) {
            msg->format = CANExtended;
            msg->id = (LPC_C_CAN0->CANIF2_ARB2 & 0x1FFF) << 16;
            msg->id |= (LPC_C_CAN0->CANIF2_ARB1 & 0xFFFF);
        } else {
            msg->format = CANStandard;
            msg->id = (LPC_C_CAN0->CANIF2_ARB2 & 0x1FFF) >> 2;
        }

       if (LPC_C_CAN0->CANIF2_ARB2 & CANIFn_ARB2_DIR) {
            msg->type   = CANRemote;
        }
        else {
            msg->type   = CANData;
        }

        msg->len        = (LPC_C_CAN0->CANIF2_MCTRL & 0xF); // TODO: If > 8, len = 8
        msg->data[0]    = ((LPC_C_CAN0->CANIF2_DA1 >> 0) & 0xFF);
        msg->data[1]    = ((LPC_C_CAN0->CANIF2_DA1 >> 8) & 0xFF);
        msg->data[2]    = ((LPC_C_CAN0->CANIF2_DA2 >> 0) & 0xFF);
        msg->data[3]    = ((LPC_C_CAN0->CANIF2_DA2 >> 8) & 0xFF);
        msg->data[4]    = ((LPC_C_CAN0->CANIF2_DB1 >> 0) & 0xFF);
        msg->data[5]    = ((LPC_C_CAN0->CANIF2_DB1 >> 8) & 0xFF);
        msg->data[6]    = ((LPC_C_CAN0->CANIF2_DB2 >> 0) & 0xFF);
        msg->data[7]    = ((LPC_C_CAN0->CANIF2_DB2 >> 8) & 0xFF);

        LPC_C_CAN0->CANSTAT &= ~(1UL << 4);
        return 1;
    }
    return 0;
}

void can_reset(can_t *obj) {
    LPC_SYSCON->PRESETCTRL1 &= ~(1UL << 7);
    LPC_C_CAN0->CANSTAT = 0;
    can_config_rxmsgobj(obj);
    can_config_txmsgobj(obj);
    
    can_enable(obj);  // clears a bus-off condition if necessary
}

unsigned char can_rderror(can_t *obj) {
    return ((LPC_C_CAN0->CANEC >> 8) & 0x7F);
}

unsigned char can_tderror(can_t *obj) {
    return (LPC_C_CAN0->CANEC & 0xFF);
}

void can_monitor(can_t *obj, int silent) {
    if (silent) {
        LPC_C_CAN0->CANCNTL |= (1UL << 7);
        LPC_C_CAN0->CANTEST |= (1UL << 3);
    } else {
        LPC_C_CAN0->CANCNTL &= ~(1UL << 7);
        LPC_C_CAN0->CANTEST &= ~(1UL << 3);
    }

    if (!(LPC_C_CAN0->CANCNTL & (1UL << 0))) {
        LPC_C_CAN0->CANCNTL |= (1UL << 0);
    }
}

const PinMap *can_rd_pinmap()
{
    return PinMap_CAN_testing;
}

const PinMap *can_td_pinmap()
{
    return PinMap_CAN_testing;
}
