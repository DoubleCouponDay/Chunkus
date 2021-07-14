mod handlers;
mod runner;
mod consts;

#[cfg(test)]
mod tests {
    use vecbot::{bot::{create_bot_with_handle, create_vec_bot, generate_bot_id}, secrettoken::{getchannelid, gettoken, getwatchertoken}, trampoline::{create_trampoline_bot}};
    use tokio;
    use serenity::{client::bridge::gateway::ShardManager, http::Http, model::{
            id::{ChannelId},
        }, prelude::Mutex, utils::MessageBuilder};
    use std::{io::Error, result::Result, sync::{Arc, atomic::{AtomicBool, Ordering}}, thread::sleep, time::{Duration}};
    
    use super::{
        handlers::{
            RECEIVE_CONTENT,
            RECEIVE_EMBED_CONTENT,
            RECEIVE_IMAGE_EMBED_CONTENT,
            ReceiveEmbedMessageHandler, 
            ReceiveMessageHandler, 
            ReceiveImageEmbedMessageHandler,
            CrashRunHandler,
            get_test_framework,
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
        let bot = create_vec_bot(tokenstr, false).await;
        let shards = bot.shard_manager.clone();
        let _running_bot: RunningBot = start_running_bot(bot);
        println!("bot shutting down...");
        sleep(Duration::from_secs(2));
        shutdown_bot(shards).await;
        Ok(())
    }

    #[tokio::test]
    async fn can_send_and_receive_a_message() -> Result<(), Error> {
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let token2sized = getwatchertoken();
        let token2 = token2sized.as_str();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token2);
        let flag = Arc::new(AtomicBool::new(false));

        let handler = ReceiveMessageHandler { 
            worked: flag.clone()
        };        
        let id = generate_bot_id(token1).await;
        let framework = get_test_framework(id.0).await;
        let bot = create_bot_with_handle(token1, handler, framework, false).await;
        let shards = bot.shard_manager.clone();
        let _running_bot: RunningBot = start_running_bot(bot);

        sleep(Duration::from_secs(2));

        if let Err(_message_sent) = channelid.send_message(&http, |m| {
            m.content(RECEIVE_CONTENT);
            m
        }).await {
            panic!("test message not sent!");
        }

        // Give time to receive message
        sleep(Duration::from_secs(2));

        if flag.load(Ordering::SeqCst) == false
        {
            shutdown_bot(shards).await;
            panic!("test message not received!");
        }

        shutdown_bot(shards).await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_a_message() -> Result<(), Error>
    { 
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let channelid = ChannelId(getchannelid());
        let token2sized = getwatchertoken();
        let token2 = token2sized.as_str();
        let http = Http::new_with_token(&token2);
        let worked = Arc::new(AtomicBool::new(false));

        let handler = ReceiveEmbedMessageHandler { 
            worked: worked.clone()
        };
        let id = generate_bot_id(token1).await;
        let framework = get_test_framework(id.0).await;
        let bot = create_bot_with_handle(token1, handler, framework, false).await;
        let shards = bot.shard_manager.clone();
        let _running_bot: RunningBot = start_running_bot(bot);
        
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

        let flag = worked.load(Ordering::SeqCst);

        if flag == false {
            shutdown_bot(shards).await;
            panic!("test message not received!");
        }

        // Shutdown bot 1
        shutdown_bot(shards).await;
        Ok(())
    }

    #[tokio::test]
    async fn can_embed_an_image_message() -> Result<(), Error>
    {    
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let channelid = ChannelId(getchannelid());
        let http = Http::new_with_token(&token1);
        
        let flag = Arc::new(AtomicBool::new(false));

        let handler = ReceiveImageEmbedMessageHandler{ 
            worked: flag.clone()
        };
        let id = generate_bot_id(token1).await;
        let framework = get_test_framework(id.0).await;
        let bot = create_bot_with_handle(token1, handler, framework, false).await;
        let shards = bot.shard_manager.clone();
        let _running_bot: RunningBot = start_running_bot(bot);
        
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
        if flag.load(Ordering::SeqCst) == false
        {
            shutdown_bot(shards).await;
            panic!("test message not received!");
        }

        // Shutdown bot 1
        shutdown_bot(shards).await;
        Ok(())
    }

    #[tokio::test]
    async fn trampoline_runnable() -> Result<(), Error> {
        let token2 = getwatchertoken();
        let trampoline = create_trampoline_bot(token2.as_str(), false, None).await;
        let shards = trampoline.shard_manager.clone();
        let _foo = start_running_bot(trampoline);
        println!("trampoline running...");
        sleep(Duration::from_secs(2));
        shutdown_bot(shards).await;
        Ok(())
    }

    #[tokio::test]
    async fn crashing_returns_an_informative_status_code() -> Result<(), Error> {
        //start trampoline with the shouldcrash flag raised
        let token2 = getwatchertoken();
        let http = Http::new_with_token(&token2);
        let id2 = generate_bot_id(&token2).await;
        let framework2 = get_test_framework(id2.0).await;
        let trampoline = create_trampoline_bot(&token2, true, Some(framework2)).await; //sets the crash scenario
        let shards2 = trampoline.shard_manager.clone();
        let _foo = start_running_bot(trampoline);
        println!("trampoline running...");

        //start auxiliary bot for reading chat
        let channelid = ChannelId(getchannelid());
        let tokensized = gettoken();
        let token1 = tokensized.as_str();
        let flag = Arc::new(AtomicBool::new(false));

        let crash_handler = CrashRunHandler{
            worked: flag.clone()
        };
        let id = generate_bot_id(token1).await;
        let framework = get_test_framework(id.0).await;
        let bot = create_bot_with_handle(token1, crash_handler, framework, false).await;
        let shards1 = bot.shard_manager.clone();
        let _running_bot: RunningBot = start_running_bot(bot);

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
        
        sleep(Duration::from_secs(2));
        let messageconfirmed = flag.load(Ordering::SeqCst);
        
        //check the mutex for confirmation of the status code in chat
        if messageconfirmed == false {
            shutdown_bot(shards2).await;
            shutdown_bot(shards1).await;
            panic!("received message was not the expected status code for this operating system!");
        }
        shutdown_bot(shards2).await;
        shutdown_bot(shards1).await;
        
        Ok(())
    }

    async fn shutdown_bot(shards: Arc<Mutex<ShardManager>>) {
        shards.lock().await.shutdown_all().await;
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
