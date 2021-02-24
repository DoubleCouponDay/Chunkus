
#[cfg(test)]
mod tests {
    use vecbot::{
        core::set_algorithm,
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
}