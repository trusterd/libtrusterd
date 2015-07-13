use std::env;

macro_rules! t {
    ($e:expr) => (match $e {
        Ok(n) => n,
        Err(e) => fail(&format!("\n{} failed with {}\n", stringify!($e), e)),
    })
}

fn main() {
	let mut here = env::current_dir().unwrap();
	here.pop();
	here.pop();
	let libtr_dir = here;
	println!("cargo:rustc-link-search=native={}",format!("{}", libtr_dir.display()));
}
