#[path = "../src/main.rs"]
mod src;

#[cfg(test)]
mod tests {
    use crate::src::secrettoken::gettoken;
    use crate::src::disco;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use serenity;
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let outcome = create_bot_test();
        Ok(())
    }

    async fn create_bot_test() -> serenity::Client {
        let token = gettoken();
        println!("creating bot...");
        let client = disco::create_bot(token).await;
        client
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = create_bot_test().await;
        let shard_man = client.shard_manager.clone();
        // Now shut it down

        client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    #[tokio::test]
    async fn can_register_event_hooks() -> Result<(), Error> {
        Ok(())
    }

    #[tokio::test]
    async fn can_receive_a_message() -> Result<(), Error> {
        Ok(())
    }

    #[tokio::test]
    async fn can_send_a_message() -> Result<(), Error> {
        Ok(())
    }
}
