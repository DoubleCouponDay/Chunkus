use std::{
    collections::{HashSet, HashMap}, 
    fs::{File, read},
    io::Write,
    ops::Add, 
    path::Path, 
    time::{Duration, Instant}};

use serenity::{
    async_trait, 
    client::{
        Client, ClientBuilder, Context, EventHandler
    }, 
    framework::standard::{
        Configuration,
        CommandResult,
        Args,
        StandardFramework,
        macros::{
            group, command,
        },
    },
    model::{
        id::UserId,
        channel::Message, 
        prelude::{MessageId, MessageUpdateEvent}
    }, 
    prelude::{TypeMapKey, TypeMap, GatewayIntents}
};

use tokio::sync::RwLockWriteGuard;
use xz2::write::XzEncoder;
use crate::core::{
    do_vectorize, crashing_this_plane
};
use crate::constants;
use crate::svg::render_svg_to_png;
use crate::options::{
    VectorizeOptions,
    ParsedOptions,
    insert_params,
    insert_parsed_params,
    get_params,
    DEFAULT_CHUNK_SIZE,
    DEFAULT_THRESHOLDS,
    DEFAULT_COLOURS
};
use crate::secrettoken::getappid;

pub const START_MESSAGE: &'static str = "Working on it...";
pub const END_MESSAGE: &'static str = "Here's your result.";
pub const ERR_MESSAGE: &'static str = "error: ";
pub const OUTPUT_ARCHIVE: &'static str = "output.svg.xz";

struct MsgListen;
struct MsgUpdate;
pub struct DefaultHandler;

#[group]
#[commands(vectorize, vectorizerparams, chunksize, thresholds, numcolours)]
struct General;

pub async fn create_vec_bot(token: &str, shouldcrash: bool) -> Client
{
    println!("creating vec bot...");
    let appid = Some(UserId::from(getappid().parse::<u64>().expect("Not given valid bot user id!")));

    let framework = StandardFramework::new().configure(|c: &mut Configuration | c
            .with_whitespace(true)
            .case_insensitivity(true)
            .allow_dm(false)
            .on_mention(appid)
        )
        .group(&GENERAL_GROUP);

    let intents: GatewayIntents = GatewayIntents::GUILD_MESSAGES |
        GatewayIntents::DIRECT_MESSAGES |
        GatewayIntents::MESSAGE_CONTENT;

    let client = ClientBuilder::new(token, intents)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error creating client");
        
    initialize_bot(&client, shouldcrash).await;

    client
}

pub async fn initialize_bot(client: &Client, shouldcrash: bool) {
        let mut data: RwLockWriteGuard<'_, TypeMap> = client.data.write().await; //only allowed one mutable reference
        data.insert::<MsgListen>(HashSet::<MessageId>::new());
        data.insert::<MsgUpdate>(HashMap::<MessageId, MessageUpdateEvent>::new());
        let params = VectorizeOptions {chunksize: DEFAULT_CHUNK_SIZE, thresholds: DEFAULT_THRESHOLDS, numcolours: DEFAULT_COLOURS, shouldcrash};
        insert_params(data, params);
}

#[async_trait]
impl EventHandler for DefaultHandler {
    async fn message(&self, _ctx: Context, _msg: Message) {
        println!("message received");
    }

    //USED TO HANDLE SITUATIONS WHERE LINKS TAKE A WHILE TO LOAD THEIR IMAGE
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

        else
        {
            println!("Msg update was not listened for ");
        }
    }
}

impl TypeMapKey for MsgListen
{
    type Value = HashSet<MessageId>;
}

impl TypeMapKey for MsgUpdate
{
    type Value = HashMap<MessageId, MessageUpdateEvent>;
}

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
        if let Some(thing) = updated_write.remove(&msg_id)
        {
            let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
            stored_hashset.remove(&msg_id);
            return Ok(thing);
        }
        else
        {
            let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
            stored_hashset.remove(&msg_id);
            return Err(String::from("couldn't remove"));
        }
    }
    
    // if exists: load data from hashmap
    let mut data_write = ctx.data.write().await;
    let stored_hashset = data_write.get_mut::<MsgListen>().unwrap();
    stored_hashset.remove(&msg_id);
    Err(String::from("timed out"))
}

#[command]
async fn thresholds(ctx: &Context, _msg: &Message, args: Args) -> CommandResult
{
    println!("Command 'thresholds' called");

    let mut mutable_args = args;

    let input = match mutable_args.single::<u32>() {
        Ok(e) => e,
        Err(err) => { 
            eprintln!("Given invalid threshold!: {}", err); 
            return Ok(());
        }
    };

    let mut params = get_params(ctx).await;
    params.thresholds = input.to_string();
    let data_write = ctx.data.write().await;
    insert_parsed_params(data_write, params);

    Ok(())
}

#[command]
async fn chunksize(ctx: &Context, _msg: &Message, args: Args) -> CommandResult
{
    println!("Command 'chunksize' called");

    let mut mutable_args = args;

    let input = match mutable_args.single::<u32>() {
        Ok(e) => e,
        Err(err) => { 
            eprintln!("Given invalid chunksize!: {}", err); 
            return Ok(());
        }
    };

    let mut params = get_params(ctx).await;
    params.chunksize = input.to_string();
    let data_write = ctx.data.write().await;
    insert_parsed_params(data_write, params);

    Ok(())
}

