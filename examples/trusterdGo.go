package main

// #cgo LDFLAGS: -L.. -ltrusterd
/*
 #include <stdlib.h>
 #include <stdio.h>


 typedef int (*FUNCPTR)(char *script);
 int myCallback(char*);

 int boot(char*, FUNCPTR);

 static void testGoGo() {
 boot("puts 1+2",(FUNCPTR)myCallback);
 printf("%s","hello, Go!\n");
 fflush(stdout);
 }
*/
import "C"

import . "fmt"

//export myCallback
func myCallback(s *C.char) C.int {
    //C.printf("%s\n",s)
    Println(C.GoString(s))
    return 0
}

func main() {
    C.testGoGo()
}
