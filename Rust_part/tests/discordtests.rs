mod handlers;
mod runner;
mod consts;

#[cfg(test)]
mod tests {
    use vecbot::{        
        secrettoken::{getchannelid, gettoken, getwatchertoken},
        bot::{create_vec_bot, create_bot_with_handle},
        trampoline::{initialize_child, create_trampoline_bot, TrampolineProcessKey}
    };
    use tokio;
    use serenity::{
        http::Http,
        model::{
            id::{ChannelId},
        },
        utils::MessageBuilder
    };
    use std::{
        thread::sleep, 
        time::{Duration},
        sync::{Mutex, Arc},
        result::Result,
        io::Error
    };    
    
    use super::{
        handlers::{
            RECEIVE_CONTENT,
            RECEIVE_EMBED_CONTENT,
            RECEIVE_IMAGE_EMBED_CONTENT,
            ReceiveEmbedMessageHandler, 
            ReceiveMessageHandler, 
            ReceiveImageEmbedMessageHandler,
            CrashRunHandler
        },
        consts::TEST_IMAGE,
        runner::{
            start_running_bot,
            RunningBot
        }
    };
    
    #[test]
    fn token_obtainable() -> Result<(), Error> {
        println!("fetching token...");
        let _token = gettoken();
        Ok(())
    }

    #[tokio::test]
    async fn bot_creatable() -> Result<(), Error> {
        let token = gettoken();
        let tokenstr = token.as_str();
        let _outcome = create_vec_bot(tokenstr, false);
        Ok(())
    }

    #[tokio::test]
    async fn bot_runnable() -> Result<(), Error> {
        let token = gettoken();
        let tokenstr = token.as_str();
        let mut client = create_vec_bot(tokenstr, false).await;
        let shard_man = client.shard_manager.clone();

        let _ = client.start();
        println!("bot shutting down...");
        Ok(())
    }


    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();
                
        let handler = ReceiveMessageHandler { 
            message_received_mutex: indicator_clone
        };

        let mut bot = create_bot_with_handle(token1, handler, false).await;
        let running_bot: RunningBot = start_running_bot(bot);

        // Wait for other bot to connect and then send message
        let message = MessageBuilder::new()
            .push(RECEIVE_CONTENT)
            .build();

        sleep(Duration::from_secs(2));

        if let Err(_message_sent) = channelid.say(&http, &message).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        sleep(Duration::from_secs(2));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let mut bot = create_bot_with_handle(token1, handler, false).await;
        let running_bot: RunningBot = start_running_bot(bot);
        
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(RECEIVE_EMBED_CONTENT);
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
        sleep(Duration::from_secs(2));


        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {    
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let shared_indicator_mutex: Arc<Mutex<bool>> = Arc::new(Mutex::new(false));
        let indicator_clone = shared_indicator_mutex.clone();

        let handler = ReceiveImageEmbedMessageHandler{ message_received_mutex: indicator_clone };
        let mut bot = create_bot_with_handle(token1, handler, false).await;
        let running_bot: RunningBot = start_running_bot(bot);
        
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content(RECEIVE_IMAGE_EMBED_CONTENT);
            m.embed(|e|
            {
                e.title("Epic Embed");
                e.description("Epic Embed but in smaller font");
                e.fields(vec![("wtf is this", "like wtf is this", true)]);
                e.image(TEST_IMAGE);
                e
            });
            m
        }).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        sleep(Duration::from_secs(2));

        // Finally check if MESSAGE_INDICATOR changed (indicating the 2nd bot received the message)
        // And reset to false
        if *shared_indicator_mutex.lock().unwrap() == false
        {
            panic!("test message not received!");
        }

        // Shutdown bot 1
        Ok(())
    }

    #[tokio::test]
    async fn trampoline_runnable() -> Result<(), Error> {
        let token2 = getwatchertoken();
        let mut trampoline = create_trampoline_bot(token2.as_str(), false).await;
        initialize_child(&trampoline.data, true).await; //raises the flag
        let _ = trampoline.start();
        println!("trampoline running...");
        sleep(Duration::from_secs(2));

        //close the unmanaged resources
        {
            println!("locking 6");
            let mut lock = trampoline.data.write().await;
            let child = lock.get_mut::<TrampolineProcessKey>().unwrap();
            let _ = child.vectorizer.kill().expect("bot was already killed");
            println!("unlocking 6");
        }
        Ok(())
    }

    #[tokio::test]
    async fn crashing_returns_an_informative_status_code() -> Result<(), Error> {
        //start trampoline with the shouldcrash flag raised
        let token2 = getwatchertoken();
        let http = Http::new_with_token(&token2);
        let mut trampoline = create_trampoline_bot(&token2, true).await; //sets the crash scenario
        let running_trampoline = start_running_bot(trampoline);
        println!("trampoline running...");

        //start auxiliary bot for reading chat
        let channelid = ChannelId(getchannelid());
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let mutex = Arc::new(Mutex::new(false));
        let crash_handler = CrashRunHandler{
            message_received_mutex: mutex.clone() //the underlying data store is shared
        };
        let mut bot = create_bot_with_handle(token1, crash_handler, false).await;
        let running_bot: RunningBot = start_running_bot(bot);

        sleep(Duration::from_secs(2));
        
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content("pls crash"); //just for fun. not required
            m
        }).await {
            panic!("test message not sent!");
        }

        //invoke vectorizer
        if let Err(_message_sent) = channelid.send_message(&http, |m| 
        {
            m.content("!v");

            m.embed(|e| {
                e.image(TEST_IMAGE);
                e
            });
            m
        }).await {
            panic!("test message not sent!");
        }
        
        sleep(Duration::from_secs(5));

        //check the mutex for confirmation of the status code in chat
        if *mutex.lock().unwrap() == false {
            panic!("received message was not the expected status code for this operating system!");
        }
        Ok(())
    }


    // #[tokio::test]
    // async fn second_bot_starts_first_bot_when_dead() -> Result<(), Error>
    // {
        // something like
        // make sure 1st bot not running
        // start 2nd bot
        // wait a bit
        // check if 1st bot running

        // let token = gettoken();
        // let handler = StartOtherBotHandler{};

        // sleep(Duration::from_secs(2));


        // Check if other bot is running
        // Either use the bot's http to check the bot UserId's status
        // Or ask the bot whether it thinks the bot is online
        

    //     Ok(())
    // }
}
