
    use discord_v::secrettoken::{gettoken, gettestbotstoken};
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
            id::{ChannelId, UserId},
            prelude::Message
        },
        utils::MessageBuilder,
        framework::standard::
        {
            Args, CommandResult, CommandGroup,
            macros::{command, group, help, check, hook},
        },
    };
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::{thread, time::{Duration}};
    use serenity::client::{Context, EventHandler};
    use libc::c_int;



#[group]
#[commands(ping, vectorize)]
struct General;

#[command]
async fn vectorize(ctx: &Context, msg: &Message, args: Args) -> CommandResult
{
    // let url = args.single::<str>()?;

    // let print_str = String::from("url thingo: {0}")
    //     ::push_str(url);

    // println!(print_str);

    Ok(())
}

// async fn create_vectorizer_bot() -> Client
async fn create_vectorizer_bot()
{
    // let token = gettoken();
    // bot::create_bot_fn(token, |c| c
    //     .group(&GENERAL_GROUP))
}

#[command]
async fn ping(ctx: &Context, msg: &Message) -> CommandResult
{
    Ok(())
}