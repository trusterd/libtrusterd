extern crate libc;
use libc::{c_int, c_char};
use std::ffi::CString;


extern { fn puts(s: *const libc::c_char); }

extern "C" fn callback(target: *const libc::c_char) {
    unsafe {
	puts(target);
    }
}

#[link(name = "trusterd")]
extern {
   fn boot(script: *const libc::c_char,
                        cb: extern fn(* const libc::c_char) ) -> c_int;
}

fn main() {
    // Create the object that will be referenced in the callback
    //let mut rust_object = Box::new(RustObject { a: 5 });
	let script = &b"p 'Hello, world!'"[..];
	let c_to_script = CString::new(script).unwrap();
    unsafe {
        boot(c_to_script.as_ptr(), callback);
    }
}