#[command]
async fn numcolours(ctx: &Context, _msg: &Message, args: Args) -> CommandResult
{
    println!("Command 'numcolours' called");

    let mut mutable_args = args;

    let input = match mutable_args.single::<u32>() {
        Ok(e) => e,
        Err(err) => { 
            eprintln!("Given invalid numcolours!: {}", err); 
            return Ok(());
        }
    };

    let mut params = get_params(ctx).await;
    params.numcolours = input.to_string();
    let data_write = ctx.data.write().await;
    insert_parsed_params(data_write, params);

    Ok(())
}

#[command]
#[aliases("params")]
async fn vectorizerparams(ctx: &Context, msg: &Message, args: Args) -> CommandResult
{
    println!("vectorizeparams command");

    let mut mutable = args;
    let possiblechunksize = mutable.single::<u32>();
    let possiblethreshold = mutable.single::<u32>();
    let possiblecolours = mutable.single::<u32>();

    if possiblechunksize.is_ok() && possiblethreshold.is_ok() && possiblecolours.is_ok() {        
        let data_write = ctx.data.write().await;
        let params = VectorizeOptions {
            chunksize: possiblechunksize.unwrap(), 
            thresholds: possiblethreshold.unwrap(),
            numcolours: possiblecolours.unwrap(),
            shouldcrash: false, //MVP will never be called during a crash test. crashing cannot occur with user facing calls
        };
        let parsed = insert_params(data_write, params);
        
        let result = msg.reply(
            &ctx.http, 
            format!("Set Chunk Size to: {}, Thresholds to: {}, Num Colours to: {}", 
            parsed.chunksize, parsed.thresholds, parsed.numcolours))
            .await;
        
        if let Err(why) = result {
            eprintln!("Error sending params reply: {:?}", why);
        }   
    }

    else if let Err(why) = msg.reply(&ctx.http, "incorrect arguments given").await {
        eprintln!("Error sending params reply: {:?}", why);
    }
    Ok(())
}

#[command]
#[aliases("vec")]
async fn vectorize(ctx: &Context, msg: &Message) -> CommandResult
{   
    println!("vectorize command");

    let mut embed_urls: Vec<String> = vec![];
    if msg.embeds.len() < 1 && msg.attachments.len() < 1 // No embed, lets wait for an on_update
    {   
        println!("No embeds; waiting for message update");
        let update: Result<MessageUpdateEvent, String> = wait_for_message_update(msg.id, &ctx).await;
        
        match update
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
            Err(err) => {
                println!("Received Err {} from wait_for_message_update for id: {}", err, msg.id);

                if let Err(why) = msg.reply(&ctx.http, "timed out waiting for message content.").await {
                    eprintln!("Error sending reply: {:?}", why);
                }
                return Ok(());
            }
                
        }
    }

    else //embed found
    {        
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
    if let Err(why) = msg.reply(&ctx.http, START_MESSAGE).await {
        eprintln!("Error sending start reply: {:?}", why);
    }
    println!("Sending {0} urls to vectoriser", embed_urls.len());
    vectorize_urls(&ctx, &msg, &embed_urls).await;
    
    Ok(())
}

async fn vectorize_urls(ctx: &Context, msg: &Message, urls: &Vec<String>)
{  
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

        let mut inputname = String::from("input.");
        let file_extension = url.split('.').last().unwrap();
        inputname.push_str(file_extension);
        println!("input filename: {}", inputname);

        {
            let mut file = match File::create(Path::new(&inputname))
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

        // Get Options        
        let options: ParsedOptions = get_params(ctx).await;
        let shouldcrash = options.shouldcrash.parse::<bool>().unwrap();
        println!("vectorizing urls. shouldcrash: {}", shouldcrash);

        if shouldcrash {
            println!("shouldcrash == true. initiating crash...");
            let _crash = crashing_this_plane();
        }

        let outputname = String::from(constants::OUTPUT_SVG_FILE);
        println!("Vectorizing....");
        let result = do_vectorize(&inputname, options);

        let possibleerror: &str = result.into();

        if possibleerror != "SuccessCode" {
            let mut errmessage: String = format!("{}", ERR_MESSAGE);
            errmessage = errmessage.add(possibleerror);
            if let Err(why) = msg.reply(&ctx.http, errmessage)
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
        
        // Compress the svg file first
        let output_file = File::create(OUTPUT_ARCHIVE).expect("Could not create the output 7z file!");
        let contents = read(outputname).expect("Failed to read output svg file!");
        let mut encoder = XzEncoder::new(&output_file, 9);

        encoder.write_all(&contents).expect("Failed to write svg file contents!");
        encoder.try_finish().expect("Failed to finish compressing!");
        
        // Send the output
        let msg_files = vec![OUTPUT_ARCHIVE, png_output.as_str()];
        let msg = msg.channel_id.send_files(&ctx.http, msg_files, |m|
        {
            m.content(END_MESSAGE)
        }).await;

        if let Err(err) = msg
        {
            println!("Error sending result {}", err);
        }
    }
}
