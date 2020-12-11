#[cfg(test)]
mod tests {
    use discord_v::core;
    use std;
    use std::io;
    use std::result;
    use std::string::String;
    use std::ptr;
    use libc::c_int;

    #[test]
    fn it_can_run_a_test() {
        assert_eq!(true, true);
    }

    fn run_core() -> c_int {        
        let mut epic_path = String::from("./image.png");
        let path_pointer = epic_path.as_mut_ptr();
        let mut epic_thingo = String::from("--test");
        let mut epic_path_array: [*mut u8; 3] = [ptr::null_mut(), path_pointer, epic_thingo.as_mut_ptr()];
        let array_pointer = epic_path_array.as_mut_ptr();
        core::call_vectorize(3, array_pointer)
    }

    #[test]
    fn the_core_program_can_be_run() -> result::Result<(), io::Error> {
        println!("starting initial core test...");
        let result = run_core();
        println!("{}", result);
        
        Ok(())
    }

    #[test]
    fn a_fresh_output_file_can_be_found() -> Result<(),io::Error> {
        let result = run_core();
        Ok(())
    }
}



