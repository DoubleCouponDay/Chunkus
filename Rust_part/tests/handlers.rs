use serenity::client::{
    Context, EventHandler
};
use serenity::framework::StandardFramework;
use serenity::{
    async_trait,
    model::{
        prelude::Message
    },
    prelude::Mutex
};
use vecbot::bot::generate_bot_id;
use vecbot::secrettoken::gettoken;
use std::sync::Arc;

pub static RECEIVE_EMBED_CONTENT: &'static str = "receive embed test";
pub static RECEIVE_CONTENT: &'static str = "receive content test";
pub static RECEIVE_IMAGE_EMBED_CONTENT: &'static str = "receive image embed test";
pub struct ReceiveEmbedMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>
}
pub struct ReceiveMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>
}
pub struct ReceiveImageEmbedMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>
}
pub struct StartOtherBotHandler;
pub struct CrashRunHandler {
    pub message_received_mutex: Arc<Mutex<bool>>
}

#[async_trait]
impl EventHandler for ReceiveEmbedMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        println!("ReceiveEmbedMessageHandler: {}", msg.content);

        if msg.content == RECEIVE_EMBED_CONTENT && msg.embeds.len() > 0
        {
            println!("Found receive embed test message");
            *self.message_received_mutex.lock().await = true;
        }
        else
        {
            println!("Found non-test message");
        }
    }
}

#[async_trait]
impl EventHandler for ReceiveImageEmbedMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        println!("ReceiveImageEmbedMessageHandler: {}", msg.content);

        if msg.content == RECEIVE_IMAGE_EMBED_CONTENT && msg.embeds.len() > 0
        {
            match &msg.embeds[0].image
            {
                Some(_img) => {
                    println!("Found image embed test message");
                    *self.message_received_mutex.lock().await = true;                    
                }
                None => {
                    println!("no image in embed")
                }
            }
        }

        else
        {
            println!("Found non-test message");
        }
    }
}

#[async_trait]
impl EventHandler for ReceiveMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        println!("ReceiveMessageHandler: {}", msg.content);

        if msg.content == RECEIVE_CONTENT
        {
            println!("Found receive test message");
            *self.message_received_mutex.lock().await = true;   
        }

        else
        {
            println!("Found non-test message");
        }
    }
}

#[async_trait]
impl EventHandler for CrashRunHandler {
    async fn message(&self, _ctx: Context, msg: Message) {
        let msgstr = msg.content.as_str();
        println!("crashrunhandler: {}", msgstr);
        *self.message_received_mutex.lock().await = true;
    }
}

pub async fn get_test_framework(token: &str) -> StandardFramework {
    let bot_id = generate_bot_id(token).await;

    let framework = StandardFramework::new().configure(|c| c
        .on_mention(Some(bot_id))
        .with_whitespace(true));

    framework
}