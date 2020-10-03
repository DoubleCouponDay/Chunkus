#[path = "../src/main.rs"]
mod src;

#[cfg(test)]
mod tests {
    use crate::src::secrettoken::gettoken;
    use crate::src::bot;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use serenity;
    use serenity::{
        async_trait,
        http::Http,
        model::{
            id::ChannelId,
            prelude::Message
        }
    };
    use std::sync::Arc;
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::{thread, time, time::{Duration, Instant}};
    use serenity::client::{Client, Context, EventHandler};
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let outcome = createbot(gettoken(), bot::defaultHandler);
        Ok(())
    }

    async fn createbot<T: EventHandler + 'static>(token: &str, Handler: T) -> serenity::Client {
        println!("creating bot...");
        let client = bot::create_bot_with_handle(token, Handler).await;
        client
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = createbot(gettoken(), bot::defaultHandler).await;
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
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT
            {
                println!("Found test message");
                MESSAGE_INDICATOR.store(true, Ordering::Relaxed);
            }
        }
    }

    static MESSAGE_CONTENT: &'static str = "testicular boi";
    static MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);

    #[tokio::test]
    async fn can_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, receivemessagehandler).await;
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
        let message = channelid.send_message(&http, |m| {
            m.content(MESSAGE_CONTENT);
            m.tts(false);
            m
        });

        let time_out = time::Duration::from_secs(69);
        thread::sleep(time_out);

        if MESSAGE_INDICATOR.load(Ordering::Relaxed) == false 
        {
            panic!("test message not found!");
        }
        MESSAGE_INDICATOR.store(false, Ordering::Relaxed);

        shard_man.lock().await.shutdown_all().await;
        //secondshard.lock().await.shutdown_all().await;
        Ok(())
    }
    
    // https://github.com/serenity-rs/serenity/blob/ffc2997f8c76285b03dc31e62d653b40a553acf0/src/builder/create_message.rs
    #[tokio::test]
    async fn can_send_a_message() -> Result<(), Error> {
        Ok(())
    }
}
