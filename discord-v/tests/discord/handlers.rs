
mod testhandlers {
    use serenity::client::{
        Context, EventHandler
    };
    use serenity::{
        async_trait,
        http::Http,
        model::{
            id::{ChannelId},
            prelude::Message
        },
        utils::MessageBuilder,
    };
    static MESSAGE_CONTENT: &'static str = "testy boi";
    static EMBED_MESSAGE_INDICATOR: mut bool = false;
    static MESSAGE_INDICATOR: mut bool = false;        
    static IMAGE_MESSAGE_INDICATOR: mut bool = false;

    struct ReceiveEmbedMessageHandler;
    struct ReceiveMessageHandler;
    struct ReceiveImageEmbedMessageHandler;

    #[async_trait]
    impl EventHandler for ReceiveEmbedMessageHandler
    {
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
            {
                println!("Found test message");
                EMBED_MESSAGE_INDICATOR = true;
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
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
            {
                match &msg.embeds[0].image
                {
                    Some(img) => {
                    println!("Found special test message");
                    IMAGE_MESSAGE_INDICATOR = true;
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

    #[async_trait]
    impl EventHandler for ReceiveMessageHandler
    {
        async fn message(&self, ctx: Context, msg: Message)
        {
            if msg.content == MESSAGE_CONTENT
            {
                println!("Found test message");
                MESSAGE_INDICATOR = true;
            }
            else
            {
                println!("Found non-test message");
            }
        }
    }
}