#include "mcp_can_dfs.h"
#include "mcp_can.h"
#include "spi.h"
#include "sam.h"




uint8_t                        m_nExtFlg;                     /* identifier xxxID             */
                                                          /* either extended (the 29 LSB) */
                                                          /* or standard (the 11 LSB)     */
uint16_t                m_nID;                         /* can id                       */
uint8_t                        m_nDlc;                        /* data length:                 */
uint8_t                        m_nDta[MAX_CHAR_IN_MESSAGE];   /* data                         */
uint8_t                        m_nRtr;                        /* rtr                          */
uint8_t                        m_nfilhit;
uint8_t                        device;


/******************* SIMPLE FUNCTIONS *******************/


void mcp2515_reset(void) {
        // Reset Instruction = 0xC0
        spiSS(device);
        spiSend(MCP_RESET);
        spiSR(device);
}


uint8_t mcp2515_readRegister(const uint8_t address) {
        uint8_t ret;
        spiSS(device);
        spiSend(MCP_READ);
        spiSend(address);
        ret = spiSend(0x00);
        spiSR(device);
        return ret;
}


void mcp2515_readRegisterS(const uint8_t address, uint8_t values[], const uint8_t n) {
        uint8_t i;
        spiSS(device);
        spiSend(MCP_READ);
        spiSend(address);
        // mcp2515 has auto-increment of address-pointer
        for (i=0; i<n && i<CAN_MAX_CHAR_IN_MESSAGE; i++) {
                values[i] = spiSend(0x00);
        }
        spiSR(device);
}


void mcp2515_setRegister(const uint8_t address, const uint8_t value) {
        spiSS(device);
        spiSend(MCP_WRITE);
        spiSend(address);
        spiSend(value);
        spiSR(device);
}


void mcp2515_setRegisterS(const uint8_t address, const uint8_t values[], const uint8_t n) {
        uint8_t i;
        spiSS(device);
        spiSend(MCP_WRITE);
        spiSend(address);
        for (i=0; i<n; i++)
        {
                spiSend(values[i]);
        }
        spiSR(device);
}


void mcp2515_modifyRegister(const uint8_t address, const uint8_t mask, const uint8_t data) {
        spiSS(device);				// Sets CS to 0 to prepare for writing (PA18 -> CS)
        spiSend(MCP_BITMOD);		// Sends bit modify instruction (0x05)
        spiSend(address);			// Sends a byte indicating the address that wants to be modified
        spiSend(mask);				// Sends mask for the data to be applied
        spiSend(data);				// Sends the data to be masked and written into the register on address
        spiSR(device);				// Sets CS to 1 to stop writing (PA18 -> CS)
}


uint8_t mcp2515_readStatus(void) {
        uint8_t ret;
        spiSS(device);				// Sets CS to 0 to prepare for reading (PA18 -> CS)
        spiSend(MCP_READ_STATUS);	// Sends read instruction
        ret = spiSend(0x00);		// Sends one byte (don't care) to read one byte (status register)
		spiSend(0x00);				// Repeat data out
        spiSR(device);				// Sets CS to 1 to stop reading (PA18 -> CS)
        return ret;					// Returns the value read through SPI (status register)
}


/****************** COMPOUND FUNCTIONS ******************/


/******************** INIT SEQUENCES ********************/


uint8_t mcp2515_setCANCTRL_Mode(const uint8_t newmode) {
        uint8_t i;


        mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);


        i = mcp2515_readRegister(MCP_CANCTRL);
        i &= MODE_MASK;


        if ( i == newmode )
        {
                return MCP2515_OK;
        }
        return MCP2515_FAIL;
}


