
char src[] = 

//0:        1         2         3         4         5         6
// 1234567890123456789012345678901234567890123456789012345678901234
  "                                                                " //0 64
  "                                                                " //1 128
  "                                                                " //2 192
  "                                                                " //3  256
  "                                                                " //4 320
  "                                                                " //5 384
  "                                                                " //6 448
  "                                                                " //7  512
  "                                                                " //8 576
  "                                                                " //9 640
  "                                                                " //A 704
  "                                                                " //B  768
  "                                                                " //C 832
  "                                                                " //D 896
  "                                                                " //E 960
  "                                                                " //F 1024

//1:        1         2         3         4         5         6
// 1234567890123456789012345678901234567890123456789012345678901234
  "                                                                " //0
  "                                                                " //1
  "                                                                " //2
  "                                                                " //3
  "                                                                " //4
  "                                                                " //5
  "                                                                " //6
  "                                                                " //7
  "                                                                " //8
  "                                                                " //9
  "                                                                " //A
  "                                                                " //B
  "                                                                " //C
  "                                                                " //D
  "                                                                " //E
  "                                                                " //F

//2:        1         2         3         4         5         6
// 1234567890123456789012345678901234567890123456789012345678901234
  "                                                                " //0
  "                                                                " //1
  "                                                                " //2
  "                                                                " //3
  "                                                                " //4
  "                                                                " //5
  "                                                                " //6
  "                                                                " //7
  "                                                                " //8
  "                                                                " //9
  "                                                                " //A
  "                                                                " //B
  "                                                                " //C
  "                                                                " //D
  "                                                                " //E
  "                                                                " //F

  ;
p = mem + 0xC000;
memcpy( p, src, sizeof src );
