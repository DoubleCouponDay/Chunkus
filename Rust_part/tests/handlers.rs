use serenity::client::{
    Context, EventHandler
};
use serenity::{
    async_trait,
    model::{
        prelude::Message
    },
};
use std::sync::{
    Mutex, Arc
};

pub static RECEIVE_EMBED_CONTENT: &'static str = "1";
pub static RECEIVE_CONTENT: &'static str = "2";
pub static RECEIVE_IMAGE_EMBED_CONTENT: &'static str = "3";
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
            *self.message_received_mutex.lock().unwrap() = true;
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
                    *self.message_received_mutex.lock().unwrap() = true;                    
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
            *self.message_received_mutex.lock().unwrap() = true;   
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
        if true {
            *self.message_received_mutex.lock().unwrap() = true;
        }

        else {
            println!("message was not the status code");
        }
    }
}