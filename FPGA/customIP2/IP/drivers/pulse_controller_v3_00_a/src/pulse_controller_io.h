#ifndef PULSE_CONTROLLER_IO_H
#define PULSE_CONTROLLER_IO_H
/***************************** Include Files *******************************/

#include "xbasic_types.h"
#include "xstatus.h"

#include "xio.h"

/************************** Constant Definitions ***************************/


/**
 * User Logic Slave Space Offsets
 * -- SLV_REG0 : user logic slave module register 0
 * -- SLV_REG1 : user logic slave module register 1
 * -- SLV_REG2 : user logic slave module register 2
 * -- SLV_REG3 : user logic slave module register 3
 * -- SLV_REG4 : user logic slave module register 4
 * -- SLV_REG5 : user logic slave module register 5
 * -- SLV_REG6 : user logic slave module register 6
 * -- SLV_REG7 : user logic slave module register 7
 */
#define PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET (0x00000000)
#define PULSE_CONTROLLER_SLV_REG0_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000000)
#define PULSE_CONTROLLER_SLV_REG1_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000008)
#define PULSE_CONTROLLER_SLV_REG2_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000010)
#define PULSE_CONTROLLER_SLV_REG3_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000018)
#define PULSE_CONTROLLER_SLV_REG4_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000020)
#define PULSE_CONTROLLER_SLV_REG5_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000028)
#define PULSE_CONTROLLER_SLV_REG6_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000030)
#define PULSE_CONTROLLER_SLV_REG7_OFFSET (PULSE_CONTROLLER_USER_SLV_SPACE_OFFSET + 0x00000038)

/**
 * Software Reset Space Register Offsets
 * -- RST : software reset register
 */
#define PULSE_CONTROLLER_SOFT_RST_SPACE_OFFSET (0x00000100)
#define PULSE_CONTROLLER_RST_REG_OFFSET (PULSE_CONTROLLER_SOFT_RST_SPACE_OFFSET + 0x00000000)

/**
 * Software Reset Masks
 * -- SOFT_RESET : software reset
 */
#define SOFT_RESET (0x0000000A)

/**
 * Read Packet FIFO Register/Data Space Offsets
 * -- RDFIFO_RST  : read packet fifo reset register
 * -- RDFIFO_SR   : read packet fifo status register
 * -- RDFIFO_DATA : read packet fifo data
 */
#define PULSE_CONTROLLER_RDFIFO_REG_SPACE_OFFSET (0x00000200)
#define PULSE_CONTROLLER_RDFIFO_RST_OFFSET (PULSE_CONTROLLER_RDFIFO_REG_SPACE_OFFSET + 0x00000000)
#define PULSE_CONTROLLER_RDFIFO_SR_OFFSET (PULSE_CONTROLLER_RDFIFO_REG_SPACE_OFFSET + 0x00000004)
#define PULSE_CONTROLLER_RDFIFO_DATA_SPACE_OFFSET (0x00000300)
#define PULSE_CONTROLLER_RDFIFO_DATA_OFFSET (PULSE_CONTROLLER_RDFIFO_DATA_SPACE_OFFSET + 0x00000000)

/**
 * Read Packet FIFO Masks
 * -- RDFIFO_EMPTY_MASK : read packet fifo empty condition
 * -- RDFIFO_AE_MASK    : read packet fifo almost empty condition
 * -- RDFIFO_DL_MASK    : read packet fifo deadlock condition
 * -- RDFIFO_SCL_MASK   : read packet fifo occupancy scaling enabled
 * -- RDFIFO_WIDTH_MASK : read packet fifo encoded data port width
 * -- RDFIFO_OCC_MASK   : read packet fifo occupancy
 * -- RDFIFO_RESET      : read packet fifo reset
 */
#define RDFIFO_EMPTY_MASK (0x80000000UL)
#define RDFIFO_AE_MASK (0x40000000UL)
#define RDFIFO_DL_MASK (0x20000000UL)
#define RDFIFO_SCL_MASK (0x10000000UL)
#define RDFIFO_WIDTH_MASK (0x0E000000UL)
#define RDFIFO_OCC_MASK (0x01FFFFFFUL)
#define RDFIFO_RESET (0x0000000A)

