
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BIQUADS_H__
#define __BIQUADS_H__


#include <stdint.h>

#ifndef NULL
#define NULL  (0U)
#endif

#ifndef FALSE
#define FALSE (0U)
#endif

#ifndef TRUE
#define TRUE  (1U)
#endif

#define pi()								acos(-1.0)
#define ONE_8_24						0x01000000
#define ONE_0_24						0x00800000


#define FIR_LPI_ST					0
#define FIR_LPII_ADI				1
#define FIR_HPI_ST					2
#define FIR_HPII_ADI				3
#define FIR_PEQ_1						4
#define FIR_LSHELF					5
#define FIR_HSHELF					6
#define FIR_NOTCH						7
#define FIR_AllPass_I				8
#define FIR_AllPass_II			9
#define FIR_LPI_ADI					10
#define FIR_HPI_ADI					11
#define FIR_LPII_ST					12
#define FIR_HPII_ST					13
#define FIR_PEQ_ST					14
#define FIR_FLAT						15
#define FIR_LAST						FIR_FLAT

#define FIR_LPI							FIR_LPI_ST
#define FIR_HPI							FIR_HPI_ST
#define FIR_LPII						FIR_LPII_ADI
#define FIR_HPII						FIR_HPII_ADI
#define FIR_PEQ							FIR_PEQ_1


#define PRESET_EQ1					0
#define PRESET_EQ2					1
#define PRESET_EQ3					2
#define PRESET_EQ4					3
#define PRESET_EQ5					4
#define PRESET_EQ6					5
#define PRESET_EQ7					6
#define PRESET_EQ8					7
#define USER_EQ1						8
#define USER_EQ2						9
#define USER_EQ3						10
#define USER_EQ4						11
#define USER_EQ5						12
#define USER_EQ6						13
#define USER_EQ7						14
#define USER_EQ8						15

#define EQ_BAND1						0
#define EQ_BAND2						1
#define EQ_BAND3						2
#define EQ_BAND4						3
#define EQ_BAND5						4
#define EQ_BAND6						5
#define EQ_BAND7						6
#define EQ_BAND8						7


typedef struct {
	int32_t b2;						// FORMAT 8.24
	int32_t b1;						// FORMAT 8.24
	int32_t b0;						// FORMAT 8.24
	int32_t a2;						// FORMAT 8.24
	int32_t a1;						// FORMAT 8.24
} BQ_TYPE;


typedef struct {
	uint8_t En;					// 1: BQ settings work; 0: Flat (BQ neglected)
	uint8_t Type;				//
	uint16_t Freq;			// 100Hz ~ 40KHz, 3 significant figures
	uint16_t Qfx100;		// 0.10 ¨C 50.00; 0.01 resolution (Qf/100)
	int16_t Gain;				// -24dB - +18dB; 0.05dB resolution (Gain/100)
//	uint16_t DapAddr;
//	BQ_TYPE BQ;
} EQ_STRUCT;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LPFI_BIQUADS(double Freq0, double QFactor);
void Calculate_biquads(uint8_t EQ_NO, uint8_t BD_NO, uint8_t EqType, double Fc, double Qf, double Gain);
void Load_User_Biquads(void);


#endif /* __MISC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
