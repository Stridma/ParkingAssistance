/*
 * mcp_can.h
 *
 */ 


#include "sam.h"


#ifndef MCP_CAN_H_
#define MCP_CAN_H_


/******************* SIMPLE FUNCTIONS *******************/


void mcp2515_reset(void);
uint8_t mcp2515_readRegister(const uint8_t address);
void mcp2515_readRegisterS(const uint8_t address, uint8_t values[], const uint8_t n);
void mcp2515_setRegister(const uint8_t address, const uint8_t value);
void mcp2515_setRegisterS(const uint8_t address, const uint8_t values[], const uint8_t n);
void mcp2515_modifyRegister(const uint8_t address, const uint8_t mask, const uint8_t data);
uint8_t mcp2515_readStatus(void);


/****************** COMPOUND FUNCTIONS ******************/


/******************** INIT SEQUENCES ********************/


uint8_t mcp2515_setCANCTRL_Mode(const uint8_t newmode);
uint8_t mcp2515_configRate(const uint8_t canSpeed);
void mcp2515_initCANBuffers(void);
uint8_t mcp2515_init(const uint8_t canSpeed);


/******************** MANAGEMENT TOOLS *******************/


void mcp2515_write_id(const uint8_t mcp_addr, const uint8_t ext, const uint16_t id);
void mcp2515_write_mf(const uint8_t mcp_addr, const uint8_t ext, const uint32_t id );
void mcp2515_read_id(const uint8_t mcp_addr, uint8_t* ext, uint16_t* id);
void mcp2515_write_canMsg(const uint8_t buffer_sidh_addr);
void mcp2515_read_canMsg(const uint8_t buffer_sidh_addr);
void mcp2515_start_transmit(const uint8_t mcp_addr);
uint8_t mcp2515_getNextFreeTXBuf(uint8_t *txbuf_n);


/******************** START MCP SLAVE *******************/


uint8_t init_Mask(uint8_t num, uint32_t ulData);
uint8_t init_Filt(uint8_t num, uint8_t ext, uint32_t ulData);
uint8_t setMsg(uint16_t id, uint8_t ext, uint8_t len, uint8_t *pData);
uint8_t clearMsg(void);
uint8_t sendMsg(void);
uint8_t sendMsgBuf(uint16_t id, uint8_t ext, uint8_t len, uint8_t *buf);
uint8_t readMsg(void);
uint8_t readMsgBuf(uint8_t *len, uint8_t buf[]);
uint8_t readMsgBufID(uint16_t *ID, uint8_t *len, uint8_t buf[]);
uint8_t checkReceive(void);
uint32_t getCanId(void);
uint8_t isRemoteRequest(void);
uint8_t isExtendedFrame(void);
uint8_t canBegin(uint8_t slave, uint8_t speedset);
void slaveSelect(uint8_t slave);


#endif /* MCP_CAN_H_ */