/**
 * Write Packet FIFO Register/Data Space Offsets
 * -- WRFIFO_RST  : write packet fifo reset register
 * -- WRFIFO_SR   : write packet fifo status register
 * -- WRFIFO_DATA : write packet fifo data
 */
#define PULSE_CONTROLLER_WRFIFO_REG_SPACE_OFFSET (0x00000400)
#define PULSE_CONTROLLER_WRFIFO_RST_OFFSET (PULSE_CONTROLLER_WRFIFO_REG_SPACE_OFFSET + 0x00000000)
#define PULSE_CONTROLLER_WRFIFO_SR_OFFSET (PULSE_CONTROLLER_WRFIFO_REG_SPACE_OFFSET + 0x00000004)
#define PULSE_CONTROLLER_WRFIFO_DATA_SPACE_OFFSET (0x00000500)
#define PULSE_CONTROLLER_WRFIFO_DATA_OFFSET (PULSE_CONTROLLER_WRFIFO_DATA_SPACE_OFFSET + 0x00000000)

/**
 * Write Packet FIFO Masks
 * -- WRFIFO_FULL_MASK  : write packet fifo full condition
 * -- WRFIFO_AF_MASK    : write packet fifo almost full condition
 * -- WRFIFO_DL_MASK    : write packet fifo deadlock condition
 * -- WRFIFO_SCL_MASK   : write packet fifo vacancy scaling enabled
 * -- WRFIFO_WIDTH_MASK : write packet fifo encoded data port width
 * -- WRFIFO_DREP_MASK  : write packet fifo DRE present
 * -- WRFIFO_VAC_MASK   : write packet fifo vacancy
 * -- WRFIFO_RESET      : write packet fifo reset
 */
#define WRFIFO_FULL_MASK (0x80000000UL)
#define WRFIFO_AF_MASK (0x40000000UL)
#define WRFIFO_DL_MASK (0x20000000UL)
#define WRFIFO_SCL_MASK (0x10000000UL)
#define WRFIFO_WIDTH_MASK (0x0E000000UL)
#define WRFIFO_DREP_MASK (0x01000000UL)
#define WRFIFO_VAC_MASK (0x00FFFFFFUL)
#define WRFIFO_RESET (0x0000000A)

/**************************** Type Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *******************/

/**
 *
 * Write a value to a PULSE_CONTROLLER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mWriteReg(Xuint32 BaseAddress, unsigned RegOffset, Xuint32 Data)
 *
 */
#define PULSE_CONTROLLER_mWriteReg(BaseAddress, RegOffset, Data) \
 	XIo_Out32((BaseAddress) + (RegOffset), (Xuint32)(Data))

/**
 *
 * Read a value from a PULSE_CONTROLLER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	Xuint32 PULSE_CONTROLLER_mReadReg(Xuint32 BaseAddress, unsigned RegOffset)
 *
 */
#define PULSE_CONTROLLER_mReadReg(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (RegOffset))


/**
 *
 * Write/Read 32 bit value to/from PULSE_CONTROLLER user logic slave registers.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   RegOffset is the offset from the slave register to write to or read from.
 * @param   Value is the data written to the register.
 *
 * @return  Data is the data from the user logic slave register.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mWriteSlaveRegn(Xuint32 BaseAddress, unsigned RegOffset, Xuint32 Value)
 * 	Xuint32 PULSE_CONTROLLER_mReadSlaveRegn(Xuint32 BaseAddress, unsigned RegOffset)
 *
 */
#define PULSE_CONTROLLER_mWriteSlaveReg0(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg1(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg2(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg3(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg4(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg5(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg6(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset), (Xuint32)(Value))
#define PULSE_CONTROLLER_mWriteSlaveReg7(BaseAddress, RegOffset, Value) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset), (Xuint32)(Value))

#define PULSE_CONTROLLER_mReadSlaveReg0(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG0_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg1(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG1_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg2(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG2_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg3(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG3_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg4(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG4_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg5(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG5_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg6(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG6_OFFSET) + (RegOffset))
#define PULSE_CONTROLLER_mReadSlaveReg7(BaseAddress, RegOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_SLV_REG7_OFFSET) + (RegOffset))

