mod handlers;

#[cfg(test)]
mod tests {
    use discord_v::secrettoken::{
        gettoken, getchannelid
    };
    use discord_v::bot::{
        create_bot_with_handle,
        DefaultHandler,

    };
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
    use std::{thread, time::{Duration}};
    use serenity::client::{Context, EventHandler};
    
    use std::sync::{
        Mutex, Arc
    };
    use super::handlers::{
        MESSAGE_CONTENT,
        ReceiveEmbedMessageHandler, ReceiveMessageHandler, ReceiveImageEmbedMessageHandler,        
    };
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let outcome = createbot(gettoken(), DefaultHandler);
        Ok(())
    }

    async fn createbot<T: EventHandler + 'static>(token: &str, handler: T) -> serenity::Client {
        println!("creating bot...");
        let client = create_bot_with_handle(token, handler).await;
        client
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = createbot(gettoken(), DefaultHandler).await;
        let shard_man = client.shard_manager.clone();

        let _ = client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
        
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        
        // Start bot 2  in another thread
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");
    
            println!("Runtime created");
                
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettoken();

                let mut client2 = createbot(token2, ReceiveMessageHandler{ message_received_mutex: shared_indicator_mutex }).await;
                
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
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
    
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));

        // Start bot 2 in another thread to *listen* for the message
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");

            println!("Runtime created");
            
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettoken();
                
                let handler = ReceiveEmbedMessageHandler{ message_received_mutex: shared_indicator_mutex };
                let mut client2 = createbot(token2, handler).await;
            
                client2.start().await.expect("client 2 couldnt start");
            });
    
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message

        thread::sleep(Duration::from_secs(5));
        
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
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {
        println!("starting two bots...");
        let token1 = gettoken();
        let mut client = createbot(token1, DefaultHandler).await;

        // Used to shutdown
        let shard_man = client.shard_manager.clone();
    
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);

        // Start bot 1 (Vectorizer)
        let _ = client.start();
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
    
        // Start bot 2 in another thread to *listen* for the message
        let tokio_thread = thread::spawn(move || {
            let mut runtime = Runtime::new().expect("Unable to create the runtime");

            println!("Runtime created");
            
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettoken();

                let mut client2 = createbot(token2, ReceiveImageEmbedMessageHandler{ message_received_mutex: shared_indicator_mutex.clone() }).await;
            
                client2.start().await.expect("client 2 couldnt start");
            });
    
            println!("Runtime finished");
        });

        // Wait for other bot to connect and then send message

        thread::sleep(Duration::from_secs(5));
        
        println!("Emptying Indicator");
        
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
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        shard_man.lock().await.shutdown_all().await;
        Ok(())
    }
}
