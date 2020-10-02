#[path = "../src/main.rs"]
mod src;

#[cfg(test)]
mod tests {
    use crate::src::secrettoken::gettoken;
    use crate::src::bot;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use tokio::timer::Delay;
    use serenity;
    use serenity::{
        http::Http,
        model::{
            id::ChannelId
        }
    };
    use std::sync::Arc;
    use std::time::{Duration, Instant};
    use serenity::client::{Client, Context, EventHandler};
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let outcome = createbot();
        Ok(())
    }

    async fn createbot<T>(token: &str) -> serenity::Client where T: EventHandler {
        println!("creating bot...");
        let client = bot::create_bot_with_handle::<T>(token).await;
        client
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = createbot().await;
        let shard_man = client.shard_manager.clone();
        // Now shut it down

        client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    fn getchannelid() -> ChannelId {
        ChannelId(418671857676451841)
    }

    struct receivemessagehandler;

    #[async_trait]
    impl EventHandler for receivemessagehandler
    {
        fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT
            {
                println!("Found test message");
                MESSAGE_COUNT += 1;
            }
        }
    }

    static MESSAGE_CONTENT: str = "testicular boi";
    static mut MESSAGE_COUNT: i32 = 0;

    #[tokio::test]
    async fn can_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot::<receivemessagehandler>(token1).await;
        let shard_man = client.shard_manager.clone();
        client.start();
                
        // let token2 = gettestbotstoken();
        // let secondclient = createsecondbot::<Handler>(token2).await;
        // let secondshard = secondclient.shard_manager.clone();
        // secondclient.start();
        
        println!("check whether messages can be sent between them...");
        let channelid = getchannelid();
        let http = Http::new_with_token(&token1);

        //send a random number as message
        let message = channelid.send_message::<Http>(&http, |m| {
            m.content(MESSAGE_CONTENT);
            m.tts(false);
            m
        });

        tokio::timer::Delay::new(Duration::new(10, 0)).await;

        match MESSAGE_COUNT == 1
        {
            false => panic!("test message not found!"),
            true => Ok(())
        }?;
        MESSAGE_COUNT = 0;

        shard_man.lock().await.shutdown_all().await;
        secondshard.lock().await.shutdown_all().await;
        Ok(())
    }
    
    // https://github.com/serenity-rs/serenity/blob/ffc2997f8c76285b03dc31e62d653b40a553acf0/src/builder/create_message.rs
    #[tokio::test]
    async fn can_send_a_message() -> Result<(), Error> {
        Ok(())
    }
}
