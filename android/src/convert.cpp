#include <string.h>
#include "convert.h"

#define MR(Y,U,V) (Y + (1.403)*(V-128))  
#define MG(Y,U,V) (Y - (0.344) * (U-128) - (0.714) * (V-128) )   
#define MB(Y,U,V) (Y + ((1.773) * (U-128))) 


void __attribute((noinline)) yv12_to_rgb565_neon(unsigned short *dst, const unsigned char *y, const unsigned char *u, const unsigned char *v, int n, int oddflag)
{
	static __attribute__((aligned(16))) uint16 acc_r[8] = {
		22840, 22840, 22840, 22840, 22840, 22840, 22840, 22840,
	};
	static __attribute__((aligned(16))) uint16 acc_g[8] = {
		17312, 17312, 17312, 17312, 17312, 17312, 17312, 17312,
	};
	static __attribute__((aligned(16))) uint16 acc_b[8] = {
		28832, 28832, 28832, 28832, 28832, 28832, 28832, 28832,
	};

	asm volatile (
		".fpu neon\n"
		".macro convert_macroblock size\n"

		".if \\size == 16\n"
		"pld [%[y], #64]\n"
		"pld [%[u], #64]\n"
		"pld [%[v], #64]\n"
		"vld1.8 {d1}, [%[y]]!\n"
		"vld1.8 {d3}, [%[y]]!\n"
		"vld1.8 {d0}, [%[u]]!\n"
		"vld1.8 {d2}, [%[v]]!\n"
		".elseif \\size == 8\n"
		"vld1.8 {d1}, [%[y]]!\n"
		"vld1.8 {d0[0]}, [%[u]]!\n"
		"vld1.8 {d0[1]}, [%[u]]!\n"
		"vld1.8 {d0[2]}, [%[u]]!\n"
		"vld1.8 {d0[3]}, [%[u]]!\n"
		"vld1.8 {d2[0]}, [%[v]]!\n"
		"vld1.8 {d2[1]}, [%[v]]!\n"
		"vld1.8 {d2[2]}, [%[v]]!\n"
		"vld1.8 {d2[3]}, [%[v]]!\n"
		".elseif \\size == 4\n"
		"vld1.8 {d1[0]}, [%[y]]!\n"
		"vld1.8 {d1[1]}, [%[y]]!\n"
		"vld1.8 {d1[2]}, [%[y]]!\n"
		"vld1.8 {d1[3]}, [%[y]]!\n"
		"vld1.8 {d0[0]}, [%[u]]!\n"
		"vld1.8 {d0[1]}, [%[u]]!\n"
		"vld1.8 {d2[0]}, [%[v]]!\n"
		"vld1.8 {d2[1]}, [%[v]]!\n"
		".elseif \\size == 2\n"
		"vld1.8 {d1[0]}, [%[y]]!\n"
		"vld1.8 {d1[1]}, [%[y]]!\n"
		"vld1.8 {d0[0]}, [%[u]]!\n"
		"vld1.8 {d2[0]}, [%[v]]!\n"
		".elseif \\size == 1\n"
		"vld1.8 {d1[0]}, [%[y]]!\n"
		"vld1.8 {d0[0]}, [%[u]]!\n"
		"vld1.8 {d2[0]}, [%[v]]!\n"
		".else\n"
		".error unsupported macroblock size\n"
		".endif\n"



		"vuzp.8      d1, d3\n"                       

		"vqadd.u8    q0, q0, q4\n"
		"vqadd.u8    q1, q1, q4\n"
		"vqsub.u8    q0, q0, q5\n"
		"vqsub.u8    q1, q1, q5\n"

		"vshr.u8     d4, d2, #1\n"                   

		"vmull.u8    q8, d1, d27\n"                  
		"vmull.u8    q9, d3, d27\n"                  

		"vld1.16     {d20, d21}, [%[acc_r], :128]\n" 
		"vsubw.u8    q10, q10, d4\n"                 
		"vmlsl.u8    q10, d2, d28\n"                 
		"vld1.16     {d22, d23}, [%[acc_g], :128]\n" 
		"vmlsl.u8    q11, d2, d30\n"                 
		"vmlsl.u8    q11, d0, d29\n"                 
		"vld1.16     {d24, d25}, [%[acc_b], :128]\n" 
		"vmlsl.u8    q12, d0, d30\n"                 
		"vmlsl.u8    q12, d0, d31\n"                 

		"vhsub.s16   q3, q8, q10\n"                  
		"vhsub.s16   q10, q9, q10\n"                 
		"vqshrun.s16 d0, q3, #6\n"                   
		"vqshrun.s16 d3, q10, #6\n"                  

		"vhadd.s16   q3, q8, q11\n"                  
		"vhadd.s16   q11, q9, q11\n"                 
		"vqshrun.s16 d1, q3, #6\n"                   
		"vqshrun.s16 d4, q11, #6\n"                  

		"vhsub.s16   q3, q8, q12\n"                  
		"vhsub.s16   q12, q9, q12\n"                 
		"vqshrun.s16 d2, q3, #6\n"                   
		"vqshrun.s16 d5, q12, #6\n"                  

		"vzip.8      d0, d3\n"                       
		"vzip.8      d1, d4\n"                       
		"vzip.8      d2, d5\n"                       

		"vshll.u8    q3, d0, #8\n\t"
		"vshll.u8    q8, d1, #8\n\t"
		"vshll.u8    q9, d2, #8\n\t"
		"vsri.u16    q3, q8, #5\t\n"
		"vsri.u16    q3, q9, #11\t\n"

		".if \\size == 16\n"
		"    vst1.16 {d6, d7}, [%[dst]]!\n"
		"    vshll.u8    q3, d3, #8\n\t"
		"    vshll.u8    q8, d4, #8\n\t"
		"    vshll.u8    q9, d5, #8\n\t"
		"    vsri.u16    q3, q8, #5\t\n"
		"    vsri.u16    q3, q9, #11\t\n"
		"    vst1.16 {d6, d7}, [%[dst]]!\n"
		".elseif \\size == 8\n"
		"    vst1.16 {d6, d7}, [%[dst]]!\n"
		".elseif \\size == 4\n"
		"    vst1.16 {d6}, [%[dst]]!\n"
		".elseif \\size == 2\n"
		"    vst1.16 {d6[0]}, [%[dst]]!\n"
		"    vst1.16 {d6[1]}, [%[dst]]!\n"
		".elseif \\size == 1\n"
		"    vst1.16 {d6[0]}, [%[dst]]!\n"
		".endif\n"
		".endm\n"

		"vmov.u8     d8, #15\n" 
		"vmov.u8     d9, #20\n" 
		"vmov.u8     d10, #31\n" 
		"vmov.u8     d11, #36\n" 

		"vmov.u8     d26, #16\n"
		"vmov.u8     d27, #149\n"
		"vmov.u8     d28, #204\n"
		"vmov.u8     d29, #50\n"
		"vmov.u8     d30, #104\n"
		"vmov.u8     d31, #154\n"

		"cmp         %[oddflag], #0\n"
		"beq         1f\n"
		"convert_macroblock 1\n"
		"sub         %[n], %[n], #1\n"
		"1:\n"
		"subs        %[n], %[n], #16\n"
		"blt         2f\n"
		"1:\n"
		"convert_macroblock 16\n"
		"subs        %[n], %[n], #16\n"
		"bge         1b\n"
		"2:\n"
		"tst         %[n], #8\n"
		"beq         3f\n"
		"convert_macroblock 8\n"
		"3:\n"
		"tst         %[n], #4\n"
		"beq         4f\n"
		"convert_macroblock 4\n"
		"4:\n"
		"tst         %[n], #2\n"
		"beq         5f\n"
		"convert_macroblock 2\n"
		"5:\n"
		"tst         %[n], #1\n"
		"beq         6f\n"
		"convert_macroblock 1\n"
		"6:\n"
		".purgem convert_macroblock\n"
		: [y] "+&r" (y), [u] "+&r" (u), [v] "+&r" (v), [dst] "+&r" (dst), [n] "+&r" (n)
		: [acc_r] "r" (&acc_r[0]), [acc_g] "r" (&acc_g[0]), [acc_b] "r" (&acc_b[0]),
		[oddflag] "r" (oddflag)
		: "cc", "memory",
		"d0",  "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",
		"d8",  "d9",  "d10", "d11", 
		"d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
		"d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
		);
}

