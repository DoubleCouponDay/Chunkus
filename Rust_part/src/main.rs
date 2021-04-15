mod secrettoken;
mod constants;
mod bot;
mod core;
mod svg;
mod options;
mod error_show;

extern crate libc;
fn main()
{
    tokio::runtime::Builder::new_current_thread()
        .enable_all()
        .build()
        .unwrap()
        .block_on(async {
            run_bot().await;
        })
}

async fn run_bot()
{
    let tokenobj = secrettoken::gettoken();
    let token = tokenobj.as_str();
    let mut client = bot::create_vec_bot(token).await;
    
    if let Err(why) = client.start().await
    {
        println!("Client error: {:?}", why);
    }
}
