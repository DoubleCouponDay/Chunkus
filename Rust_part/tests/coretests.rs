
#[cfg(test)]
mod tests {
    use vecbot::{
        core::{do_vectorize},
        options::ParsedOptions,
        constants::FfiResult
    };
    use tokio;

    #[tokio::test]
    async fn invoke() {
        let input_file = String::from("test.png");

        let input = ParsedOptions{
            chunksize: String::from("1"), thresholds: String::from("5"),
            numcolours: String::from("256"), 
            shouldcrash: String::from("false")
        };
        let algo_result = do_vectorize(&input_file, input);

        if algo_result != FfiResult::SuccessCode
        {
            panic!("Algorithm failed");
        }
    }
}