void ConvertYCbCrToRGB565(const uint8* y_buf,
						  const uint8* u_buf,
						  const uint8* v_buf,
						  uint8* rgb_buf,
						  int pic_x,
						  int pic_y,
						  int pic_width,
						  int pic_height,
						  int y_pitch,
						  int uv_pitch,
						  int rgb_pitch,
						  int yuv_type)
{
	int rgbstride = pic_width;
	int i;
	for (i = 0; i < pic_height; i++) {
		yv12_to_rgb565_neon((uint16*)rgb_buf + rgbstride * i,
			y_buf + y_pitch * i,
			u_buf + uv_pitch * (i / 2),
			v_buf + uv_pitch * (i / 2),
			y_pitch,
			0);
	}
}

void YUV420_C_RGB( char* pYUV, unsigned char* pRGB, int height, int width)  
{  
    char* pY = pYUV;  
    char* pU = pYUV+height*width;  
    char* pV = pU+(height*width/4);  
  
  
    unsigned char* pBGR = NULL;  
    unsigned char R = 0;  
    unsigned char G = 0;  
    unsigned char B = 0;  
    char Y = 0;  
    char U = 0;  
    char V = 0;  
    double tmp = 0;  
    for ( int i = 0; i < height; ++i )  
    {  
        for ( int j = 0; j < width; ++j )  
        {  
            pBGR = pRGB+ i*width*3+j*3;  
  
            Y = *(pY+i*width+j);  
            U = *pU;  
            V = *pV;  
  
            //B  
            tmp = MB(Y, U, V);  
            //B = (tmp > 255) ? 255 : (char)tmp;  
            //B = (B<0) ? 0 : B;  
            B = (unsigned char)tmp;  
            //G  
            tmp = MG(Y, U, V);  
            //G = (tmp > 255) ? 255 : (char)tmp;  
           // G = (G<0) ? 0 : G;  
            G = (unsigned char)tmp;  
            //R  
            tmp = MR(Y, U, V);  
            //R = (tmp > 255) ? 255 : (char)tmp;  
            //R = (R<0) ? 0 : R;  
            R = (unsigned char)tmp;  
  
  
            *pBGR     = R;              
            *(pBGR+1) = G;          
            *(pBGR+2) = B;  
          
  
            if ( i%2 == 0 && j%2 == 0)  
            {  
                *pU++;  
                //*pV++;  
            }  
            else  
            {  
                if ( j%2 == 0 )  
                {  
                    *pV++ ;  
                }  
            }  
        }  
      
    }  
}