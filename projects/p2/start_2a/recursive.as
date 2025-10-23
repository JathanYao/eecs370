        lw      0       1       n
        lw	0	2	r
        lw      0       4       Faddr   // load function address
        jalr    4       7               // call function
        halt
comb    sw      5       7       Stack   //save return address to Stack  
        lw      0       6       pos1    //load reg6 = 1
        add     5       6       5       //sp++
        sw      5       1       Stack   //save n to Stack
        add     5       6       5       //sp++
        sw      5       2       Stack   //save r to Stack
        add     5       6       5       //sp++
        beq     2       0       base    //if r == 0 go to base case
        beq     1       2       base    //if n == r go to base case
        lw      0       6       neg1    //load reg6 = -1
        add     1       6       1       //n--;
        add     2       6       2       //r--;
        jalr    4       7               //call comb(n-1, r-1)
        lw      0       6       neg1    //load reg6 = -1
        add     5       6       5       //sp--
        lw      5       2       Stack   //restore r
        add     5       6       5       //sp--
        lw      5       1       Stack   //restore n
        add     1       6       1       //n--;
        jalr    4       7               //call comb(n-1, r)
        lw      0       6       neg1    //reg6 = -1
        add     5       6       5       //sp--
        lw      5       7       Stack   //restore return address
        jalr    7       6               //jump back to main
base    lw      0       6       neg1    //reg6 = -1
        add     5       6       5       //sp--
        add     5       6       5       //sp-- //pop r
        add     5       6       5       //sp-- //pop n
        lw      0       6       pos1    //reg6 = 1
        add     6       3       3       //reg3++;
        jalr    7       6               //jump back
n       .fill   2
r       .fill   0
pos1    .fill   1
neg1    .fill   -1
Faddr   .fill   comb
