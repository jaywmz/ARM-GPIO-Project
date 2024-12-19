 .data                                                  // Declare static data
    str: .string "\nThis is the string to reverse\n"    // Declare string variable 

 .text                                                  // Code section 
    .global reverse                                     // Declare reverse function as a global function 

    .global main                                        // Declare reverse function as a global function


 reverse:                                               // Reverse function 
    stp x29, x30, [sp, #-16]!                           // This instruction pushes the frame pointer (x29) and link register (x30) onto the stack, saving the context for the function call. x30 contains the address to return to after execution of function.


    cmp x1, x2                                          // Compare x1 and x2. x1 = 0, x2 = length of string - 1 
    bge endif                                           // branch to endif if x1 >= x2  
    ldrb w3, [x0, x1]                                   // Load the character in str[0] to w3. w3 = str[0]. w3 contains the ascii hexadecimal of the character in str[0] 
    ldrb w4, [x0, x2]                                   // Load the character in str[length-1] to w4. w4 = str[length-1]. w4 contains the ascii hexadecimal of the character in str[length-1]
    strb w4, [x0, x1]                                   // store w4 into str[0]. The last character of the string is now the first character  
    strb w3, [x0, x2]                                   // store w3 into str[30]. The first character of the string is now the last character 


    add w1, w1, #1                                      // w1 = 0+1. This refers to the next character index of the string 
    sub w2, w2, #1                                      // w2 = (length-1)-1. This refers to the next character index of the string from the back 
    bl reverse                                          // Recursive function. Function ends when x1 >= x2. This means that all characters of the string has been swapped 



    endif: ldp x29, x30, [sp], #16                      // POP FP and LR

    ret                                                 // Return updated str 


 main: 
    stp x29, x30, [sp, #-16]!                           // This instruction pushes the frame pointer (x29) and link register (x30) onto the stack, saving the context for the function call. x30 contains the address to return to after execution of function.


    adr x0, str                                         // storing address of str into register x0. x0 shows 64 bit value whereas w0 shows 32 bit value of the address 
    bl printf                                           // branch link to printf function. This calls the printf function to print the string out. x0 and w0 are changed in values to the length of characters the string has. 

    
    
   // This chunk of code defines the parameters for reverse function, (str, x1, x2) and calls the reverse function. x0, x1 and x2 are inbuilt parameters for any function where x0 is the first parameter 

    adr x0, str                                         // Store address of str in x0 since value of x0 changed because of bl printf. 
    bl strlen                                           // execute strlen to calculate length in x0. value of x0 and w0 changed to length of string.   
    add w0, w0, #-1                                     // w0 = w0 - #1. w0 = Length of string - 1  
    mov w2, w0                                          // Move value of w0 into w2. This will also change x2 to the value of w2 in 64 bits 
    mov w1, #0                                          // Move value 0 to w1. This also changes x1 to 0
    adr x0, str                                         // Store address of str in x0 since value of x0 changed because of bl strlen. 
    bl reverse                                          // Calling reverse function while having x0 = address of str, x1 = 0, x2 = Length of string - 1 



    adr x0, str                                         // Store address of str in x0 once again
    bl printf                                           // branch link to printf function. This calls the printf function to print the string out.  


    // This chunk of code returns 0 when executing echo $? command 

    mov w0, #0                                          // Move 0 to w0. w0 = 0
    ldp x29, x30, [sp], #16                             // POP FP and LR 
    ret                                                 // Return 0 


