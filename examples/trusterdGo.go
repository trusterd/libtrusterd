package main

// #cgo LDFLAGS: -L.. -ltrusterd
/*
 #include <stdlib.h>
 #include <stdio.h>


 typedef int (*FUNCPTR)(char *script);
 int myCallback(char*);

 int boot(char*, FUNCPTR);

 static void testGoGo(char *scpt) {
 //boot("puts 1+2",(FUNCPTR)myCallback);
 boot(scpt,(FUNCPTR)myCallback);
 printf("%s","hello, Go!\n");
 fflush(stdout);
 }
*/
import "C"

import (
	"fmt"
	"io/ioutil"
)

//export myCallback
func myCallback(s *C.char) C.int {
	//C.printf("%s\n",s)
	fmt.Println(C.GoString(s))
	return 0
}

func main() {
	rbscript, err := ioutil.ReadFile("../trusterd.conf.rb")
	if err != nil {
		fmt.Println(rbscript, err)
		return
	}
	C.testGoGo(C.CString(string(rbscript)))
}
