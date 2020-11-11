#include "crc32.h"
static unsigned int u32CRC32[256];
//初始化表,在单片机等RAM较小的系�?可以在源码中把tab构造为const数组
static void init_table() {
   for (int i = 0; i < 256; i++) {
      unsigned int u32crc = i;
      for (int j = 0; j < 8; j++) {
         if (u32crc & 1) {
            u32crc = (u32crc >> 1) ^ 0xEDB88320;
         }
         else {
            u32crc = u32crc >> 1;
         }
      }
      u32CRC32[i] = u32crc;
   }
}

//crc32实现函数
unsigned int u32crc32(const unsigned char* buf, int len) {
   static unsigned char init = 0;
   unsigned int ret = 0xFFFFFFFF;
   if (!init) {
      init_table();
      init = 1;
   }
   for (int i = 0; i < len; i++) {
      ret = u32CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);
   }
   ret = ~ret;
   return ret;
}

#if 0
void Process_crc32_test()
{
	unsigned char DispStr[20];
	unsigned char test_crc_data[]={0,1,2,3,4,"",6,7,8,9,0};
	static unsigned char i=0;
	unsigned u32 crc_result;
	{
		i=1;
		crc_result=crc32(test_crc_data,5);
		convert_num_to_string(crc_result,DispStr,8);
		display(0,0,DispStr,20);	
		NumberInStr("you can see",15,4,2,DispStr);
		display(2,0,"you look here",20);	
		display(1,0,DispStr,20);	
		while(1);
	}
}
#endif

