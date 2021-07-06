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

pub static MESSAGE_CONTENT: &'static str = "test";

pub struct ReceiveEmbedMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>,
}
pub struct ReceiveMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>
}
pub struct ReceiveImageEmbedMessageHandler
{
    pub message_received_mutex: Arc<Mutex<bool>>,
}

pub struct StartOtherBotHandler
{

}

#[async_trait]
impl EventHandler for ReceiveEmbedMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
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
        if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
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
        if msg.content == MESSAGE_CONTENT
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