uint8_t mcp2515_configRate(const uint8_t canSpeed) {
        uint8_t set, cfg1, cfg2, cfg3;
        set = 1;
        switch (canSpeed)
        {
                case (CAN_5KBPS):
                cfg1 = MCP_16MHz_5kBPS_CFG1;
                cfg2 = MCP_16MHz_5kBPS_CFG2;
                cfg3 = MCP_16MHz_5kBPS_CFG3;
                break;


                case (CAN_10KBPS):
                cfg1 = MCP_16MHz_10kBPS_CFG1;
                cfg2 = MCP_16MHz_10kBPS_CFG2;
                cfg3 = MCP_16MHz_10kBPS_CFG3;
                break;


                case (CAN_20KBPS):
                cfg1 = MCP_16MHz_20kBPS_CFG1;
                cfg2 = MCP_16MHz_20kBPS_CFG2;
                cfg3 = MCP_16MHz_20kBPS_CFG3;
                break;
                
                case (CAN_31K25BPS):
                cfg1 = MCP_16MHz_31k25BPS_CFG1;
                cfg2 = MCP_16MHz_31k25BPS_CFG2;
                cfg3 = MCP_16MHz_31k25BPS_CFG3;
                break;


                case (CAN_33KBPS):
                cfg1 = MCP_16MHz_33kBPS_CFG1;
                cfg2 = MCP_16MHz_33kBPS_CFG2;
                cfg3 = MCP_16MHz_33kBPS_CFG3;
                break;


                case (CAN_40KBPS):
                cfg1 = MCP_16MHz_40kBPS_CFG1;
                cfg2 = MCP_16MHz_40kBPS_CFG2;
                cfg3 = MCP_16MHz_40kBPS_CFG3;
                break;


                case (CAN_50KBPS):
                cfg1 = MCP_16MHz_50kBPS_CFG1;
                cfg2 = MCP_16MHz_50kBPS_CFG2;
                cfg3 = MCP_16MHz_50kBPS_CFG3;
                break;


                case (CAN_80KBPS):
                cfg1 = MCP_16MHz_80kBPS_CFG1;
                cfg2 = MCP_16MHz_80kBPS_CFG2;
                cfg3 = MCP_16MHz_80kBPS_CFG3;
                break;


                case (CAN_83K3BPS):
                cfg1 = MCP_16MHz_83k3BPS_CFG1;
                cfg2 = MCP_16MHz_83k3BPS_CFG2;
                cfg3 = MCP_16MHz_83k3BPS_CFG3;
                break;


                case (CAN_95KBPS):
                cfg1 = MCP_16MHz_95kBPS_CFG1;
                cfg2 = MCP_16MHz_95kBPS_CFG2;
                cfg3 = MCP_16MHz_95kBPS_CFG3;
                break;


                case (CAN_100KBPS):                                             /* 100KBPS                  */
                cfg1 = MCP_16MHz_100kBPS_CFG1;
                cfg2 = MCP_16MHz_100kBPS_CFG2;
                cfg3 = MCP_16MHz_100kBPS_CFG3;
                break;


                case (CAN_125KBPS):
                cfg1 = MCP_16MHz_125kBPS_CFG1;
                cfg2 = MCP_16MHz_125kBPS_CFG2;
                cfg3 = MCP_16MHz_125kBPS_CFG3;
                break;


                case (CAN_200KBPS):
                cfg1 = MCP_16MHz_200kBPS_CFG1;
                cfg2 = MCP_16MHz_200kBPS_CFG2;
                cfg3 = MCP_16MHz_200kBPS_CFG3;
                break;


                case (CAN_250KBPS):
                cfg1 = MCP_16MHz_250kBPS_CFG1;
                cfg2 = MCP_16MHz_250kBPS_CFG2;
                cfg3 = MCP_16MHz_250kBPS_CFG3;
                break;


                case (CAN_500KBPS):
                cfg1 = MCP_16MHz_500kBPS_CFG1;
                cfg2 = MCP_16MHz_500kBPS_CFG2;
                cfg3 = MCP_16MHz_500kBPS_CFG3;
                break;
                
                case (CAN_1000KBPS):
                cfg1 = MCP_16MHz_1000kBPS_CFG1;
                cfg2 = MCP_16MHz_1000kBPS_CFG2;
                cfg3 = MCP_16MHz_1000kBPS_CFG3;
                break;


                default:
                set = 0;
                break;
        }


        if (set) {
                mcp2515_setRegister(MCP_CNF1, cfg1);
                mcp2515_setRegister(MCP_CNF2, cfg2);
                mcp2515_setRegister(MCP_CNF3, cfg3);
                return MCP2515_OK;
        }
        else {
                return MCP2515_FAIL;
        }
}


