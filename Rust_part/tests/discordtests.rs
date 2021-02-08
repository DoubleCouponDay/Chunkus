mod handlers;
mod svgtests;

#[cfg(test)]
mod tests {
    use std::marker::Send;
    use vecbot::secrettoken::{
        gettoken, getchannelid
    };
    use vecbot::bot::{
        create_vec_bot,
        create_bot_with_handle,
        DefaultHandler,

    };
    use std::result::Result;
    use std::io::Error;
    use tokio;
    use tokio::runtime::Runtime;
    use serenity;
    use serenity::{
        client::bridge::gateway::ShardManager,
        Client,
        http::Http,
        model::{
            id::{ChannelId},
        },
        utils::MessageBuilder,
    };
    use std::{thread, thread::JoinHandle, time::{Duration}};
    use serenity::client::{EventHandler};
    
    use std::sync::{Mutex, Arc};
    
    use super::handlers::{
        MESSAGE_CONTENT,
        ReceiveEmbedMessageHandler, ReceiveMessageHandler, ReceiveImageEmbedMessageHandler,        
    };

    struct RunningBot {
        client: Client,
        shard_manager: Arc<tokio::sync::Mutex<ShardManager>>,
        thread: JoinHandle<()>
    }
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let _token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let _outcome = create_vec_bot(gettoken());
        Ok(())
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let mut client = create_vec_bot(gettoken()).await;
        let shard_man = client.shard_manager.clone();

        let _ = client.start();
        println!("bot shutting down...");
        shard_man.lock().await.shutdown_all().await;

        Ok(())
    }

    async fn start_running_bot<H: EventHandler + 'static>(handler: H) -> RunningBot {
        let token1 = gettoken();
        let mut client = create_bot_with_handle(token1, DefaultHandler).await;

        // Used to shutdown
        let shard_manager = client.shard_manager.clone();

        // Start bot 1 (Vectorizer)
        let _ = client.start();
        
        // Start bot 2  in another thread
        let thread = thread::spawn(move || {
            let runtime = Runtime::new().expect("Unable to create the runtime");
    
            println!("Runtime created");
                
            // Continue running until notified to shutdown
            runtime.block_on(async {
                println!("inside async block");

                let token2 = gettoken();

                let mut client2 = create_bot_with_handle(token2, handler).await;
                
                client2.start().await.expect(" big pp");
            });
        
            println!("Runtime finished");
        });
        
        RunningBot {
            client,
            shard_manager,
            thread
        }
    }

    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        println!("starting two bots...");
        let token1 = gettoken();
        
        println!("check whether messages can be sent between them...");
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();
                
        let handler = ReceiveMessageHandler { 
            message_received_mutex: indicator_clone
        };

        let running_bot: RunningBot = start_running_bot(handler).await;

        // Wait for other bot to connect and then send message
        let message = MessageBuilder::new()
            .push(MESSAGE_CONTENT)
            .build();

        thread::sleep(Duration::from_secs(5));

        if let Err(_message_sent) = channelid.say(&http, &message).await {
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
        running_bot.shard_manager.lock().await.shutdown_all().await;

        Ok(())
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        let token1 = gettoken();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let running_bot: RunningBot = start_running_bot(handler).await;

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
        running_bot.shard_manager.lock().await.shutdown_all().await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {    
        let token1 = gettoken();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveImageEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let running_bot: RunningBot = start_running_bot(handler).await;

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
                e.image(super::svgtests::TEST_IMAGE);
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
        running_bot.shard_manager.lock().await.shutdown_all().await;
        Ok(())
    }
}
