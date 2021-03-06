/* Iff2Src conversion: Get.iff */

UWORD getdata[150] =
	{
	/* Plane 0 */
		0xFFA8,0x0000,
		0xE000,0x0000,
		0xEFFE,0x0000,
		0xE9FE,0x0000,
		0xABFE,0x0000,
		0x6FFE,0x0000,
		0xAFFE,0x0000,
		0x4FFE,0x0000,
		0xAFFE,0x0800,
		0x1FFE,0x1C00,
		0x2FFC,0x3A00,
		0x0000,0x4F00,
		0x3FFE,0x9780,
		0x057D,0x23C0,
		0x7FFA,0x47E0,
		0x70F6,0x8FF0,
		0x1757,0x1CF0,
		0x0703,0xBA60,
		0x0761,0xF640,
		0x07F8,0xECC0,
		0x07FF,0x7580,
		0x07FE,0x3B00,
		0x0778,0x1E00,
		0x0060,0x0C00,
		0x0000,0x0000,
	/* Plane 1 */
		0x0000,0x0000,
		0x1FFC,0x0000,
		0x1FF8,0x0000,
		0x1D08,0x0000,
		0x1808,0x0000,
		0x1C08,0x0000,
		0x1808,0x0000,
		0x1808,0x0000,
		0x1FF8,0x1800,
		0x0000,0x3800,
		0x0000,0x6000,
		0x7FFF,0xD200,
		0x4001,0xA900,
		0x4547,0x5580,
		0x0006,0xA9C0,
		0x0F09,0x50E0,
		0x0E86,0xA360,
		0x0EE3,0x4480,
		0x0EF9,0x8800,
		0x0FFE,0xD640,
		0x0FFE,0x6880,
		0x0FF8,0x3100,
		0x00E0,0x1A00,
		0x0080,0x0000,
		0x0080,0x0000,
	/* Plane 2 */
		0xFFA8,0x0578,
		0xE000,0x2AF8,
		0xE004,0x0158,
		0xE204,0x0BF8,
		0xA404,0x0578,
		0x6004,0x2AF8,
		0xA004,0x0158,
		0x4004,0x0BF8,
		0xA004,0x0578,
		0x1FFC,0x12F8,
		0x2000,0x3558,
		0x0000,0x68F8,
		0x3FFE,0xD478,
		0x3A81,0xA838,
		0x7FFB,0x5218,
		0x0000,0xA408,
		0x6628,0x4808,
		0x8608,0x1318,
		0x5664,0x2398,
		0xE7F8,0x0138,
		0x57FE,0x0278,
		0xF7F9,0xC4F8,
		0xD865,0xC1D8,
		0xFF1F,0xF3F8,
		0xFF7F,0xFFF8
	};


struct Image getim =
	{
		0,0,
		29,25,
		3,
		&getdata[0],
		0xff,0x0,
		NULL
	};