void mcp2515_initCANBuffers(void) {
        uint8_t i, a1, a2, a3;
        
        uint8_t std = 0;
        uint8_t ext = 1;
        uint32_t ulMask = 0x00000000;
        uint32_t ulFilt = 0x00000000;


        mcp2515_write_mf(MCP_RXM0SIDH, ext, ulMask);        /*Set both masks to 0           */
        mcp2515_write_mf(MCP_RXM1SIDH, ext, ulMask);        /*Mask register ignores ext bit */
        
        /* Set all filters to 0         */
        mcp2515_write_mf(MCP_RXF0SIDH, ext, ulFilt);        /* RXB0: extended               */
        mcp2515_write_mf(MCP_RXF1SIDH, std, ulFilt);        /* RXB1: standard               */
        mcp2515_write_mf(MCP_RXF2SIDH, ext, ulFilt);        /* RXB2: extended               */
        mcp2515_write_mf(MCP_RXF3SIDH, std, ulFilt);        /* RXB3: standard               */
        mcp2515_write_mf(MCP_RXF4SIDH, ext, ulFilt);
        mcp2515_write_mf(MCP_RXF5SIDH, std, ulFilt);


        /* Clear, deactivate the three  */
        /* transmit buffers             */
        /* TXBnCTRL -> TXBnD7           */
        a1 = MCP_TXB0CTRL;
        a2 = MCP_TXB1CTRL;
        a3 = MCP_TXB2CTRL;
        for (i = 0; i < 14; i++) {                         /* in-buffer loop               */
                mcp2515_setRegister(a1, 0);
                mcp2515_setRegister(a2, 0);
                mcp2515_setRegister(a3, 0);
                a1++;
                a2++;
                a3++;
        }
        mcp2515_setRegister(MCP_RXB0CTRL, 0);
        mcp2515_setRegister(MCP_RXB1CTRL, 0);
}


uint8_t mcp2515_init(const uint8_t canSpeed) {            /* mcp2515init                  */


        uint8_t res;


        mcp2515_reset();
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }


        res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
        if(res > 0)
        {
                for(int aux=0;aux<500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }


        /* set boadrate                 */
        if(mcp2515_configRate(canSpeed))
        {
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }


        if ( res == MCP2515_OK ) {


                /* init canbuffers              */
                mcp2515_initCANBuffers();


                /* interrupt mode               */
                mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);
                
                /* enable both receive-buffers    */
                /* to receive messages            */
                /* with std. and ext. identifiers */
                /* and enable rollover            */
                mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
                mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
                /* enter normal mode            */
                res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);
                if(res)
                {
                        for(int aux=0;aux<1500;aux++) 
// We need to wait about X microseconds to let it load
                        {
                                ;//this works like a NOPE instruction
                        }
                        return res;
                }
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
        }
        return res;
}


/******************** MANAGMENT TOOLS *******************/


void mcp2515_write_id(const uint8_t mcp_addr, const uint8_t ext, const uint16_t id) {
        uint16_t canid;
        uint8_t tbufdata[4];


        canid = (uint16_t)(id & 0x0FFFF);


        if (ext == 1) {
                tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
                tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
                canid = (uint16_t)(id >> 16);
                tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
                tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
                tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5);
        }
        else {
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3);
                tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07) << 5);
                tbufdata[MCP_EID0] = 0;
                tbufdata[MCP_EID8] = 0;
        }
        mcp2515_setRegisterS(mcp_addr, tbufdata, 4);
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
                {
                        ;//this works like a NOPE instruction
                }
}


void mcp2515_write_mf( const uint8_t mcp_addr, const uint8_t ext, const uint32_t id )
{
        uint16_t canid;
        uint8_t tbufdata[4];
        canid = (uint16_t)(id & 0x0FFFF);


        if ( ext == 1)
        {
                tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
                tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
                canid = (uint16_t)(id >> 16);
                tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
                tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
                tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5 );
        }
        else
        {
                tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
                tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
                canid = (uint16_t)(id >> 16);
                tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07) << 5);
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3 );
        }
        
        mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
}






void mcp2515_read_id(const uint8_t mcp_addr, uint8_t* ext, uint16_t* id) {
        uint8_t tbufdata[4];


        *ext = 0;
        *id = 0;


        mcp2515_readRegisterS(mcp_addr, tbufdata, 4);


        *id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);


        if ((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M)
        {
                /* extended id                  */
                *id = (*id<<2) + (tbufdata[MCP_SIDL] & 0x03);
                *id = (*id<<8) + tbufdata[MCP_EID8];
                *id = (*id<<8) + tbufdata[MCP_EID0];
                *ext = 1;
        }
}