/**
 *
 * Reset PULSE_CONTROLLER via software.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mReset(Xuint32 BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mReset(BaseAddress) \
 	XIo_Out32((BaseAddress)+(PULSE_CONTROLLER_RST_REG_OFFSET), SOFT_RESET)

/**
 *
 * Reset read packet FIFO of PULSE_CONTROLLER to its initial state.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mResetReadFIFO(Xuint32 BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mResetReadFIFO(BaseAddress) \
 	XIo_Out32((BaseAddress)+(PULSE_CONTROLLER_RDFIFO_RST_OFFSET), RDFIFO_RESET)

/**
 *
 * Check status of PULSE_CONTROLLER read packet FIFO module.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  Status is the result of status checking.
 *
 * @note
 * C-style signature:
 * 	bool PULSE_CONTROLLER_mReadFIFOEmpty(Xuint32 BaseAddress)
 * 	Xuint32 PULSE_CONTROLLER_mReadFIFOOccupancy(Xuint32 BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mReadFIFOEmpty(BaseAddress) \
 	((XIo_In32((BaseAddress)+(PULSE_CONTROLLER_RDFIFO_SR_OFFSET)) & RDFIFO_EMPTY_MASK) == RDFIFO_EMPTY_MASK)
#define PULSE_CONTROLLER_mReadFIFOOccupancy(BaseAddress) \
 	(XIo_In32((BaseAddress)+(PULSE_CONTROLLER_RDFIFO_SR_OFFSET)) & RDFIFO_OCC_MASK)

/**
 *
 * Read 32 bit data from PULSE_CONTROLLER read packet FIFO module.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   DataOffset is the offset from the data port to read from.
 *
 * @return  Data is the data from the read packet FIFO.
 *
 * @note
 * C-style signature:
 * 	Xuint32 PULSE_CONTROLLER_mReadFromFIFO(Xuint32 BaseAddress, unsigned DataOffset)
 *
 */
#define PULSE_CONTROLLER_mReadFromFIFO(BaseAddress, DataOffset) \
 	XIo_In32((BaseAddress) + (PULSE_CONTROLLER_RDFIFO_DATA_OFFSET) + (DataOffset))

/**
 *
 * Reset write packet FIFO of PULSE_CONTROLLER to its initial state.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mResetWriteFIFO(Xuint32 BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mResetWriteFIFO(BaseAddress) \
 	XIo_Out32((BaseAddress)+(PULSE_CONTROLLER_WRFIFO_RST_OFFSET), WRFIFO_RESET)

/**
 *
 * Check status of PULSE_CONTROLLER write packet FIFO module.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 *
 * @return  Status is the result of status checking.
 *
 * @note
 * C-style signature:
 * 	bool PULSE_CONTROLLER_mWriteFIFOFull(Xuint32 BaseAddress)
 * 	Xuint32 PULSE_CONTROLLER_mWriteFIFOVacancy(Xuint32 BaseAddress)
 *
 */
#define PULSE_CONTROLLER_mWriteFIFOFull(BaseAddress) \
 	((XIo_In32((BaseAddress)+(PULSE_CONTROLLER_WRFIFO_SR_OFFSET)) & WRFIFO_FULL_MASK) == WRFIFO_FULL_MASK)
#define PULSE_CONTROLLER_mWriteFIFOVacancy(BaseAddress) \
 	(XIo_In32((BaseAddress)+(PULSE_CONTROLLER_WRFIFO_SR_OFFSET)) & WRFIFO_VAC_MASK)

/**
 *
 * Write 32 bit data to PULSE_CONTROLLER write packet FIFO module.
 *
 * @param   BaseAddress is the base address of the PULSE_CONTROLLER device.
 * @param   DataOffset is the offset from the data port to write to.
 * @param   Data is the value to be written to write packet FIFO.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void PULSE_CONTROLLER_mWriteToFIFO(Xuint32 BaseAddress, unsigned DataOffset, Xuint32 Data)
 *
 */
#define PULSE_CONTROLLER_mWriteToFIFO(BaseAddress, DataOffset, Data) \
 	XIo_Out32((BaseAddress) + (PULSE_CONTROLLER_WRFIFO_DATA_OFFSET) + (DataOffset), (Xuint32)(Data))

#endif // PULSE_CONTROLLER_IO_H
