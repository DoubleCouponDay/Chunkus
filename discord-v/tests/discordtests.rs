#[cfg(test)]
mod tests {
    use discord_v::secrettoken::{
        gettoken, gettestbotstoken, getchannelid
    };
    use discord_v::bot;
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use tokio::runtime::Runtime;
    use serenity;
    use serenity::{
        async_trait,
        http::Http,
        model::{
            id::{ChannelId},
            prelude::Message
        },
        utils::MessageBuilder,
    };
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::{thread, time::{Duration}};
    use serenity::client::{Context, EventHandler};
    use libc::c_int;
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let outcome = createbot(gettoken(), bot::DefaultHandler);
        Ok(())
    }

    async fn createbot<T: EventHandler + 'static>(token: &str, handler: T) -> serenity::Client {
        println!("creating bot...");
        let client = bot::create_bot_with_handle(token, handler).await;
        client
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = createbot(gettoken(), bot::DefaultHandler).await;
        let shard_man = client.shard_manager.clone();

        let _ = client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    struct ReceiveMessageHandler;

    #[async_trait]
    impl EventHandler for ReceiveMessageHandler
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
        let mut client = createbot(token1, bot::DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
        
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
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

                let mut client2 = createbot(token2, ReceiveMessageHandler).await;
                
                client2.start().await.expect(" big pp");
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

    static EMBED_MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);

    struct ReceiveEmbedMessageHandler;

    #[async_trait]
    impl EventHandler for ReceiveEmbedMessageHandler
    {
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
            {
                println!("Found test message");
                EMBED_MESSAGE_INDICATOR.store(true, Ordering::Relaxed);
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
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, bot::DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
    
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
    
        // Start bot 2 in another thread to *listen* for the message
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");

            println!("Runtime created");
            
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettestbotstoken();

                let mut client2 = createbot(token2, ReceiveEmbedMessageHandler).await;
            
                client2.start().await.expect("client 2 couldnt start");
            });
    
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message

        thread::sleep(Duration::from_secs(5));
        
        EMBED_MESSAGE_INDICATOR.store(false, Ordering::Relaxed);
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
        if EMBED_MESSAGE_INDICATOR.load(Ordering::Relaxed) == false 
        {
            panic!("test message not received!");
        }
        EMBED_MESSAGE_INDICATOR.store(false, Ordering::Relaxed);

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }
    
    static IMAGE_MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);

    struct ReceiveImageEmbedMessageHandler;

    #[async_trait]
    impl EventHandler for ReceiveImageEmbedMessageHandler
    {
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
            {
                match &msg.embeds[0].image
                {
                    Some(img) => {
                    println!("Found special test message");
                    IMAGE_MESSAGE_INDICATOR.store(true, Ordering::Relaxed);
                    }
                    None => {}
                }
            }
            else
            {
                println!("Found non-test message");
            }
        }
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, bot::DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
    
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
    
        // Start bot 2 in another thread to *listen* for the message
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");

            println!("Runtime created");
            
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettestbotstoken();

                let mut client2 = createbot(token2, ReceiveImageEmbedMessageHandler).await;
            
                client2.start().await.expect("client 2 couldnt start");
            });
    
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message

        thread::sleep(Duration::from_secs(5));
        
        println!("Emptying Indicator");
        IMAGE_MESSAGE_INDICATOR.store(false, Ordering::Relaxed);
        if let Err(message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(MESSAGE_CONTENT);
            m.embed(|e|
            {
                e.title("Epic Embed");
                e.description("Epic Embed but in smaller font");
                e.fields(vec![("wtf is this", "like wtf is this", true)]);
                e.image("https://cdn.vox-cdn.com/thumbor/7BMHi-0CALa3odBFMD-MJK9Ye4Y=/0x44:1268x889/1200x800/filters:focal(0x44:1268x889)/cdn.vox-cdn.com/uploads/chorus_image/image/47684009/Screenshot_2014-07-19_15.24.57.0.png");
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
        if IMAGE_MESSAGE_INDICATOR.load(Ordering::Relaxed) == false 
        {
            panic!("test message not received!");
        }
        IMAGE_MESSAGE_INDICATOR.store(false, Ordering::Relaxed);

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }
}
