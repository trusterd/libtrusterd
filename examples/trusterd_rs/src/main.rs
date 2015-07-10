extern crate libc;
use libc::{c_int, c_char};
use std::ffi::CString;

extern { fn puts(s: *const libc::c_char); }

extern "C" fn callback(target: *const libc::c_char) -> c_int {
	println!("This is Rust.I'm called from mruby");
    unsafe {
	puts(target);
    }
	println!("Above line is script which executed by mruby.");
	return 0;
}

#[link(name = "trusterd")]
extern {
   fn boot(script: *const libc::c_char,
                        cb: extern fn(* const libc::c_char) -> c_int ) -> c_int;
   fn boot_from_file_path(filepath: *const c_char, cb: extern fn(* const c_char) -> c_int ) -> c_int;
}

fn main() {
	println!("Hello, this is Rust.");
    // Create the object that will be referenced in the callback
    //let mut rust_object = Box::new(RustObject { a: 5 });
	let script = &b"p 'Hello, world!'\nMyCall.my_exec('this is trusterd!')\n"[..];
	let c_to_script = CString::new(script).unwrap();
	let conf = &b"../test.conf.rb"[..];
	let c_to_conf = CString::new(conf).unwrap();
    unsafe {
        boot(c_to_script.as_ptr(), callback);
	boot_from_file_path(c_to_conf.as_ptr(), callback);
    }
}
