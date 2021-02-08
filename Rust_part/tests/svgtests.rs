pub static TEST_IMAGE: &'static str = "https://i.imgur.com/j71oECe.png";

#[cfg(test)]
mod tests
{
    use vecbot::secrettoken::gettoken;
    use vecbot::bot::{create_vec_bot};

    #[tokio::test]
    async fn works_endtoend() {
        let client = create_vec_bot(gettoken()).await;
        let shardmanager = client.shard_manager.clone();
    }
}
