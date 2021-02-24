
#[cfg(test)]
mod tests {
    use vecbot::{
        core::{set_algorithm, do_vectorize},
        options::ParsedOptions,
        constants::FfiResult
    };
    use tokio;

    #[tokio::test]
    async fn algorithm_can_be_changed() {
        let algo_result = set_algorithm("bobsweep");
        let mut midstep: String = algo_result.to_string();
        let tripstep: &str = midstep.as_mut_str();

        match algo_result {
            FfiResult::SuccessCode => println!("{}", tripstep), 
            _ => panic!("{}", tripstep) 
        };
        ()
    }

    #[tokio::test]
    async fn invoke() {
        let input_file = String::from("test.png");
        let output_file = String::from("output.png");

        let algo_result = do_vectorize(&input_file, &output_file, ParsedOptions{chunksize: String::from("1"), threshold: String::from("5")});

        if algo_result != FfiResult::SuccessCode
        {
            panic!("Algorithm failed");
        }


    }
}