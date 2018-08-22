#include "Dps422.h"

int16_t Dps422::getSingleResult(int32_t &result)
{
	int16_t rdy;
	switch (m_opMode)
	{
	case CMD_TEMP: //temperature
		rdy = readByteBitfield(registers[TEMP_RDY]);
		break;
	case CMD_PRS: //pressure
		rdy = readByteBitfield(registers[PRS_RDY]);
		break;
	case CMD_BOTH:
		rdy = readByteBitfield(registers[PRS_RDY]) & readByteBitfield(registers[TEMP_RDY]);
	default: //not in command mode
		return DPS__FAIL_TOOBUSY;
	}

	//read new measurement result
	switch (rdy)
	{
	case DPS__FAIL_UNKNOWN: //could not read ready flag
		return DPS__FAIL_UNKNOWN;
	case 0: //ready flag not set, measurement still in progress
		return DPS__FAIL_UNFINISHED;
	case 1: //measurement ready, expected case
		DpsClass::Mode oldMode = m_opMode;
		m_opMode = IDLE; //opcode was automatically reseted by DPS310
		int32_t raw_val;
		switch (oldMode)
		{
		case CMD_TEMP: //temperature
			getRawResult(&raw_val, registerBlocks[TEMP]);
			result = calcTemp(raw_val);
			return DPS__SUCCEEDED; // TODO
		case CMD_PRS: //pressure
			getRawResult(&raw_val, registerBlocks[PRS]);
			result = calcPressure(raw_val);
			return DPS__SUCCEEDED; // TODO
		case CMD_BOTH:
			return 0; // TODO
		default:
			return DPS__FAIL_UNKNOWN; //should already be filtered above
		}
	}
	return DPS__FAIL_UNKNOWN;
}

int16_t Dps422::getContResults(int32_t *tempBuffer,
							   uint8_t &tempCount,
							   int32_t *prsBuffer,
							   uint8_t &prsCount)
{
}

int16_t Dps422::setInterruptPolarity(uint8_t polarity)
{
}

int16_t Dps422::setInterruptSources(bool fifoFull, bool tempReady, bool prsReady)
{
}

int16_t Dps422::getIntStatusFifoFull(void)
{
}
int16_t Dps422::getIntStatusTempReady(void)
{
}

int16_t Dps422::getIntStatusPrsReady(void)
{
}

void Dps422::init(void)
{
	readcoeffs();
	standby();
	configTemp(DPS310__TEMP_STD_MR, DPS310__TEMP_STD_OSR);
	configPressure(DPS310__PRS_STD_MR, DPS310__PRS_STD_OSR);
}

int16_t Dps422::getFIFOvalue(int32_t *value)
{
}

int16_t Dps422::setOpMode(uint8_t opMode)
{
	if (writeByteBitfield(opMode, registers[MSR_CTRL]) == -1)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_opMode = (DpsClass::Mode)opMode;
	return DPS__SUCCEEDED;
}

int16_t Dps422::configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	int16_t ret = writeByteBitfield(tempMr, registers[TEMP_MR]);
	ret = writeByteBitfield(tempOsr, registers[TEMP_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_tempMr = tempMr;
	m_tempOsr = tempOsr;
}

int16_t Dps422::configPressure(uint8_t prsMr, uint8_t prsOsr)
{
	int16_t ret = writeByteBitfield(prsMr, registers[PRS_MR]);
	ret = writeByteBitfield(prsOsr, registers[PRS_OSR]);

	//abort immediately on fail
	if (ret != DPS__SUCCEEDED)
	{
		return DPS__FAIL_UNKNOWN;
	}
	m_prsMr = prsMr;
	m_prsOsr = prsOsr;
}

int16_t Dps422::readcoeffs(void)
{
	uint8_t buffer_temp[3];
	uint8_t buffer_prs[20];
	readBlock(registerBlocks[COEF_TEMP], buffer_temp);
	readBlock(registerBlocks[COEF_PRS], buffer_prs);

	t_gain = buffer_temp[0];								 // 8 bits
	t_dVbe = (buffer_temp[1] & 0xFE) >> 1;					 // 7 bits
	t_Vbe = ((buffer_temp[1] & 0x01) << 8) | buffer_temp[2]; // 9 bits

	// c00, c01, c02, c10 : 20 bits
	// c11, c12: 17 bits
	// c20: 15 bits; c21: 14 bits; c30 12 bits
	m_c00 = ((uint32_t)buffer_prs[0] << 12) | ((uint32_t)buffer_prs[1] << 4) | (((uint32_t)buffer_prs[2] & 0xF0) >> 4);
	m_c10 = ((uint32_t)(buffer_prs[2] & 0x0F) << 16) | ((uint32_t)buffer_prs[3] << 8) | (uint32_t)buffer_prs[4];
	m_c01 = ((uint32_t)buffer_prs[5] << 12) | ((uint32_t)buffer_prs[6] << 4) | (((uint32_t)buffer_prs[7] & 0xF0) >> 4);
	m_c02 = ((uint32_t)(buffer_prs[7] & 0x0F) << 16) | ((uint32_t)buffer_prs[8] << 8) | (uint32_t)buffer_prs[9];
	m_c20 = ((uint32_t)(buffer_prs[10] & 0xEF) << 10) | (uint32_t)buffer_prs[11];
	m_c30 = ((uint32_t)(buffer_prs[12] & 0x0F) << 8) | (uint32_t)buffer_prs[13];
	m_c11 = ((uint32_t)buffer_prs[14] << 9) | ((uint32_t)buffer_prs[15] << 1) | (((uint32_t)buffer_prs[16] & 0x80) >> 7);
	m_c12 = (((uint32_t)buffer_prs[16] & 0xEF) << 10) | ((uint32_t)buffer_prs[17] << 2) | (((uint32_t)buffer_prs[18] & 0xC0) >> 6);
	m_c21 = (((uint32_t)buffer_prs[18] & 0x3F) << 8) | ((uint32_t)buffer_prs[19]);

	getTwosComplement(&m_c00, 20);
	getTwosComplement(&m_c01, 20);
	getTwosComplement(&m_c02, 20);
	getTwosComplement(&m_c10, 20);
	getTwosComplement(&m_c11, 17);
	getTwosComplement(&m_c12, 17);
	getTwosComplement(&m_c20, 15);
	getTwosComplement(&m_c21, 14);
	getTwosComplement(&m_c30, 12);

	return DPS__SUCCEEDED;
}

int16_t Dps422::enableFIFO()
{
	return writeByteBitfield(1U, registers[FIFO_EN]);
}
int16_t Dps422::disableFIFO()
{
	int16_t ret = writeByteBitfield(1U, registers[FIFO_FL]);
	if (ret < 0)
	{
		return ret;
	}
	return writeByteBitfield(0U, registers[FIFO_EN]);
}