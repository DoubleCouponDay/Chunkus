use serenity::{
    async_trait,
    http::Http,
    model::{
        prelude::{Message, MessageId, MessageUpdateEvent}
    },
    prelude::TypeMapKey,
    framework::standard::{
        CommandResult,
        Args,
        StandardFramework,
        macros::{
            group, command,
        },
    },
    client::{
        Client, ClientBuilder, Context, EventHandler
    },
};
use crate::core::{do_vectorize};
use std::{
    collections::{HashSet, HashMap},
    fs::File,
    fs::remove_file,
    path::Path,
    io::prelude::*,
    time::{Duration, Instant},
};
use crate::constants;
use crate::svg::render_svg_to_png;


pub async fn create_bot_with_handle<H: EventHandler + 'static>(token: &str, handler: H) -> Client {    
    println!("creating http token...");
    let http = Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (_, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new().configure(|c| c
        .on_mention(Some(_bot_id))
        .with_whitespace(true));
        
    println!("creating client...");

    // Login with a bot token from the environment
    let client = ClientBuilder::new(&token)
        .event_handler(handler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub async fn create_vec_bot(token: &str) -> Client
{
    println!("creating http token...");
    let http = Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (_, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new().configure(|c| c
        .prefix("!")
        .with_whitespace(true))
            .group(&GENERAL_GROUP);
        
    println!("bot is running...");

    // Login with a bot token from the environment
    let client = ClientBuilder::new(&token)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error bot is running");

    {
        let mut data = client.data.write().await;
        data.insert::<MsgListen>(HashSet::<MessageId>::new());
        data.insert::<MsgUpdate>(HashMap::<MessageId, MessageUpdateEvent>::new());
        data.insert::<VectorizeOptionsKey>(VectorizeOptions { chunk_size: 4u32, threshold: 0.25f32 });
    }

    client
}

pub struct DefaultHandler;

#[group]
#[commands(vectorize, set_algorithm, params, delete)]
struct General;

#[async_trait]
impl EventHandler for DefaultHandler {
    async fn message(&self, _ctx: Context, _msg: Message) {
        println!("message received {:?}", _msg.content);
    }

    async fn message_update(&self, ctx: Context, _old_if_available: Option<Message>, _new: Option<Message>, event: MessageUpdateEvent)
    {
        // check if message id is one we are listening to
        // only send embeds if there are embeds (if not then could've been a general message edit not embed attached) 
        
        println!("Received Message Update for {:?}", event.id);

        let id = event.id;

        // wait_for_message_update

        // Wait for a message to start listening
        if let Ok(_) = has_listener(&ctx, id, Duration::from_secs(constants::WAIT_FOR_MESSAGE_UPDATE_TIMEOUT_S)).await
        {
            // get data
            println!("Message Update is being listened for");
            let mut data_write = ctx.data.write().await;
            let update_hashmap = data_write.get_mut::<MsgUpdate>().unwrap();
            update_hashmap.insert(id, event);
            println!("Inserted MessageUpdateEvent into hashmap");

        }
        // {
        //     let data = ctx.data.read().await;
        //     let listen_hashset = data.get::<MsgListen>().unwrap();
        //     contains = listen_hashset.contains(&id);
        //     debug_output = format!("{:?}", listen_hashset);
        // }
        // if contains
        // {
            
        // }
        else
        {
            println!("Msg update was not listened for ");
        }
    }
}

struct MsgListen;
struct MsgUpdate;
struct VectorizeOptionsKey;
struct VectorizeOptions
{
    chunk_size: u32,
    threshold: f32,
}

impl TypeMapKey for MsgListen
{
    type Value = HashSet<MessageId>;
}

impl TypeMapKey for MsgUpdate
{
    type Value = HashMap<MessageId, MessageUpdateEvent>;
}

impl TypeMapKey for VectorizeOptionsKey
{
    type Value = VectorizeOptions;
}

//async fn has_data<F>(ctx: &Context, msg_id: MessageId, timeout: Duration, f: F) -> Result<(), ()> where F: Fn(&Context) -> bool
async fn has_data(ctx: &Context, msg_id: MessageId, timeout: Duration) -> Result<(), ()>
{
    let start_time = Instant::now();

    loop
    {
        if start_time.elapsed() > timeout
        {
            break;
        }

        // Check if the data is now there
        //println!("Checking for message update for {0}", &msg_id);
        let contains: bool;
        {
            let data_read = ctx.data.read().await;
            let updated = data_read.get::<MsgUpdate>().unwrap();
            contains = updated.contains_key(&msg_id)
        }

        if contains
        {
            return Ok(());
        }
    }

    Err(())
}

async fn has_listener(ctx: &Context, msg_id: MessageId, timeout: Duration) -> Result<(), ()>
{
    let start_time = Instant::now();

    loop
    {
        if start_time.elapsed() > timeout
        {
            break;
        }

        // Check if the data is now there
        //println!("Checking for message update for {0}", &msg_id);
        let contains: bool;
        {
            let data_read = ctx.data.read().await;
            let listeners = data_read.get::<MsgListen>().unwrap();
            contains = listeners.contains(&msg_id)
        }

        if contains
        {
            return Ok(());
        }
    }

    Err(())
}

async fn wait_for_message_update(msg_id: MessageId, ctx: &Context) -> Result<MessageUpdateEvent, String>
{
    // Insert the data for the on_update to listen to
    {
        println!("Inserting {:?} into Listening", msg_id);
        let mut data_write = ctx.data.write().await;
        let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
        stored_hashset.insert(msg_id);
    }

    // Wait for there to be an update
    if let Ok(_) = has_data(&ctx, msg_id, Duration::from_secs(constants::WAIT_FOR_MESSAGE_UPDATE_TIMEOUT_S)).await
    {
        // get data
        let mut data_write = ctx.data.write().await;
        let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
        stored_hashset.remove(&msg_id);
        let updated_write = data_write.get_mut::<MsgUpdate>().unwrap();
        if let Some(ass) = updated_write.remove(&msg_id)
        {
            let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
            stored_hashset.remove(&msg_id);
            return Ok(ass);
        }
        else
        {
            let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
            stored_hashset.remove(&msg_id);
            return Err(String::from("couldn't remove"));
        }
    }
    
    // wait_for_data()
    // if exists: load data from hashmap
    let mut data_write = ctx.data.write().await;
    let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
    stored_hashset.remove(&msg_id);
    Err(String::from("timed out"))
}

#[command]
#[aliases("p")]
async fn params(ctx: &Context, msg: &Message, args: Args) -> CommandResult
{
    let mut argss = args;
    if let Ok(chunk_size) = argss.single::<u32>()
    {
        let threshold = argss.single::<f32>().unwrap_or(0.5f32);

        let mut data_write = ctx.data.write().await;
        let options = data_write.get_mut::<VectorizeOptionsKey>().unwrap();
        options.chunk_size = chunk_size;
        options.threshold = threshold;

        if let Err(why) = msg.reply(&ctx.http, format!("Set Chunk Size to: {} and Threshold to: {}", options.chunk_size, options.threshold)).await
        {
            eprintln!("Error sending params reply: {:?}", why);
        }
    }
    Ok(())
}

#[command]
#[aliases("d")]
async fn delete(ctx: &Context, msg: &Message, args: Args) -> CommandResult
{
    if let Ok(msg_id) = args.rest().parse::<u64>()
    {
        if let Err(why) = ctx.http.delete_message(msg.channel_id.0, msg_id).await
        {
            eprintln!("Error deleting message: {:?}", why);
        }
    }
    Ok(())
}

#[command]
#[aliases("v")]
async fn vectorize(ctx: &Context, msg: &Message, _args: Args) -> CommandResult {
    actual_vectorize(ctx, msg).await
}

#[command]
#[aliases("algo")]
async fn set_algorithm(ctx: &Context, msg: &Message, args: Args) -> CommandResult
{
    println!("Setting algorithm");
    let potential_algo = args.rest().parse::<i32>();
    if potential_algo.is_err()
    {
        if let Err(why) = msg.reply(&ctx.http, "Invalid input given!").await
        {
            eprintln!("Failed to reply to set_algorithm: {}", why);
            return Ok(());
        }
    }
    let algorithm: i32 = potential_algo.unwrap();

    let c_error = super::core::set_algorithm(algorithm);

    if c_error != constants::FfiResult::SuccessCode
    {
        if let Err(why) = msg.reply(&ctx.http, format!("An error occurred: {}", c_error)).await
        {
            eprintln!("Failed to send an error message: {}", why);
        }
    }
    else
    {
        if let Err(why) = msg.reply(&ctx.http, format!("Successfully set algorithm to: {}", algorithm)).await
        {
            eprintln!("Failed to reply to set_algorithm call: {}", why);
        }
    }
    Ok(())
}

async fn actual_vectorize(ctx: &Context, msg: &Message) -> CommandResult
{
    println!("Joe Mama");
    
    let mut embed_urls: Vec<String> = vec![];
    if msg.embeds.len() < 1 && msg.attachments.len() < 1
    {   
        println!("No embeds; waiting for message update");
        // No embed, lets wait for an on_update
        match wait_for_message_update(msg.id, &ctx).await
        {
            Ok(update_data) =>
                {
                    println!("Received Ok from wait_for_message_update");
                    if let Some(embeds) = update_data.embeds
                    {
                        for embed in embeds.iter()
                        {
                            if let Some(pp_url) = &embed.url
                            {
                                println!("Pushing embed url");
                                embed_urls.push(pp_url.clone());
                            }
                        }
                    }
                    if let Some(attachments) = update_data.attachments
                    {
                        for attachment in attachments.iter()
                        {
                            println!("Pushing Attachment url");
                            embed_urls.push(attachment.url.clone());
                        }
                    }
                },
            Err(err) =>

            println!("Received Err {} from wait_for_message_update for id: {}", err, msg.id),
        }
    }
    else
    {
            //embed_url = blah
        // We have an embed
        println!("vectorizing...");
        println!("embed count {0}", msg.embeds.len());
        println!("attachments count {0}", msg.attachments.len());
        println!("message contents {0}", msg.content);
        for embed in msg.embeds.iter() {
            if let Some(url) = &embed.url
            {
                println!("Pushing embed url");
                embed_urls.push(url.clone());
            }
        }
        for attachment in msg.attachments.iter()
        {
            println!("Pushing attachment url");
            embed_urls.push(attachment.url.clone());
        }
    }
    
    println!("Sending {0} urls to vectoriser", embed_urls.len());
    println!("Yo Mama {:?}", embed_urls);
    vectorize_urls(&ctx, &msg, &embed_urls).await;
    
    Ok(())
}

async fn vectorize_urls(ctx: &Context, msg: &Message, urls: &Vec<String>)
{  
    println!("vectorize_urls with: {:?}", urls);
    for url in urls.iter()
    {
        println!("vectorizing url {:?}", url);
        // Download file using Reqwest

        let client = reqwest::Client::new();

        let response = match client.get(url).send().await
            {
                Err(_) => { println!("could not download url: {:?}", url); continue },
                Ok(thing) => thing
            };
    
        {
            let mut file = match File::create(Path::new(constants::INPUTFILENAME))
            {
                Err(err) => { println!("could not create path to copy image to Err: {:?}", err); continue },
                Ok(thing) => thing
            };
            
            match response.bytes().await
            {
                Ok(bytes) => match file.write_all(&bytes) { Err(_) => continue, _ => () },
                Err(err) => { println!("could get bytes to copy from reqwest response Err: {:?}", err); continue },
            };

            if let Err(err) = file.sync_all()
            {
                eprintln!("Error {}", err);
                continue;
            }
        }

        println!("Removing existing file");
        if let Err(err) = remove_file(Path::new(constants::OUTPUTFILENAME))
        {
            println!("Remove file Err: {:?} probably not important", err);
        }



        // Execute Algorithm
        let inputname = String::from(constants::INPUTFILENAME);
        let outputname = String::from(constants::OUTPUT_SVG_FILE);


        // Get Options
        let chunksize;
        let threshold;

        { //dont hold the entire read in memory for too long
            let data_read = ctx.data.read().await;
            let options = data_read.get::<VectorizeOptionsKey>().unwrap();
            chunksize = String::from(format!("{}", options.chunk_size));
            threshold = String::from(format!("{}", options.threshold));
        }
        
        println!("Vectorizing....");
        let result = do_vectorize(&inputname, &outputname, Some(chunksize), Some(threshold));        

        let possibleerror: &str = result.into();

        if possibleerror != "SuccessCode" {
            if let Err(why) = msg.reply(&ctx.http, possibleerror)
            .await { 
                println!("Error replying: {}", why);
            };
            continue;
        }

        else {
            println!("success");
        }

        let png_output = String::from(constants::OUTPUTFILENAME);

        // Render to png
        println!("Rendering Output");
        if let Err(why) = render_svg_to_png(&outputname, &png_output)
        {
            println!("Failed to render svg to png: {}", why);
            if let Err(msg_why) = msg.reply(&ctx.http, format!("Failed to render svg to png: {}", why)).await
            {
                println!("Failed to reply to msg: {}", msg_why);
            }
            continue;
        }


        // Send the output
        let msg_files = vec![outputname.as_str(), png_output.as_str()];

        let msg = msg.channel_id.send_files(&ctx.http, msg_files, |m|
        {
            m.content("Here's your result")
        }).await;

        if let Err(err) = msg
        {
            println!("Error sending result {}", err);
        }
    }
}
