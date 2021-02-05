mod secrettoken;
mod constants;
mod bot;
mod core;
mod svg;

// #[tokio::main(flavor = "current_thread")]
// async fn main() {
    
// }

fn main()
{
    tokio::runtime::Builder::new_current_thread()
        .enable_all()
        .build()
        .unwrap()
        .block_on(async {
            fk_you().await;
        })
}

async fn fk_you()
{
    //start discord bot
    let token = secrettoken::gettoken();
    let mut client = bot::create_vec_bot(token).await;
    
    if let Err(why) = client.start().await
    {
        println!("Client poopoo error: {:?}", why);
    }
}