void mcp2515_write_canMsg(const uint8_t buffer_sidh_addr) {
        uint8_t mcp_addr;
        mcp_addr = buffer_sidh_addr;
        mcp2515_setRegisterS(mcp_addr+5, m_nDta, m_nDlc);    /* write data bytes             */
        if ( m_nRtr == 1)                                    /* if RTR set bit in byte       */
        {
                m_nDlc |= MCP_RTR_MASK;
        }
        mcp2515_setRegister((mcp_addr+4), m_nDlc);          /* write the RTR and DLC        */
        mcp2515_write_id(mcp_addr, m_nExtFlg, m_nID);       /* write CAN id                 */
}


void mcp2515_read_canMsg(const uint8_t buffer_sidh_addr) { /* read can msg                 */
        uint8_t mcp_addr, ctrl;


        mcp_addr = buffer_sidh_addr;


        mcp2515_read_id(mcp_addr, &m_nExtFlg, &m_nID);


        ctrl = mcp2515_readRegister(mcp_addr-1);
        m_nDlc = mcp2515_readRegister(mcp_addr+4);


        if ((ctrl & 0x08)) {
                m_nRtr = 1;
        }
        else {
                m_nRtr = 0;
        }


        m_nDlc &= MCP_DLC_MASK;
        mcp2515_readRegisterS(mcp_addr+5, &(m_nDta[0]), m_nDlc);
}


void mcp2515_start_transmit(const uint8_t mcp_addr)   {     /* start transmit               */
        mcp2515_modifyRegister(mcp_addr-1 , MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);
}


uint8_t mcp2515_getNextFreeTXBuf(uint8_t *txbuf_n) {      /* get Next free txbuf          */
        uint8_t res, i, ctrlval;
        uint8_t ctrlregs[MCP_N_TXBUFFERS] = {MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL};


        res = MCP_ALLTXBUSY;
        *txbuf_n = 0x00;
        //


        /* check all 3 TX-Buffers       */
        for (i=0; i<MCP_N_TXBUFFERS; i++) {
                ctrlval = mcp2515_readRegister(ctrlregs[i]);
                if ( (ctrlval & MCP_TXB_TXREQ_M) == 0) {
                        *txbuf_n = ctrlregs[i]+1;          /* return SIDH-address of Buffe */
                        /* r                            */
                        res = MCP2515_OK;
                        return res;                       /* ! function exit              */
                }
        }
        return res;
}


/******************** START MCP SLAVE *******************/


uint8_t init_Mask(uint8_t num, uint32_t ulData) {
        uint8_t res = MCP2515_OK;
        uint8_t ext =0;
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        if(res > 0){
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        
        if((ulData & 0x80000000) == 0x80000000)
            ext = 1;
        
        if (num == 0){
                mcp2515_write_mf(MCP_RXM0SIDH, ext, ulData);
        }
        else if(num == 1){
                mcp2515_write_mf(MCP_RXM1SIDH, ext, ulData);
        }
        else res =  MCP2515_FAIL;
        
        res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);
        if(res > 0){
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        return res;
}


uint8_t init_Filt(uint8_t num, uint8_t ext, uint32_t ulData) {
        uint8_t res = MCP2515_OK;
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
        if(res > 0)
        {
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<500;aux++) // We need to wait about X microseconds to let it load
                {
                        ;//this works like a NOPE instruction
                }
        
        switch(num)
        {
                case 0:
                mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
                break;


                case 1:
                mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
                break;


                case 2:
                mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
                break;


                case 3:
                mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
                break;


                case 4:
                mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
                break;


                case 5:
                mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
                break;


                default:
                res = MCP2515_FAIL;
        }
        
        res = mcp2515_setCANCTRL_Mode(MODE_NORMAL);
        if(res > 0)
        {
                for(int aux=0;aux<1500;aux++) // We need to wait about X microsec to let it load
                {
                        ;//this works like a NOPE instruction
                }
                return res;
        }
        for(int aux=0;aux<1500;aux++) // We need to wait about X microseconds to let it load
        {
                ;//this works like a NOPE instruction
        }
        return res;
}


uint8_t setMsg(uint16_t id, uint8_t ext, uint8_t len, uint8_t *pData) {
        int i = 0;
        m_nExtFlg = ext;
        m_nID     = id;
        m_nDlc    = len;
        for(i = 0; i<MAX_CHAR_IN_MESSAGE; i++)
        {
                m_nDta[i] = *(pData+i);
        }
        return MCP2515_OK;
}


uint8_t clearMsg() {
        m_nID       = 0;
        m_nDlc      = 0;
        m_nExtFlg   = 0;
        m_nRtr      = 0;
        m_nfilhit   = 0;
        for(int i = 0; i<m_nDlc; i++ )
        m_nDta[i] = 0x00;


        return MCP2515_OK;
}


uint8_t sendMsg() {
        uint8_t res, res1, txbuf_n;
        uint16_t uiTimeOut = 0;


        do {
                res = mcp2515_getNextFreeTXBuf(&txbuf_n);    /* info = addr.                 */
                uiTimeOut++;
        } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));


        if(uiTimeOut == TIMEOUTVALUE)
        {
                return CAN_GETTXBFTIMEOUT;                  /* get tx buff time out         */
        }
        uiTimeOut = 0;
        mcp2515_write_canMsg(txbuf_n);
        mcp2515_start_transmit(txbuf_n);
        do
        {
                uiTimeOut++;
                res1= mcp2515_readRegister(txbuf_n);      /* read send buff ctrl reg         */
                res1 = res1 & 0x08;
        }while(res1 && (uiTimeOut < TIMEOUTVALUE));
        if(uiTimeOut == TIMEOUTVALUE)                    /* send msg timeout             */
        {
                return CAN_SENDMSGTIMEOUT;
        }
        return CAN_OK;


}


