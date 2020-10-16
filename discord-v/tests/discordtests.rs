#[path = "../src/main.rs"]
mod src;

#[cfg(test)]
mod tests {
    use crate::src::secrettoken::{gettoken, gettestbotstoken};
    use crate::src::bot;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use tokio::{time::Delay, runtime::Runtime};
    use serenity;
    use serenity::{
        async_trait,
        http::Http,
        model::{
            id::{ChannelId, UserId},
            prelude::Message
        },
        utils::MessageBuilder,
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

        client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    fn getchannelid() -> ChannelId {
        ChannelId(418671857676451841) //the bot chat
    }

    fn get_vectorizer_bot_id() -> UserId {
        UserId(690684027019067393)
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
            else
            {
                println!("Found non-test message");
            }
        }
    }

    static MESSAGE_CONTENT: &'static str = "testy boi";
    static MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);

    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, receivemessagehandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
        
        println!("check whether messages can be sent between them...");
        let channelid = getchannelid();
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
        
        // Start bot 2 (Gay Fag Machine) in another thread
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");
    
            println!("Runtime created");
                
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettestbotstoken();

                let mut client2 = createbot(token2, receivemessagehandler).await;
                
                client2.start().await;
            });
        
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message
        let message = MessageBuilder::new()
        .push(MESSAGE_CONTENT)
        .build();

        thread::sleep(Duration::from_secs(5));

        if let Err(message_sent) = channelid.say(&http, &message).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        thread::sleep(Duration::from_secs(10));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if MESSAGE_INDICATOR.load(Ordering::Relaxed) == false 
        {
            panic!("test message not received!");
        }
        MESSAGE_INDICATOR.store(false, Ordering::Relaxed);

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }

    struct ReceiveEmbedMessageHandler;

    #[async_trait]
    impl EventHandler for ReceiveEmbedMessageHandler
    {
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
            {
                println!("Found test message");
                MESSAGE_INDICATOR.store(true, Ordering::Relaxed);
            }
            else
            {
                println!("Found non-test message");
            }
        }
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        // I am lazy and I need this test to not be executed at the same time as the other one
        // This shows problems due to the fact that MESSAGE_INDICATOR is used for both tests
        // Its problematic that these tests are multi-threaded at all, only one test should really be trying to control the bots 
        thread::sleep(Duration::from_secs(15));


        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, receivemessagehandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
    
        println!("check whether messages can be sent between them...");
        let channelid = getchannelid();
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
    
        // Start bot 2 (Gay Fag Machine) in another thread to *listen* for the message
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");

            println!("Runtime created");
            
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettestbotstoken();

                let mut client2 = createbot(token2, ReceiveEmbedMessageHandler).await;
            
                client2.start().await;
            });
    
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message

        thread::sleep(Duration::from_secs(5));
        
        MESSAGE_INDICATOR.store(false, Ordering::Relaxed);
        if let Err(message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(MESSAGE_CONTENT);
            m.embed(|e|
            {
                e.title("Epic Embed");
                e.description("Epic Embed but in smaller font");
                e.fields(vec![("wtf is this", "like wtf is this", true)]);
                e
            });
            m
        }).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        thread::sleep(Duration::from_secs(10));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if MESSAGE_INDICATOR.load(Ordering::Relaxed) == false 
        {
            panic!("test message not received!");
        }
        MESSAGE_INDICATOR.store(false, Ordering::Relaxed);

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }
}
