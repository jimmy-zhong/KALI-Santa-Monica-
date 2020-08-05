
uint32_t dwPolynomial = 0x04c11db7;

uint32_t CRC_cal_proc(uint32_t *ptr, int len)
{
	uint32_t xbit;
	uint32_t data;
	uint32_t CRCn = 0xFFFFFFFF;    // init

	while (len--)
	{
		xbit = 1 << 31;

		data = *ptr++;
		for (int bits = 0; bits < 32; bits++)
		{
			if (CRCn & 0x80000000)
			{
				CRCn <<= 1;
				CRCn ^= dwPolynomial;
			}
			else
			{
				CRCn <<= 1;
			}
			if (data & xbit)
			{
				CRCn ^= dwPolynomial;
			}

			xbit >>= 1;
		}
	}
	return CRCn;
}