uint8_t sendMsgBuf(uint16_t id, uint8_t ext, uint8_t len, uint8_t *buf) {
        setMsg(id, ext, len, buf);
        return sendMsg();
}


uint8_t readMsg() {
        uint8_t stat, res;


        stat = mcp2515_readStatus();


        if (stat & MCP_STAT_RX0IF)                          /* Msg in Buffer 0              */
        {
                mcp2515_read_canMsg(MCP_RXBUF_0);
                mcp2515_modifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
                res = CAN_OK;
        }
        else if (stat & MCP_STAT_RX1IF)                    /* Msg in Buffer 1              */
        {
                mcp2515_read_canMsg(MCP_RXBUF_1);
                mcp2515_modifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
                res = CAN_OK;
        }
        else
        {
                res = CAN_NOMSG;
        }
        return res;
}


uint8_t readMsgBuf(uint8_t *len, uint8_t buf[]) {
        uint8_t  rc;
        
        rc = readMsg();
        
        if (rc == CAN_OK) {
                *len = m_nDlc;
                for(int i = 0; i<m_nDlc; i++) {
                        buf[i] = m_nDta[i];
                }
                } else {
                //*len = 0;
        }
        return rc;
}


uint8_t readMsgBufID(uint16_t *ID, uint8_t *len, uint8_t buf[]) {
        uint8_t rc;
        rc = readMsg();


        if (rc == CAN_OK) {
                *len = m_nDlc;
                *ID  = m_nID;
                for(int i = 0; i<m_nDlc && i < MAX_CHAR_IN_MESSAGE; i++) {
                        buf[i] = m_nDta[i];
                }
                } else {
                //*len = 0;
        }
        return rc;
}


uint8_t checkReceive(void) {
        uint8_t res;
        res = mcp2515_readStatus();                  /* RXnIF in Bit 1 and 0         */
        if (res & MCP_STAT_RXIF_MASK)
        {
                return CAN_MSGAVAIL;
        }
        else
        {
                return CAN_NOMSG;
        }
}


uint32_t getCanId(void) {
        return m_nID;
}


uint8_t isRemoteRequest(void) {
        return m_nRtr;
}


uint8_t isExtendedFrame(void) {
        return m_nExtFlg;
}


uint8_t canBegin(uint8_t slave, uint8_t speedset) {
        uint8_t res;
        device = slave;
        res = mcp2515_init(speedset);
        if (res == MCP2515_OK) 
                return CAN_OK;
        else 
                return CAN_FAILINIT;
}


void slaveSelect(uint8_t slave) {
        spiSend(0xFF);
        device = slave;
}