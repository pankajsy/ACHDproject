/* RC5REF.C -- Reference implementation of RC5-32/12/16 in C.        */
/* Copyright (C) 1995 RSA Data Security, Inc.                        */
#include <stdio.h>
#include <time.h>
/* For windows use the following line.. for linux based (MAC) use --> typedef unsigned int WORD; */
//typedef unsigned long int WORD; /* Should be 32-bit = 4 bytes        */
typedef unsigned int WORD;
#define w        32             /* word size in bits                 */
#define r        12             /* number of rounds                  */
#define b        16             /* number of bytes in key            */
#define c         4             /* number  words in key = ceil(8*b/w)*/
#define t        26             /* size of table S = 2*(r+1) words   */
WORD S[t];                      /* expanded key table                */
WORD P = 0xb7e15163, Q = 0x9e3779b9;  /* magic constants             */
/* Rotation operators. x must be unsigned, to get logical right shift*/
#define ROTL(x,y) (((x)<<(y&(w-1))) | ((x)>>(w-(y&(w-1)))))
#define ROTR(x,y) (((x)>>(y&(w-1))) | ((x)<<(w-(y&(w-1)))))

void RC5_ENCRYPT(WORD *pt, WORD *ct) /* 2 WORD input pt/output ct    */
{ WORD i, A=pt[0]+S[0], B=pt[1]+S[1];
  for (i=1; i<=r; i++)
    { A = ROTL(A^B,B)+S[2*i];
      B = ROTL(B^A,A)+S[2*i+1];
    }
  ct[0] = A; ct[1] = B;
}

void RC5_DECRYPT(WORD *ct, WORD *pt) /* 2 WORD input ct/output pt    */
{ WORD i, B=ct[1], A=ct[0];
  for (i=r; i>0; i--)
    { B = ROTR(B-S[2*i+1],A)^A;
      A = ROTR(A-S[2*i],B)^B;
    }
  pt[1] = B-S[1]; pt[0] = A-S[0];
}

void RC5_SETUP(unsigned char *K) /* secret input key K[0...b-1]      */
{  WORD i, j, k, u=w/8, A, B, L[c];
   /* Initialize L, then S, then mix key into S */
   for (i=b-1,L[c-1]=0; i!=-1; i--) L[i/u] = (L[i/u]<<8)+K[i];
   for (S[0]=P,i=1; i<t; i++) S[i] = S[i-1]+Q;
   for (A=B=i=j=k=0; k<3*t; k++,i=(i+1)%t,j=(j+1)%c)   /* 3*t > 3*c */
     {
       printf("L[%d] = %x\n S[%d] = %x\n", j, L[j], i, S[i]);
       A = S[i] = ROTL(S[i]+(A+B),3);
       B = L[j] = ROTL(L[j]+(A+B),(A+B));

     }
}

void main()
{ WORD i, j, pt1[2], pt2[2], ct[2] = {0,0};
  unsigned char key[b]; //= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  time_t t0, t1;
  if (sizeof(WORD)!=4)
    printf("RC5 error: WORD has %d bytes.\n",sizeof(WORD));
  printf("RC5-32/12/16 examples:\n");
  for (i=1;i<6;i++)
    { /* Initialize pt1 and key pseudorandomly based on previous ct */
      pt1[0]=ct[0]; pt1[1]=ct[1];
      for (j=0;j<b;j++) key[j] = ct[0]%(255-j);
      /* Setup, encrypt, and decrypt */
      RC5_SETUP(key);
      RC5_ENCRYPT(pt1,ct);
      RC5_DECRYPT(ct,pt2);
      /* Print out results, checking for decryption failure */
      printf("\n%d. key = ",i);
      for (j=0; j<b; j++) printf("%.2X ",key[j]);
      printf("\n   plaintext %.8lX %.8lX  --->  ciphertext %.8lX %.8lX  \n",
             pt1[0], pt1[1], ct[0], ct[1]);
      if (pt1[0]!=pt2[0] || pt1[1]!=pt2[1])
        printf("Decryption Error!");
    }
  time (&t0);
  for (i=1;i<100000;i++)
    RC5_ENCRYPT(ct,ct);
  time (&t1);
  printf ("\n   Time_t for 100000 blocks:  %ld \n", t1-t0);
}