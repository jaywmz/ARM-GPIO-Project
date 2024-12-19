 #include <stdio.h>                                         // pre-processor statements 
 #include <string.h>                                        // pre-processor statements 

 void reverse(char *a, int left, int right)                 // Reverse Function with parameters (char *a, int left, int right)
 {
    if(left < right)                                        // If-else statement to compare if left < right 
    {
        char tmp = a[left];                                 // a[left] is assigned to tmp variable. Left starts from 0, giving the first character of the string
        a[left] = a[right];                                 // The first character of the string is replaced by the last character  
        a[right] = tmp;                                     // The last character of the string is replaced by the first character 
        reverse(a, left+1, right-1);                        // Recursive function whereby the index of left is increase while right decreases
    }
 }

 int main()                                                 // Main function 
 {
    char str[] = "\nThis is the string to reverse\n";       // str = String variable 'This is the string to reverse'
    printf("%s",str);                                       // This prints out the string variable 
    reverse(str, 0, strlen(str)-1);                         // Reverse function with parameters; *a = str, left = 0, right = strlen(str)-1. strlen gives the length of string. 
    printf("%s",str);                                       // This prints out the string variable after the reverse function 
    return 0;                                               // Return 0
 }


